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
from chipregs import *

SPRxCTL_Layout = data.StructLayout({
  "start_h0": unsigned(1),
  "stop_v8": unsigned(1),
  "start_v8": unsigned(1),
  "_1": unsigned(4),
  "attach": unsigned(1),
  "stop_v0_v7": unsigned(8),
})

SPRxPOS_Layout = data.StructLayout({
  "start_h1_h8": unsigned(8),
  "start_v0_v7": unsigned(8),
})


#
# Sprites
#
class Sprite(Elaboratable):
  def __init__(self, regBase):
    self.regBase = regBase
    self.i_hpos = Signal(9)
    self.i_vpos = Signal(9)
    self.o_color = Signal(2)
    self.o_attach = Signal()

    # Chip reg write access (ingress)
    self.i_chip_reg_addr = Signal(9)
    self.i_chip_reg_data = Signal(16)
    self.i_chip_reg_wen = Signal()

  def elaborate(self, platform):
    m = Module()

    sprpos = Signal(SPRxPOS_Layout)
    sprctl = Signal(SPRxCTL_Layout)
    sprdata = Signal(16)
    sprdatb = Signal(16)


    start_v = Signal(9)
    start_h = Signal(9)
    stop_v = Signal(9)
    m.d.comb += [
      start_v.eq(Cat(sprpos.start_v0_v7, sprctl.start_v8)),
      start_h.eq(Cat(sprctl.start_h0, sprpos.start_h1_h8)),
      stop_v.eq(Cat(sprctl.stop_v0_v7, sprctl.stop_v8)),
    ]

    m.d.comb += [
      self.o_color.eq(0), # Output transparent by default
      self.o_attach.eq(sprctl.attach),
    ]

    enabled = Signal()
    cntr = Signal(4, reset=15)
    with m.If(enabled):
      m.d.comb += self.o_color.eq(Cat(sprdata.bit_select(cntr, 1), sprdatb.bit_select(cntr, 1)))
      m.d.sync += cntr.eq(cntr - 1)
      with m.If(cntr == 0):
        m.d.sync += enabled.eq(0)

    with m.If((start_v <= self.i_vpos) & (self.i_vpos < stop_v) & (start_h == self.i_hpos)):
      m.d.sync += enabled.eq(1)

    # Register access
    with m.If(self.i_chip_reg_wen):
      with m.Switch(self.i_chip_reg_addr):
        with m.Case(self.regBase + REG_SPR0POS - REG_SPR0POS):
          m.d.sync += sprpos.as_value().eq(self.i_chip_reg_data)
        with m.Case(self.regBase + REG_SPR0CTL - REG_SPR0POS):
          m.d.sync += sprctl.as_value().eq(self.i_chip_reg_data)
        with m.Case(self.regBase + REG_SPR0DATA - REG_SPR0POS):
          m.d.sync += sprdata.eq(self.i_chip_reg_data)
        with m.Case(self.regBase + REG_SPR0DATB - REG_SPR0POS):
          m.d.sync += sprdatb.eq(self.i_chip_reg_data)

    return m
