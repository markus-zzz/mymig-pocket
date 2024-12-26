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
import enum

InstrLayout = data.UnionLayout({
  "raw": unsigned(32),
  "move": data.StructLayout({
    "always_0": unsigned(1),
    "reg": unsigned(8),
    "reserved": unsigned(7),
    "value": unsigned(16),
  }),
  "wait": data.StructLayout({
    "always_1": unsigned(1),
    "hp": unsigned(7),
    "vp": unsigned(8),
    "always_0": unsigned(1),
    "he": unsigned(7),
    "ve": unsigned(7),
    "bfd": unsigned(1),
  }),
})

#
# Copper
#
class Copper(Elaboratable):
  def __init__(self):

    # Chip RAM read access
    self.o_chip_ram_addr = Signal(20)
    self.i_chip_ram_data = Signal(16)
    self.o_chip_ram_req = Signal()
    self.i_chip_ram_ack = Signal()

    # Chip reg write access (egress)
    self.o_chip_reg_addr = Signal(9)
    self.o_chip_reg_data = Signal(16)
    self.o_chip_reg_wen = Signal()

    # Chip reg write access (ingress)
    self.i_chip_reg_addr = Signal(8)
    self.i_chip_reg_data = Signal(16)
    self.i_chip_reg_wen = Signal()

    # Video
    self.i_vsync = Signal()
    self.i_hpos = Signal(9)
    self.i_vpos = Signal(9)

  def elaborate(self, platform):
    m = Module()

    pc = Signal(19)
    ir = Signal(InstrLayout)
    location1 = Signal(19)
    location2 = Signal(19)
    enabled = Signal() # XXX: Does not really exist in real HW

    m.d.comb += self.o_chip_ram_addr.eq(pc)

    class State(enum.Enum):
      FETCH1 = 0
      FETCH2 = 1
      EXECUTE = 2
    state = Signal(State)

    with m.If(enabled):
      with m.Switch(state):
        with m.Case(State.FETCH1):
          m.d.comb += self.o_chip_ram_req.eq(1)
          with m.If(self.i_chip_ram_ack):
            m.d.sync += [
              ir.raw[0:16].eq(self.i_chip_ram_data),
              pc.eq(pc + 1),
              state.eq(State.FETCH2),
            ]
        with m.Case(State.FETCH2):
          m.d.comb += self.o_chip_ram_req.eq(1)
          with m.If(self.i_chip_ram_ack):
            m.d.sync += [
              ir.raw[16:32].eq(self.i_chip_ram_data),
              pc.eq(pc + 1),
              state.eq(State.EXECUTE),
            ]
        with m.Case(State.EXECUTE):
          with m.If(ir.move.always_0 == 0): # MOVE
            m.d.comb += [
              self.o_chip_reg_addr.eq(Cat(C(0, 1), ir.move.reg)),
              self.o_chip_reg_data.eq(ir.move.value),
              self.o_chip_reg_wen.eq(1),
            ]
            m.d.sync += state.eq(State.FETCH1)
          with m.Elif((ir.wait.always_1 == 1) & (ir.wait.always_0 == 0)): # WAIT
            masked_vpos = (self.i_vpos & Cat(ir.wait.ve, C(1, 1))) # MSB cannot be masked
            masked_hpos = (self.i_hpos[2:] & ir.wait.he) # Two LSB not used in comparison
            with m.If((masked_vpos > ir.wait.vp) |
                      ((masked_vpos == ir.wait.vp) & (masked_hpos >= ir.wait.hp))):
              m.d.sync += state.eq(State.FETCH1)
          with m.Elif((ir.raw[0] == 1) & (ir.raw[16] == 1)): # SKIP
            pass

    # Register access
    with m.If(self.i_chip_reg_wen):
      with m.Switch(self.i_chip_reg_addr):
        with m.Case(REG_COP1LCH):
          m.d.sync += location1[16:19].eq(self.i_chip_reg_data)
        with m.Case(REG_COP1LCL):
          m.d.sync += location1[0:16].eq(self.i_chip_reg_data)
        with m.Case(REG_COP2LCH):
          m.d.sync += location2[16:19].eq(self.i_chip_reg_data)
        with m.Case(REG_COP2LCL):
          m.d.sync += location2[0:16].eq(self.i_chip_reg_data)
        with m.Case(REG_COPJMP1):
          m.d.sync += [
            pc.eq(location1),
            state.eq(State.FETCH1),
            enabled.eq(1),
          ]
        with m.Case(REG_COPJMP2):
          m.d.sync += [
            pc.eq(location2),
            state.eq(State.FETCH1),
          ]

    # Program restarts every vsync
    with m.If(self.i_vsync):
      m.d.sync += [
        pc.eq(location1),
        state.eq(State.FETCH1),
      ]

    return m
