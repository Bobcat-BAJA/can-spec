#!/usr/bin/env python3
"""Check baja_daq.dbc against the CAN ID plan. Run before every DBC commit.

    pip install cantools
    python tools/check_dbc.py dbc/baja_daq.dbc
"""
import sys
import cantools

BLOCKS = [  # (lo, hi, name)
    (0x000, 0x07F, "Faults"),
    (0x080, 0x0FF, "Timing"),
    (0x100, 0x1FF, "Steering"),
    (0x200, 0x2FF, "Powertrain"),
    (0x300, 0x3FF, "Suspension and wheels"),
    (0x400, 0x4FF, "Vehicle dynamics"),
    (0x500, 0x5FF, "Fuel and electrical"),
    (0x600, 0x6FF, "Spare"),
    (0x700, 0x77F, "Heartbeats"),
]
BITRATE = 1_000_000
LOAD_LIMIT = 0.50


def frame_bits(dlc):
    """Worst-case bits for a standard (11-bit) frame incl. bit stuffing."""
    return 47 + 8 * dlc + (34 + 8 * dlc - 1) // 4


def main(path):
    db = cantools.database.load_file(path, strict=True)  # strict: overlaps, DLC, ranges
    errors, fps, bps = [], 0.0, 0.0
    ids = {}
    for m in db.messages:
        if m.frame_id in ids:
            errors.append(f"0x{m.frame_id:03X} used by {ids[m.frame_id]} and {m.name}")
        ids[m.frame_id] = m.name
        if m.is_extended_frame:
            errors.append(f"{m.name}: extended ID not allowed (CAN 2.0A only)")
        if not any(lo <= m.frame_id <= hi for lo, hi, _ in BLOCKS):
            errors.append(f"{m.name} 0x{m.frame_id:03X}: outside every ID block")
        if m.name.startswith("HB_"):
            if not (0x701 <= m.frame_id <= 0x77F):
                errors.append(f"{m.name}: heartbeat must be 0x700 + node ID")
        for s in m.signals:
            if s.byte_order != "little_endian":
                errors.append(f"{m.name}.{s.name}: must be little-endian")
            if s.unit is None and s.scale != 1:
                errors.append(f"{m.name}.{s.name}: scaled signal needs a unit")
        # round-trip: encode the midpoint of every signal and decode it back
        data = {s.name: ((s.minimum or 0) + (s.maximum or 0)) / 2 for s in m.signals}
        back = m.decode(m.encode(data, scaling=True, strict=False), decode_choices=False)
        for s in m.signals:
            if abs(back[s.name] - data[s.name]) > abs(s.scale):
                errors.append(f"{m.name}.{s.name}: round-trip mismatch")
        on_bus = m.dbc.attributes.get("HubInternal").value == 0 if m.dbc and "HubInternal" in m.dbc.attributes else True
        cyc = m.cycle_time or 0
        if on_bus and cyc:
            fps += 1000 / cyc
            bps += 1000 / cyc * frame_bits(m.length)

    load = bps / BITRATE
    print(f"{len(db.messages)} messages, {len(db.nodes)} nodes")
    print(f"Periodic bus traffic: {fps:.0f} frames/s, {load:.1%} of 1 Mbit/s worst case")
    if load > LOAD_LIMIT:
        errors.append(f"bus load {load:.1%} over {LOAD_LIMIT:.0%} limit")
    for e in errors:
        print("FAIL:", e)
    print("PASS" if not errors else f"{len(errors)} problem(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "dbc/baja_daq.dbc"))
