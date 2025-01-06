#
# Copyright (C) 2025 Markus Lavin (https://www.zzzconsulting.se/)
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
from chipregs import *



#
# Bitplanes
#
class Bitplanes(Elaboratable):
  def __init__(self):
    self.o_color = Signal(5)

    # Chip reg write access (ingress)
    self.i_chip_reg_addr = Signal(9)
    self.i_chip_reg_data = Signal(16)
    self.i_chip_reg_wen = Signal()

  def elaborate(self, platform):
    m = Module()

    bplxshift_array = Array([Signal(16) for _ in range(6)])
    bplxdat_array = Array([Signal(16) for _ in range(6)])

    for idx, bplshift in enumerate(bplxshift_array):
      m.d.comb += [
        self.o_color[idx-1].eq(bplshift[15]),
      ]
      m.d.sync += [
        bplshift.eq(Cat(C(0,1), bplshift[0:15])),
      ]

    # Generate address decode logic for writes
    for idx, bpldat in enumerate(bplxdat_array):
      with m.If((self.i_chip_reg_addr == (ChipReg.BPL1DAT + idx * (ChipReg.BPL2DAT - ChipReg.BPL1DAT))) & self.i_chip_reg_wen):
        m.d.sync += bpldat.eq(self.i_chip_reg_data)

    # Write to BPL1DAT triggers a reload of all pixel shifters
    with m.If((self.i_chip_reg_addr == ChipReg.BPL1DAT) & self.i_chip_reg_wen):
      m.d.sync += bplxshift_array[0].eq(self.i_chip_reg_data)
      for idx in range(1,6):
        m.d.sync += bplxshift_array[idx].eq(bplxdat_array[idx])

    return m
