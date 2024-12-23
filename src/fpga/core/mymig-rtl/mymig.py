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

REG_COLOR00 = 0x180


########################################################
# Video Timing for 320x256@60
# https://tomverbeure.github.io/video_timings_calculator
########################################################
# Aspect Ratio    5:4
# Pixel Clock     8               MHz
# H Total         480             Pixels
# H Active        320             Pixels
# H Blank         160             Pixels
# H Front Porch   48              Pixels
# H Sync          32              Pixels
# H Back Porch    80              Pixels
# H Freq          16.667          kHz
# V Total         280             Lines
# V Active        256             Lines
# V Blank         16              Lines
# V Front Porch   3               Lines
# V Sync          7               Lines
# V Back Porch    6               Lines
# V Freq          59.524          Hz


class VideoTiming(Elaboratable):
  def __init__(self):

    self.o_hsync = Signal()
    self.o_vsync = Signal()
    self.o_de = Signal()
    self.o_hpos = Signal(9)
    self.o_vpos = Signal(8)

  def elaborate(self, platform):
    m = Module()

    H_TOTAL       = 480
    H_ACTIVE      = 320
    H_BLANK       = 160
    H_FRONT_PORCH = 48
    H_SYNC        = 32
    H_BACK_PORCH  = 80
    V_TOTAL       = 280
    V_ACTIVE      = 256
    V_BLANK       = 16
    V_FRONT_PORCH = 3
    V_SYNC        = 7
    V_BACK_PORCH  = 6

    hcntr = Signal(range(H_TOTAL))
    vcntr = Signal(range(V_TOTAL))

    m.d.sync += [self.o_hsync.eq(0), self.o_vsync.eq(0)]
    with m.If(hcntr == (H_TOTAL - 1)):
      m.d.sync += [hcntr.eq(0), self.o_hsync.eq(1)]
      with m.If(vcntr == (V_TOTAL - 1)):
        m.d.sync += [vcntr.eq(0), self.o_vsync.eq(1)]
      with m.Else():
        m.d.sync += vcntr.eq(vcntr + 1)
    with m.Else():
      m.d.sync += hcntr.eq(hcntr + 1)

    # XXX: Could adjust these values (hcntr with -1) so that it can be synched instead of combinatorial?
    m.d.comb += self.o_de.eq(((H_SYNC + H_BACK_PORCH) <= hcntr) & (hcntr < (H_TOTAL - H_FRONT_PORCH)) &
                                   ((V_SYNC + V_BACK_PORCH) <= vcntr) & (vcntr < (V_TOTAL - V_FRONT_PORCH)))

#    with m.If(self.o_hsync):
#      m.d.sync += self.o_hpos.eq(0)
#    with m.Elif(self.o_de):
#      m.d.sync += self.o_hpos.eq(self.o_hpos + 1)
#
#    with m.If(self.o_vsync):
#      m.d.sync += self.o_vpos.eq(0)
#    with m.Elif(self.o_de):
#      m.d.sync += self.o_vpos.eq(self.o_vpos + 1)

    m.d.comb += [self.o_hpos.eq(hcntr), self.o_vpos.eq(vcntr)]

    return m

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
    self.o_chip_reg_addr = Signal(8)
    self.o_chip_reg_data = Signal(16)

    # Chip reg write access (ingress)
    self.i_chip_reg_addr = Signal(8)
    self.i_chip_reg_data = Signal(16)

  def elaborate(self, platform):
    m = Module()
    return m

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
    self.i_cpu_addr = Signal(32)
    self.i_cpu_data = Signal(16)
    self.o_cpu_data = Signal(16)
    self.i_cpu_req = Signal()
    self.i_cpu_we = Signal()
    self.o_cpu_ack = Signal()

    self.ports = [
        self.o_video_rgb, self.o_video_hsync, self.o_video_vsync, self.o_video_de,
        self.i_cpu_addr, self.i_cpu_data, self.o_cpu_data, self.i_cpu_req, self.i_cpu_we, self.o_cpu_ack,
    ]

  def elaborate(self, platform):
    m = Module()

    m.submodules.u_video = u_video = VideoTiming()
    m.submodules.u_copper = u_copper = Copper()

    m.d.comb += [
      self.o_video_hsync.eq(u_video.o_hsync),
      self.o_video_vsync.eq(u_video.o_vsync),
      self.o_video_de.eq(u_video.o_de),
    ]

    color = Signal(5)
    m.d.comb += color.eq(0) # Default background color

    with m.If(u_video.o_de & ((u_video.o_hpos[0:5] == 0) | (u_video.o_vpos[0:5] == 0))):
      m.d.comb += color.eq(1)

    chip_reg_addr = Signal(9)
    chip_reg_rdata = Signal(16)
    chip_reg_wdata = Signal(16)
    chip_reg_wen = Signal()

    # XXX: Copper should also have access to the chip_reg bus
    with m.If((self.i_cpu_addr[24:32] == 0xff) & self.i_cpu_req & self.i_cpu_we):
      m.d.comb += [
        chip_reg_addr.eq(self.i_cpu_addr[0:9]),
        chip_reg_wdata.eq(self.i_cpu_data),
        chip_reg_wen.eq(1),
        self.o_cpu_ack.eq(1),
      ]

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
