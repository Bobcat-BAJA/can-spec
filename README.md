# Baja DAQ CAN spec

The single source of truth for the Bobcat Baja DAQ CAN bus: what every node sends, on which ID, in what format, and how a node board is built. If your board talks on the bus, it follows this repo.

> **Frozen Oct 19, 2026 (tag `spec-v0`).** Other teams build on these IDs and signals. Adding a message in your own slice: announce it at the Monday meeting. Changing or removing anything existing: agree at the meeting first, then bump `VERSION` in the DBC.

## What's here

| Path | What | Kept by |
|---|---|---|
| `dbc/baja_daq.dbc` | Every message and signal: bytes, type, factor, offset, unit, cycle time. The machine-readable spec. | Toph |
| `docs/CAN_Node_Spec_v0.1.docx` | The human-readable spec: bus rules, node hardware, frame format, message table. | Toph |
| `docs/can_id_plan.md` | ID blocks, per-node slices, node IDs, how to add a node | Toph |
| `docs/altium-standard.md` | Club Altium standard: board defaults, title block, OutJob, parts library, schematic and PCB rules | Toph |
| `registry/can_id_registry.csv` | Who owns which node ID and ID slices | Wrigley |
| `tools/check_dbc.py` | DBC checker. Run before every DBC commit; must print `PASS`. | Toph |

## Quick start

```sh
git clone https://github.com/Bobcat-BAJA/can-spec.git
cd can-spec
python -m pip install cantools

# Check the DBC (duplicate IDs, ID blocks, endianness, round-trip, bus load)
python tools/check_dbc.py dbc/baja_daq.dbc

# Decode a candump log into real units
cantools decode dbc/baja_daq.dbc < candump.log

# Browse every message and signal
cantools dump dbc/baja_daq.dbc
```

On Windows, if `python` opens the Microsoft Store, turn off the `python.exe` alias under Settings › Apps › Advanced app settings › App execution aliases, or use `py`.

## The bus at a glance

| | |
|---|---|
| Protocol | Classic CAN 2.0A, 11-bit IDs, 1 Mbit/s. No CAN FD, no remote frames. |
| Transceiver | TI SN65HVD230DR on 3.3 V, RS to GND |
| Termination | 120 Ω in the two harness end plugs only (bus measures 60 Ω). No terminator on any node. |
| Connector | One Deutsch DT13-4P per node (12 V, GND, CAN-H, CAN-L), mates with DT06-4S drop plugs |
| Timing | Hub sends `SYNC` on `0x080` every 50 ms; nodes timestamp data relative to it |
| Bus load | ~1,060 frames/s, 12.9% of 1 Mbit/s worst case. Limit is 50%. |

## Node hardware

| | |
|---|---|
| MCU | Seeed XIAO ESP32-C3 on every node. The hub is a XIAO ESP32-S3. |
| CAN pins | TX = **D7** (GPIO20), RX = **D10** (GPIO10) |
| Boot pins | **D0, D8, D9** are sampled at reset. Never pull them low at power-up. |
| Firmware | Arduino "USB CDC On Boot" = **Enabled**, or `Serial` takes D7. Use D-pin names, not GPIO numbers. |
| Board | Start from the club Altium project template and place the **CAN Node Core** reuse block. See `docs/altium-standard.md`. |

## Frame format

**Data frames** carry a 3-byte header and up to 5 data bytes:

| Byte | Field | Type | Meaning |
|---|---|---|---|
| 0 | Sequence counter | u8 | +1 per message on this ID, wraps 255 → 0. A gap means a dropped frame. |
| 1–2 | Time since SYNC | u16 | µs since the node last saw SYNC. `0xFFFF` = no SYNC yet. |
| 3–7 | Data | — | Per message, see the DBC |

**Heartbeat** on `0x700 + node ID`, 1 Hz, every node, no header: node ID, state (0 booting, 1 OK, 2 warning, 3 fault), error flags (u16), firmware major/minor, uptime (s).

**Data rules:** integers only, little-endian, no floats. Real value = raw × factor + offset. Signed types for anything that can go negative. "Not available" is all ones (`0xFF`, `0xFFFF`, `0x7FFF` signed), never 0. Periodic messages on a steady schedule; faults on change, then 10 Hz while active.

## ID map

Lower ID wins arbitration, so faults sit lowest and slow data highest. Each node gets a 16-ID slice inside a block.

| IDs | Block |
|---|---|
| `0x000–0x07F` | Faults |
| `0x080–0x0FF` | Timing (SYNC `0x080`, GPS time `0x090`) |
| `0x100–0x1FF` | Steering |
| `0x200–0x2FF` | Powertrain |
| `0x300–0x3FF` | Suspension and wheels |
| `0x400–0x4FF` | Vehicle dynamics |
| `0x500–0x5FF` | Fuel and electrical |
| `0x600–0x6FF` | Spare |
| `0x700–0x77F` | Heartbeats (`0x700` + node ID) |

`0x400`, `0x401` and `0x510` are hub-internal (marked `HubInternal = 1` in the DBC): logged under these IDs so logs decode, but never put on the wire. Nobody else may use them.

## Nodes

| Node ID | Node | DBC name | Built by | Status |
|---|---|---|---|---|
| `0x01` | Hub | `HUB` | DAQ | assigned |
| `0x02` | Rear-steer | `REARSTEER` | Toph | assigned |
| `0x03` | Front diff | `DIFF_F` | Diff team | assigned |
| `0x04` | Fuel gauge (PIC) | `FUEL` | Unassigned | assigned |
| `0x05` | SensorBoardV2 (wheel speed, shock) | `SENSORV2` | Shock team | assigned |
| `0x06` | Engine/CVT speed | `POWERTRAIN` | Unplanned | reserved |
| `0x07` | GPS | `GPS` | DAQ, if built | reserved |
| `0x08` | Rear diff | `DIFF_R` | Diff team | assigned |
| `0x09–0x0F` | — | — | — | reserved |
| `0x10–0x7F` | New nodes | — | Claim in the registry | open |

Full slice and message ownership is in `registry/can_id_registry.csv`.

## Adding a node or message

1. **Claim** a node ID and ID slice in `registry/can_id_registry.csv` (ask Wrigley).
2. **Define** every signal in `dbc/baja_daq.dbc`: bytes, type, factor, offset, unit, cycle time.
3. **Check:** `python tools/check_dbc.py dbc/baja_daq.dbc` until it prints `PASS`.
4. **Review:** open a pull request; Toph reviews.
5. **Announce** additions at the Monday meeting.
6. **Build** the board from the club template and CAN Node Core block.
7. **Bench test:** plug in, see the heartbeat on the dashboard, check values decode in real units.

Changes to **existing** IDs or signals need agreement at the Monday meeting *before* the pull request, and a `VERSION` bump in the DBC.

## What the checker enforces

`tools/check_dbc.py` loads the DBC in strict mode (no overlapping signals, valid DLCs) and fails on:

- duplicate frame IDs, or extended (29-bit) IDs
- any ID outside the blocks above
- `HB_*` messages not on `0x701–0x77F`
- big-endian signals, or scaled signals with no unit
- signals that don't survive an encode/decode round trip
- periodic on-bus traffic over 50% of 1 Mbit/s

## Contacts

| Area | Who |
|---|---|
| Spec, DBC, ID plan, Altium library | Toph Jones |
| ID registry | Wrigley Beardmore |
| Changes to anything frozen | Monday meeting |
