# Analogue Pocket - MyMig

[Amiga](https://en.wikipedia.org/wiki/Amiga) inspired
[RISC-V](https://en.wikipedia.org/wiki/RISC-V) system for the [Analogue
Pocket](https://www.analogue.co/pocket).

MyMig aims to be a fun platform for experimentation (both HW and SW) for those
interested in tinkering around with Amiga style hardware in a modern setting.

**It does not aim to be an Amiga emulator**.

Implementation of the Amiga chipset (OCS) is based around the information found
in [Amiga Hardware Reference
Manual](http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0000.html)
and all implemented registers aim to be compatible with the definitions found
there (within reasonable extents).

The [FPGA](https://en.wikipedia.org/wiki/Field-programmable_gate_array) based
implementation is written in a mix of
[Verilog](https://en.wikipedia.org/wiki/Verilog) and [Amaranth
HDL](https://github.com/amaranth-lang/amaranth) and, at least from my
viewpoint, provides an easy and clear insight into the concepts of the various
parts that make up the Amiga Original Chip Set (OCS).

Not yet burdened with too many details and the nature of a HDL description is
much better suited for building and understanding than what would be found in
any software equivalent.
