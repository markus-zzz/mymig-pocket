#
# Copyright (C) 2024-2025 Markus Lavin (https://www.zzzconsulting.se/)
#
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# yapf --in-place --recursive --style="{indent_width: 2, column_limit: 120}"

from amaranth import *
from amaranth.lib import data
import enum

# From http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0060.html

BPLCON0_Layout = data.StructLayout({
  "_1": unsigned(12),
  "bpu": unsigned(3),
  "_2": unsigned(1),
})

DDF_Layout = data.StructLayout({
  "_1": unsigned(2),
  "h3_h8": unsigned(6),
  "_2": unsigned(8),
})

DIW_Layout = data.StructLayout({
  "h0_h7": unsigned(8),
  "v0_v7": unsigned(8),
})

class ChipReg(enum.IntEnum):
  BLTDDAT = 0x000

  DMACONR = 0x002
  VPOSR = 0x004
  VHPOSR = 0x006
  DSKDATR = 0x008
  JOY0DAT = 0x00A
  JOY1DAT = 0x00C
  CLXDAT = 0x00E
  ADKCONR = 0x010
  POT0DAT = 0x012
  POT1DAT = 0x014
  POTGOR = 0x016
  SERDATR = 0x018
  DSKBYTR = 0x01A
  INTENAR = 0x01C
  INTREQR = 0x01E
  DSKPTH = 0x020
  DSKPTL = 0x022
  DSKLEN = 0x024
  DSKDAT = 0x026
  REFPTR = 0x028
  VPOSW = 0x02A
  VHPOSW = 0x02C
  COPCON = 0x02E
  SERDAT = 0x030
  SERPER = 0x032
  POTGO = 0x034
  JOYTEST = 0x036

  STREQU = 0x038
  STRVBL = 0x03A
  STRHOR = 0x03C
  STRLONG = 0x03E

  BLTCON0 = 0x040
  BLTCON1 = 0x042
  BLTAFWM = 0x044
  BLTALWM = 0x046
  BLTCPTH = 0x048
  BLTCPTL = 0x04A
  BLTBPTH = 0x04C
  BLTBPTL = 0x04E
  BLTAPTH = 0x050
  BLTAPTL = 0x052
  BLTDPTH = 0x054

  BLTDPTL = 0x056

  BLTSIZE = 0x058
  BLTCON0L = 0x05A
  BLTSIZV = 0x05C
  BLTSIZH = 0x05E
  BLTCMOD = 0x060
  BLTBMOD = 0x062
  BLTAMOD = 0x064
  BLTDMOD = 0x066

  BLTCDAT = 0x070
  BLTBDAT = 0x072

  BLTADAT = 0x074

  SPRHDAT = 0x078

  DENISEID = 0x07C

  DSKSYNC = 0x07E
  COP1LCH = 0x080

  COP1LCL = 0x082

  COP2LCH = 0x084

  COP2LCL = 0x086

  COPJMP1 = 0x088
  COPJMP2 = 0x08A
  COPINS = 0x08C
  DIWSTRT = 0x08E

  DIWSTOP = 0x090

  DDFSTRT = 0x092

  DDFSTOP = 0x094

  DMACON = 0x096
  CLXCON = 0x098
  INTENA = 0x09A

  INTREQ = 0x09C

  ADKCON = 0x09E
  AUD0LCH = 0x0A0

  AUD0LCL = 0x0A2
  AUD0LEN = 0x0A4
  AUD0PER = 0x0A6
  AUD0VOL = 0x0A8
  AUD0DAT = 0x0AA

  AUD1LCH = 0x0B0
  AUD1LCL = 0x0B2
  AUD1LEN = 0x0B4
  AUD1PER = 0x0B6
  AUD1VOL = 0x0B8
  AUD1DAT = 0x0BA

  AUD2LCH = 0x0C0
  AUD2LCL = 0x0C2
  AUD2LEN = 0x0C4
  AUD2PER = 0x0C6
  AUD2VOL = 0x0C8
  AUD2DAT = 0x0CA

  AUD3LCH = 0x0D0
  AUD3LCL = 0x0D2
  AUD3LEN = 0x0D4
  AUD3PER = 0x0D6
  AUD3VOL = 0x0D8
  AUD3DAT = 0x0DA

  BPL1PTH = 0x0E0
  BPL1PTL = 0x0E2
  BPL2PTH = 0x0E4
  BPL2PTL = 0x0E6
  BPL3PTH = 0x0E8
  BPL3PTL = 0x0EA
  BPL4PTH = 0x0EC
  BPL4PTL = 0x0EE
  BPL5PTH = 0x0F0
  BPL5PTL = 0x0F2
  BPL6PTH = 0x0F4
  BPL6PTL = 0x0F6

  BPLCON0 = 0x100

  BPLCON1 = 0x102

  BPLCON2 = 0x104
  BPLCON3 = 0x106

  BPL1MOD = 0x108
  BPL2MOD = 0x10A

  BPL1DAT = 0x110
  BPL2DAT = 0x112
  BPL3DAT = 0x114
  BPL4DAT = 0x116
  BPL5DAT = 0x118
  BPL6DAT = 0x11A

  SPR0PTH = 0x120
  SPR0PTL = 0x122
  SPR1PTH = 0x124
  SPR1PTL = 0x126
  SPR2PTH = 0x128
  SPR2PTL = 0x12A
  SPR3PTH = 0x12C
  SPR3PTL = 0x12E
  SPR4PTH = 0x130
  SPR4PTL = 0x132
  SPR5PTH = 0x134
  SPR5PTL = 0x136
  SPR6PTH = 0x138
  SPR6PTL = 0x13A
  SPR7PTH = 0x13C
  SPR7PTL = 0x13E
  SPR0POS = 0x140

  SPR0CTL = 0x142

  SPR0DATA = 0x144
  SPR0DATB = 0x146
  SPR1POS = 0x148

  SPR1CTL = 0x14A

  SPR1DATA = 0x14C
  SPR1DATB = 0x14E
  SPR2POS = 0x150

  SPR2CTL = 0x152

  SPR2DATA = 0x154
  SPR2DATB = 0x156
  SPR3POS = 0x158

  SPR3CTL = 0x15A

  SPR3DATA = 0x15C
  SPR3DATB = 0x15E
  SPR4POS = 0x160

  SPR4CTL = 0x162

  SPR4DATA = 0x164
  SPR4DATB = 0x166
  SPR5POS = 0x168

  SPR5CTL = 0x16A

  SPR5DATA = 0x16C
  SPR5DATB = 0x16E
  SPR6POS = 0x170

  SPR6CTL = 0x172

  SPR6DATA = 0x174
  SPR6DATB = 0x176
  SPR7POS = 0x178

  SPR7CTL = 0x17A

  SPR7DATA = 0x17C
  SPR7DATB = 0x17E
  COLOR00 = 0x180
  COLOR01 = 0x182
  COLOR02 = 0x184
  COLOR03 = 0x186
  COLOR04 = 0x188
  COLOR05 = 0x18A
  COLOR06 = 0x18C
  COLOR07 = 0x18E
  COLOR08 = 0x190
  COLOR09 = 0x192
  COLOR10 = 0x194
  COLOR11 = 0x196
  COLOR12 = 0x198
  COLOR13 = 0x19A
  COLOR14 = 0x19C
  COLOR15 = 0x19E
  COLOR16 = 0x1A0
  COLOR17 = 0x1A2
  COLOR18 = 0x1A4
  COLOR19 = 0x1A6
  COLOR20 = 0x1A8
  COLOR21 = 0x1AA
  COLOR22 = 0x1AC
  COLOR23 = 0x1AE
  COLOR24 = 0x1B0
  COLOR25 = 0x1B2
  COLOR26 = 0x1B4
  COLOR27 = 0x1B6
  COLOR28 = 0x1B8
  COLOR29 = 0x1BA
  COLOR30 = 0x1BC
  COLOR31 = 0x1BE

  HTOTAL = 0x1C0

  HSSTOP = 0x1C2
  HBSTRT = 0x1C4
  HBSTOP = 0x1C6
  VTOTAL = 0x1C8

  VSSTOP = 0x1CA
  VBSTRT = 0x1CC
  VBSTOP = 0x1CE

  BEAMCON0 = 0x1DC
  HSSTRT = 0x1DE
  VSSTRT = 0x1E0
  HCENTER = 0x1E2
  DIWHIGH = 0x1E4
