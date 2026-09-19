#!/usr/bin/env python3
"""Print key events from the temporary MECHREVO P916F input device."""

from pathlib import Path
import struct


KEY_NAMES = {
    143: "KEY_WAKEUP",
    190: "KEY_F20",
    191: "KEY_F21",
    192: "KEY_F22",
    193: "KEY_F23",
    194: "KEY_F24",
    212: "KEY_CAMERA",
    228: "KEY_KBDILLUMTOGGLE",
    229: "KEY_KBDILLUMDOWN",
    230: "KEY_KBDILLUMUP",
    248: "KEY_MICMUTE",
    531: "KEY_TOUCHPAD_ON",
    532: "KEY_TOUCHPAD_OFF",
    148: "KEY_PROG1",
}


def find_device() -> Path:
    for name_file in Path("/sys/class/input").glob("event*/device/name"):
        if name_file.read_text().strip() == "MECHREVO P916F hotkeys":
            return Path("/dev/input") / name_file.parents[1].name
    raise SystemExit("MECHREVO P916F hotkey device not found")


device = find_device()
event_struct = struct.Struct("llHHi")
print(f"Reading {device}; press the requested keys, then press Ctrl+C.", flush=True)

with device.open("rb", buffering=0) as stream:
    while True:
        raw = stream.read(event_struct.size)
        if len(raw) != event_struct.size:
            continue
        _, _, event_type, code, value = event_struct.unpack(raw)
        if event_type == 1:
            state = {0: "release", 1: "press", 2: "repeat"}.get(value, str(value))
            print(f"EV_KEY code={code} {KEY_NAMES.get(code, 'UNKNOWN')} {state}", flush=True)
