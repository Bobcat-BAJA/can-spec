# CAN ID Plan v0

Frozen at the Oct 19, 2026 meeting. Owner: Toph Jones. Registry kept by Wrigley Beardmore (`registry/can_id_registry.csv`).

**Rules**
- Lower ID wins arbitration, so the most critical messages get the lowest IDs: faults first, then SYNC.
- Blocks are by function. Inside a block, each node gets a 16-ID slice (`0xN0–0xNF`).
- A node transmits only on IDs assigned to it here. Two nodes on one ID corrupt each other.
- Heartbeat = `0x700 + node ID`, 1 Hz, every node.
- `0x400`, `0x401`, `0x510` are hub-internal: logged under these IDs so logs decode with the DBC, but never put on the wire. Nobody else may use them.

## Blocks

| IDs | Block | Assigned slices |
|---|---|---|
| 0x000–0x07F | Faults | 0x000–0x00F reserved · 0x010–0x01F rear-steer · 0x020–0x02F diffs (F 0x020, R 0x021) |
| 0x080–0x0FF | Timing | 0x080 SYNC (hub) · 0x090–0x09F GPS time |
| 0x100–0x1FF | Steering | 0x100–0x10F rear-steer |
| 0x200–0x2FF | Powertrain | 0x200–0x20F engine/CVT (reserved) · 0x210–0x21F diffs (F 0x210, R 0x211) |
| 0x300–0x3FF | Suspension and wheels | 0x300–0x31F SensorBoardV2 (wheel speed 0x300, shock 0x310) |
| 0x400–0x4FF | Vehicle dynamics | 0x400–0x40F hub IMU (internal) · 0x410–0x41F GPS |
| 0x500–0x5FF | Fuel and electrical | 0x500–0x50F fuel gauge · 0x510–0x51F hub battery (internal) |
| 0x600–0x6FF | Spare | — |
| 0x700–0x77F | Heartbeats | 0x700 + node ID |

## Node IDs

| Node ID | Node | Heartbeat | Built by |
|---|---|---|---|
| 0x01 | Hub | 0x701 | DAQ |
| 0x02 | Rear-steer | 0x702 | Toph |
| 0x03 | Front diff | 0x703 | Diff team |
| 0x04 | Fuel gauge (PIC) | 0x704 | Unassigned |
| 0x05 | SensorBoardV2 (wheel speed, shock) | 0x705 | Shock team |
| 0x06 | Engine/CVT speed | 0x706 | Unplanned, reserved |
| 0x07 | GPS | 0x707 | DAQ, if built |
| 0x08 | Rear diff | 0x708 | Diff team |
| 0x09–0x0F | Reserved | | |
| 0x10–0x7F | New nodes, claimed in the registry | 0x710–0x77F | |

## Adding a node or message
1. Claim a node ID and slice in the registry (Wrigley).
2. Add every signal to `dbc/baja_daq.dbc`: bytes, type, factor, offset, unit, rate.
3. Run `python tools/check_dbc.py`. It must print PASS (no duplicate IDs, every ID in a block, bus load under 50%).
4. Toph reviews. Additions are announced at the Monday meeting; changes to existing IDs or signals need agreement there first.
