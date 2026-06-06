# D100 BLE Bridge — wire protocol

Generic GATT transport over TCP. ESP32 = câm (chỉ relay byte BLE). Crypto, framing, sessionKey, AES-CCM hoàn toàn ở phía Python client.

## Transport

- TCP, port `8888` (configurable build-time / NVS).
- Một client connect đồng thời. Client mới connect → ESP32 disconnect client cũ + tear down BLE.
- Mỗi message = 1 dòng ASCII kết thúc `\n`. Token tách bằng space đơn.
- Reply là synchronous: mỗi request có **đúng 1** reply (ngoại trừ async event prefix `#`).
- Async events (notifications từ khoá) đẩy bất cứ lúc nào, line bắt đầu bằng `#`.

## Commands (client → ESP32)

| Command | Args | Reply | Mô tả |
|---|---|---|---|
| `PING` | — | `OK PONG <uptime_ms>` | health check |
| `STATUS` | — | `OK STATUS <wifi> <ble_state> <peer_mac>` | trạng thái |
| `SCAN` | `<mac_hex 12 char>` `<timeout_ms>` | `OK SCAN <addr_type:0|1> <rssi>` hoặc `ERR SCAN TIMEOUT` | scan tới khi thấy MAC |
| `CONNECT` | `<mac_hex>` `<addr_type:0|1>` | `OK CONN <mtu>` hoặc `ERR CONN <reason>` | GAP+GATT open, KHÔNG discover |
| `DISCOVER` | — | `OK DISC <n_svc>` rồi N dòng `# SVC <uuid_hex>` rồi N dòng `# CHR <uuid_hex> <val_handle:hex> <props_hex> <cccd_handle:hex_or_-1>` | full GATT enumeration |
| `SUB` | `<val_handle_hex>` | `OK SUB` hoặc `ERR SUB <reason>` | enable notification (write 01,00 CCCD) |
| `UNSUB` | `<val_handle_hex>` | `OK UNSUB` | disable notify (00,00) |
| `WRITE` | `<val_handle_hex>` `<data_hex>` | `OK WRITE` hoặc `ERR WRITE <reason>` | write WITH response |
| `WRITENR` | `<val_handle_hex>` `<data_hex>` | `OK WRITENR` ngay khi flush | write WITHOUT response |
| `READ` | `<val_handle_hex>` | `OK READ <data_hex>` | read characteristic |
| `MTU` | `<mtu_decimal>` | `OK MTU <negotiated>` | request MTU exchange |
| `DISCONNECT` | — | `OK DC` | tear BLE, giữ TCP |
| `RESET` | — | `OK RESET` | disconnect BLE + clear state + ready next |

## Async events (ESP32 → client)

| Event | Mô tả |
|---|---|
| `# N <val_handle_hex> <data_hex>` | notification từ khoá trên characteristic đã SUB |
| `# DC <reason_hex>` | BLE peer disconnect (link loss, peer-initiated) |

## Error codes

| `<reason>` | Nghĩa |
|---|---|
| `TIMEOUT` | hết time mà không có event mong đợi |
| `BUSY` | đang xử lý lệnh khác |
| `NOTCONN` | chưa GAP connect |
| `NOHANDLE` | handle truyền vào không thuộc GATT đã discover |
| `BLE <code_hex>` | NimBLE error code (vd `BLE 0x05` = ATT INSUF_AUTHEN) |
| `OOM` | hết heap khi parse |

## Ví dụ phiên unlock (Python ↔ ESP32 ↔ khoá D100)

```
> PING
< OK PONG 12345
> SCAN 5A58004D56ED 10000
< OK SCAN 1 -55
> CONNECT 5A58004D56ED 1
< OK CONN 247
> MTU 247
< OK MTU 247
> DISCOVER
< OK DISC 4
# SVC 1800
# CHR 2a00 0003 02 -1
...
# SVC ffb0
# CHR ffb1 002a 0c -1            ← handshake write (no notify)
# CHR ffb2 002d 10 002e          ← handshake notify
# SVC ff60
# CHR ff61 0033 0c -1            ← cmd write
# CHR ff62 0036 10 0037          ← cmd notify
> SUB 002d
< OK SUB
> SUB 0036
< OK SUB
> WRITE 002a 5a00000610...        ← 0610 cloudPublicKey frame
< OK WRITE
# N 002d 5a00...                  ← devicePublicKey response
> WRITE 002a 5a00000710...        ← 0710 verifyData
< OK WRITE
# N 002d 5a00ack...
> WRITE 0033 <AES-CCM openLock>
< OK WRITE
# N 0036 <AES-CCM lock_status>
> RESET
< OK RESET
```

## Notes

- Handle in HEX (Aqara dùng giá trị nhỏ, format `%04x`).
- Tất cả data hex KHÔNG có space/colon.
- Lệnh case-insensitive (uppercase recommended).
- Line max 4096 char (đủ cho data hex ~2KB).
- NimBLE host task ưu tiên cao hơn TCP task để notification không bị buffer trễ.
