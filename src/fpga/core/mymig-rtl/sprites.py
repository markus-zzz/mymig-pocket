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
from chipregs import *

#
# Sprites
#
class Sprite(Elaboratable):
  def __init__(self):

    self.i_hpos = Signal(9)
    self.i_vpos = Signal(8)
    self.o_color = Signal(5)
    self.o_color_en = Signal()

    # Chip RAM read access
    self.o_chip_ram_addr = Signal(20)
    self.i_chip_ram_data = Signal(16)
    self.o_chip_ram_req = Signal()
    self.i_chip_ram_ack = Signal()

    # Chip reg write access (ingress)
    self.i_chip_reg_addr = Signal(8)
    self.i_chip_reg_data = Signal(16)

  def elaborate(self, platform):
    m = Module()
    return m
