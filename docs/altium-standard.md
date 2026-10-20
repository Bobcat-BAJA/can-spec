# Bobcat Baja Altium Standard v0

Start every board from the club project template and place parts only from the club library. Then the rules, title block and fab outputs are already right, and you don't build your own OutJob (Essentials Module 27).

Template and library live in the Altium workspace (Americas – Ohio University – Bobcat Baja). This README is the reference copy.

## Board defaults (JLCPCB standard 2-layer)

| Item | Club value | JLC minimum |
|---|---|---|
| Stackup | 2 layers, FR-4, 1.6 mm, 1 oz | — |
| Clearance | 0.15 mm | 0.127 mm |
| Trace width | 0.15 min / 0.25 signal / 0.5 power (12 V, 5 V, GND) | 0.127 mm |
| Via | 0.3 mm hole / 0.6 mm diameter | 0.3 / 0.5 mm |
| Annular ring | ≥ 0.13 mm | 0.13 mm |
| Copper to board edge | 0.3 mm | 0.2 mm |
| Solder mask expansion | 0.05 mm | 0.05 mm |
| Solder mask bridge | 0.2 mm | 0.2 mm (green) |
| Silkscreen | 0.18 mm stroke, 1.2 mm text (never below 0.16 / 1.0) | 0.153 mm, 1.0 mm |
| Smallest passive | 0805 (hand assembly) | — |

- Units mm. PCB snap grid 0.25 mm; schematic grid stays at the Altium default.
- Board outline on **Mechanical 1**. Silkscreen on Top/Bottom Overlay, never a copper layer.
- Club values sit just above JLC's limits so a slightly-off fab run still passes.

## Title block (schematic sheet template)

Board name, Revision, Designer, Checked by, Date, Sheet x of y, "Bobcat Baja – Ohio University". Fill these as project parameters; the sheet reads them automatically.

## Fab outputs (OutJob)

One click makes a zip ready to upload to JLC:

- Gerbers: top/bottom copper, mask, paste, silkscreen, Mechanical 1 outline
- NC drill: Excellon, 2:4 precision
- Schematic PDF, BOM (MPN column)
- Pick-and-place (only if using JLC assembly)

## Club library: CAN test node parts

Picked by DigiKey's Most Popular sort within each spec (Oct 18, 2026), in stock and active.

| Ref | Part | MPN |
|---|---|---|
| U1 | XIAO ESP32-C3 (nodes; hub stays S3) | Seeed 113991054. CAN TX = D7, RX = D10; keep D0, D8, D9 (strapping) free |
| U2 | 5 V LDO, TO-263 (club standard) | TI LM1084ISX-5.0/NOPB. Pinout 1 GND, 2 OUT, 3 IN; tab = OUT. Needs 10 µF tantalum on the output |
| U3 | CAN transceiver, SOIC-8 | TI SN65HVD230DR |
| D1 | Reverse-polarity Schottky 40 V 3 A, SMA | Diodes Inc. B340A-13-F |
| D2 | 15 V TVS, SMB | Littelfuse SMBJ15A |
| D3 | CAN ESD, SOT-23 | TI ESD2CAN24DBZRQ1 |
| D4 | USB backfeed Schottky 40 V 1 A, SOD-123 | Diodes Inc. 1N5819HW-7-F |
| C1 | 10 µF 25 V X7S 0805 (regulator input) | TDK CGA4J1X7S1E106K125AE |
| C2 | 100 nF 50 V X7R 0805 | KEMET C0805C104K5RACTU |
| C3 | 10 µF 16 V tantalum, case A (regulator output) | KEMET T491A106K016AT (not yet DigiKey-checked) |
| R | 1 kΩ 1% 0805 | Vishay CRCW08051K00FKEA |
| R | 10 kΩ 1% 0805 | Yageo RC0805FR-0710KL |
| LED1 | Red 0805 | Lite-On LTST-C171KRKT |
| J1 | Deutsch DT 4-pin PCB header, 90° (one per node, mates with DT06-4S drop plugs) | TE DT13-4P |
| RV1 | Pot | Club stock, TBD |

No 120 Ω terminator on node boards: termination lives in the harness end plugs.

## Schematic rules

- **Power nets** are power ports with these exact names, and nothing else uses them: `+12`, `+5`, `+3.3`, `GND`. Power ports connect across the whole project, so no ports are needed for them.
- Project Options » Options » **Net Identifier Scope = Hierarchical** (not Strict Hierarchical, which makes power ports local).
- Signals leaving a sheet (CANH, CANL, sensor pins) need **ports**; net labels stay on their sheet.
- Start every node from the **CAN Node Core** reuse block (Design Reuse panel » Place as Sheet Symbol). Ports `D0_BOOT`, `D8_BOOT`, `D9_BOOT` are boot pins: never pull them low at power-up.

## PCB rules

- One **Deutsch DT13-4P** per node at the board edge, mating face overhanging. Pinout per the harness standards doc.
- Define CANH/CANL as differential pair `CAN` (PCB panel » Differential Pairs Editor), route with Interactive Differential Pair Routing, 0.25 mm width / 0.25 mm gap.
- Bottom-layer GND pour on every board.

## Adding a part
Ask Toph (library owner). Every part gets symbol + footprint + 3D model + MPN, checked against the datasheet before release.
