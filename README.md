# Baja DAQ CAN spec v0

| Path | What |
|---|---|
| `dbc/baja_daq.dbc` | Every message in CAN Node Spec v0 with factor, offset, units and cycle time |
| `docs/can_id_plan.md` | ID blocks, slices, node IDs, how to add a node |
| `registry/can_id_registry.csv` | Who owns which IDs (kept by Wrigley) |
| `tools/check_dbc.py` | Run before every DBC commit; must print PASS |

Decode a log: `pip install cantools`, then `cantools decode dbc/baja_daq.dbc < candump.log`.

**Frozen Oct 19, 2026 (tag `spec-v0`).** Other teams build on these IDs and signals. Adding a message in your own slice: announce it at the Monday meeting. Changing or removing anything existing: agree at the meeting first, then bump `VERSION` in the DBC.
