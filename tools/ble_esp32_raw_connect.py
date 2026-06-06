#!/usr/bin/env python3
"""Minimal connect test: bypass all retry/timeout logic, just CONNECT + observe.

Goal: confirm whether ESPHome bluetooth_proxy 2026.5.3 schedules an internal
disconnect at ~350ms when GAP is slow. If we still see "Disconnect before
connected, disconnect scheduled" in ESPHome logs, the issue is the proxy
firmware (not Python).
"""
from __future__ import annotations
import asyncio, logging, os, sys
from aioesphomeapi import APIClient

logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)s %(message)s")
_LOG = logging.getLogger("raw")

MAC_HEX = os.environ.get("LOCK_MAC", "5A58004D56ED")
TARGETS = (int(MAC_HEX, 16), int.from_bytes(bytes.fromhex(MAC_HEX)[::-1], "big"))

async def main():
    cli = APIClient("192.168.2.162", 6053, None,
                    noise_psk=os.environ["ESP32_NOISE_PSK"])
    await cli.connect(login=True)
    info = await cli.device_info()
    _LOG.info("proxy: %s feature_flags=%s", info.name, info.bluetooth_proxy_feature_flags)

    loop = asyncio.get_running_loop()
    addr_fut = loop.create_future()

    def on_adv(resp):
        for a in resp.advertisements:
            if a.address in TARGETS and not addr_fut.done():
                addr_fut.set_result((a.address, a.address_type))

    unsub = cli.subscribe_bluetooth_le_raw_advertisements(on_adv)
    addr, addr_type = await asyncio.wait_for(addr_fut, 30)
    unsub()
    _LOG.info("lock advert ✓ addr=%012x type=%s", addr, addr_type)

    # IMPORTANT: do NOT call unsubscribe; do NOT set any timeout in connect.
    # Just observe state callbacks.
    events = []
    def on_state(is_connected: bool, mtu: int, error: int):
        ts = loop.time()
        events.append((ts, is_connected, mtu, error))
        _LOG.info("STATE callback: connected=%s mtu=%s error=%s", is_connected, mtu, error)

    _LOG.info(">>> bluetooth_device_connect (no timeout)")
    await cli.bluetooth_device_connect(
        addr, on_state,
        timeout=90.0,
        feature_flags=0,           # force legacy CONNECT (no cache)
        address_type=addr_type,
    )
    _LOG.info(">>> CONNECT request returned, observing for 60s")
    await asyncio.sleep(60)
    _LOG.info("FINAL events: %s", events)
    try:
        await cli.bluetooth_device_disconnect(addr)
    except Exception as e:
        _LOG.warning("disconnect: %s", e)
    await cli.disconnect()

asyncio.run(main())
