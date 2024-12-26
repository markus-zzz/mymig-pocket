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
from video import *
from copper import Copper
from chipregs import *
from sprites import *

#
# MyMig
#
class MyMig(Elaboratable):
  def __init__(self):

    # Video signal
    self.o_video_rgb = Signal(24)
    self.o_video_hsync = Signal()
    self.o_video_vsync = Signal()
    self.o_video_de = Signal()

    # CPU interface
    self.i_cpu_addr = Signal(24)
    self.i_cpu_data = Signal(16)
    self.o_cpu_data = Signal(16)
    self.i_cpu_req = Signal()
    self.i_cpu_we = Signal()
    self.o_cpu_ack = Signal()

    # Chip RAM interface
    self.o_chip_ram_addr = Signal(20)
    self.i_chip_ram_data = Signal(16)
    self.o_chip_ram_data = Signal(16)
    self.o_chip_ram_we = Signal()

    self.ports = [
        self.o_video_rgb, self.o_video_hsync, self.o_video_vsync, self.o_video_de,
        self.i_cpu_addr, self.i_cpu_data, self.o_cpu_data, self.i_cpu_req, self.i_cpu_we, self.o_cpu_ack,
        self.o_chip_ram_addr, self.i_chip_ram_data, self.o_chip_ram_data, self.o_chip_ram_we,
    ]

  def elaborate(self, platform):
    m = Module()

    m.submodules.u_video = u_video = VideoTiming()
    m.submodules.u_copper = u_copper = Copper()

    chip_reg_addr = Signal(9)
    chip_reg_rdata = Signal(16)
    chip_reg_wdata = Signal(16)
    chip_reg_wen = Signal()


    #
    # Chip RAM arbitration - begin
    #
    m.d.comb += [
      u_copper.i_chip_ram_data.eq(self.i_chip_ram_data),
      u_copper.i_vsync.eq(u_video.o_vsync),
      u_copper.i_hpos.eq(u_video.o_hpos),
      u_copper.i_vpos.eq(u_video.o_vpos),
      u_copper.i_chip_reg_addr.eq(chip_reg_addr),
      u_copper.i_chip_reg_data.eq(chip_reg_wdata),
      u_copper.i_chip_reg_wen.eq(chip_reg_wen),
    ]
    with m.If(u_copper.o_chip_ram_req): # Copper (R)
      m.d.comb += [
        self.o_chip_ram_addr.eq(u_copper.o_chip_ram_addr),
        u_copper.i_chip_ram_ack.eq(1),
      ]
    with m.Elif(self.i_cpu_req & ((0 <= self.i_cpu_addr) & (self.i_cpu_addr < 0x1fffff))): # CPU (R/W)
      m.d.comb += [
        self.o_chip_ram_addr.eq(self.i_cpu_addr[1:]),
        self.o_chip_ram_data.eq(self.i_cpu_data),
        self.o_chip_ram_we.eq(self.i_cpu_we),
        self.o_cpu_data.eq(self.i_chip_ram_data),
        self.o_cpu_ack.eq(1),
      ]
    #
    # Chip RAM arbitration - end
    #

    m.d.comb += [
      self.o_video_hsync.eq(u_video.o_hsync),
      self.o_video_vsync.eq(u_video.o_vsync),
      self.o_video_de.eq(u_video.o_de),
    ]

    color = Signal(5)
    m.d.comb += color.eq(0) # Default background color

    # XXX: Sprites and Playfields to override color

    #
    # Chip REG arbitration - begin
    #
    with m.If(u_copper.o_chip_reg_wen):
      m.d.comb += [
        chip_reg_addr.eq(u_copper.o_chip_reg_addr),
        chip_reg_wdata.eq(u_copper.o_chip_reg_data),
        chip_reg_wen.eq(u_copper.o_chip_reg_wen),
      ]
    with m.If((self.i_cpu_addr[12:24] == 0xdff) & self.i_cpu_req):
      m.d.comb += [
        chip_reg_addr.eq(self.i_cpu_addr[0:9]),
        chip_reg_wdata.eq(self.i_cpu_data),
        chip_reg_wen.eq(self.i_cpu_we),
        self.o_cpu_data.eq(chip_reg_rdata),
        self.o_cpu_ack.eq(1),
      ]
    #
    # Chip REG arbitration - end
    #

    # Registers COLOR00-COLOR31
    color_palette = Array([Signal(12) for _ in range(32)])
    # Generate address decode logic for writes
    for idx, color_reg in enumerate(color_palette):
      with m.If((chip_reg_addr == (REG_COLOR00 + idx * 2)) & chip_reg_wen):
        m.d.sync += color_reg.eq(chip_reg_wdata)
      s = Signal(color_reg.shape(), name='color_reg_{}'.format(idx))
      m.d.comb += s.eq(color_reg)

    color_rgb = Signal(12)
    m.d.comb += color_rgb.eq(color_palette[color])
    m.d.comb += self.o_video_rgb.eq(Cat(C(0,4), color_rgb[0:4], C(0,4), color_rgb[4:8], C(0,4), color_rgb[8:12]))

    return m

#
# Generate verilog
#

from amaranth.back import verilog
import sys
import os

if __name__ == "__main__":

  mymig = MyMig()

  with open("mymig.v", "w") as f:
    f.write(verilog.convert(elaboratable=mymig, name='mymig_top', ports=mymig.ports))
