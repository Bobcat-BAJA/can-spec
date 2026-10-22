# CAN ID Plan v0.3

v0 frozen at the Oct 19, 2026 meeting; v0.2 adds the corner-node topology approved there; v0.3 (Oct 20, 2026) retires the diff nodes (diff boards scrapped, car moves to mechanical diff control). Owner: Toph Jones. Registry kept by Wrigley Beardmore (`registry/can_id_registry.csv`).

**Rules**
- Lower ID wins arbitration, so the most critical messages get the lowest IDs: faults first, then SYNC.
- Blocks are by function. Inside a block, each node gets a 16-ID slice (`0xN0–0xNF`).
- A node transmits only on IDs assigned to it here. Two nodes on one ID corrupt each other.
- Heartbeat = `0x700 + node ID`, 1 Hz, every node.
- `0x400`, `0x401`, `0x510` are hub-internal: logged under these IDs so logs decode with the DBC, but never put on the wire. Nobody else may use them.
- Retired node IDs and ID slices are never reused. Corners stay on 0x320–0x323; nothing moves onto 0x210–0x213.

## Blocks

| IDs | Block | Assigned slices |
|---|---|---|
| 0x000–0x07F | Faults | 0x000–0x00F reserved · 0x010–0x01F rear-steer · 0x020–0x02F retired (old diffs) |
| 0x080–0x0FF | Timing | 0x080 SYNC (hub) · 0x090–0x09F GPS time |
| 0x100–0x1FF | Steering | 0x100–0x10F rear-steer · 0x120–0x12F cockpit |
| 0x200–0x2FF | Powertrain | 0x200–0x20F powertrain (0x200 speeds, 0x201 temps) · 0x210–0x21F retired (old diffs) |
| 0x300–0x3FF | Suspension and wheels | 0x300–0x31F retired (old SensorBoardV2) · 0x320–0x32F corners (FL 0x320, FR 0x321, RL 0x322, RR 0x323) |
| 0x400–0x4FF | Vehicle dynamics | 0x400–0x40F hub IMU (internal) · 0x410–0x41F GPS |
| 0x500–0x5FF | Electrical | 0x500–0x50F retired (old fuel gauge) · 0x510–0x51F hub battery (internal) |
| 0x600–0x6FF | Spare | — |
| 0x700–0x77F | Heartbeats | 0x700 + node ID |

## Node IDs

| Node ID | Node | Heartbeat | Built by |
|---|---|---|---|
| 0x01 | Hub | 0x701 | DAQ |
| 0x02 | Rear-steer | 0x702 | Toph |
| 0x03 | Retired (was front diff). Do not reuse | — | — |
| 0x04 | Retired (was fuel gauge). Do not reuse | — | — |
| 0x05 | Retired (was SensorBoardV2). Do not reuse | — | — |
| 0x06 | Powertrain | 0x706 | Unassigned |
| 0x07 | GPS | 0x707 | DAQ, if built |
| 0x08 | Retired (was rear diff). Do not reuse | — | — |
| 0x09 | Front-left corner | 0x709 | Shock team |
| 0x0A | Front-right corner | 0x70A | Shock team |
| 0x0B | Rear-left corner | 0x70B | Shock team |
| 0x0C | Rear-right corner | 0x70C | Shock team |
| 0x0D | Cockpit | 0x70D | Unassigned |
| 0x0E–0x0F | Reserved | | |
| 0x10–0x7F | New nodes, claimed in the registry | 0x710–0x77F | |

## Adding a node or message
1. Claim a node ID and slice in the registry (Wrigley).
2. Add every signal to `dbc/baja_daq.dbc`: bytes, type, factor, offset, unit, rate.
3. Run `python tools/check_dbc.py`. It must print PASS (no duplicate IDs, every ID in a block, bus load under 50%).
4. Toph reviews. Additions are announced at the Monday meeting; changes to existing IDs or signals need agreement there first.
