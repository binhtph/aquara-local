"""Decode the lock's `lock_local_log` entries into who/how, for HA events & sensors.

Ported from the research app (`app/src/cloud/lockmeta.ts decodeUnlockLog`). A log entry is
`{value: <hex>, source: "<src>,,<ts>,...", timeStamp: <ms>}`. The `value` encodes the action
and (for credential opens) the lock slot/userId; `source`'s first field is the trigger.
"""

from __future__ import annotations

from typing import Any

# credential type → machine label (matches docs/API.md §1.6)
CRED_LABEL: dict[int, str] = {
    1: "fingerprint",
    2: "password",
    3: "nfc",
    4: "ekey",
    5: "temp_password",
    6: "face",
    7: "nfc_tag",
}


def build_uid_map(credentials: list[dict[str, Any]]) -> dict[int, dict[str, Any]]:
    """userId (lock slot) → {who: user/group name, type: credential type}."""
    out: dict[int, dict[str, Any]] = {}
    for c in credentials or []:
        try:
            uid = int(str(c.get("typeValue", "0"))) & 0xFFFF
        except (TypeError, ValueError):
            continue
        out[uid] = {
            "who": c.get("typeGroupName") or c.get("typeName"),
            "type": int(c.get("type", 0) or 0),
        }
    return out


def decode_log(entry: dict[str, Any], uid_map: dict[int, dict[str, Any]]) -> dict[str, Any]:
    """Decode one `lock_local_log` entry → {action, method, user, user_id, timestamp, raw}.

    action: "unlock" | "lock" | "event". method: fingerprint/password/nfc/key/remote/auto/...
    """
    value = str(entry.get("value") or "").lower()
    source = str(entry.get("source") or "").split(",")[0]
    ts = entry.get("timeStamp") or entry.get("timestamp")
    try:
        b = bytes.fromhex(value)
    except ValueError:
        b = b""
    out: dict[str, Any] = {
        "action": "event",
        "method": "unknown",
        "user": None,
        "user_id": None,
        "timestamp": int(ts) if ts and str(ts).isdigit() else None,
        "raw": value,
    }
    # `0b0009 <marker> [uid LE] ...` — opened/closed at the lock
    if value.startswith("0b0009") and len(b) >= 8:
        marker = b[3]
        if marker == 0x20:  # opened with a registered credential
            uid = b[4] | (b[5] << 8)
            out["action"] = "unlock"
            out["user_id"] = uid
            u = uid_map.get(uid)
            out["user"] = (u or {}).get("who")
            out["method"] = CRED_LABEL.get((u or {}).get("type", -1), "credential")
            return out
        if marker in (0xB1, 0x00):  # auto-lock / closed
            out["action"] = "lock"
            out["method"] = "auto"
            return out
    # fall back to the source/trigger code
    out["action"] = "unlock"
    if source == "46":
        out["method"] = "remote"
    elif source == "11":
        out["method"] = "nfc"
    elif source == "12":
        out["method"] = "key"
    else:
        out["action"] = "event"
    return out
