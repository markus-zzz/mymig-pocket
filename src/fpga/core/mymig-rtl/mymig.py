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


class MyMig(Elaboratable):
  def __init__(self):

    self.o_video_rgb = Signal(24)
    self.o_video_hsync = Signal()
    self.o_video_vsync = Signal()
    self.o_video_de = Signal()

    self.ports = [
        self.o_video_rgb, self.o_video_hsync, self.o_video_vsync, self.o_video_de
    ]

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

    m.d.sync += [self.o_video_hsync.eq(0), self.o_video_vsync.eq(0)]
    with m.If(hcntr == (H_TOTAL - 1)):
      m.d.sync += [hcntr.eq(0), self.o_video_hsync.eq(1)]
      with m.If(vcntr == (V_TOTAL - 1)):
        m.d.sync += [vcntr.eq(0), self.o_video_vsync.eq(1)]
      with m.Else():
        m.d.sync += vcntr.eq(vcntr + 1)
    with m.Else():
      m.d.sync += hcntr.eq(hcntr + 1)

    # XXX: Could adjust these values (hcntr with -1) so that it can be synched instead of combinatorial?
    m.d.comb += self.o_video_de.eq(((H_SYNC + H_BACK_PORCH) <= hcntr) & (hcntr < (H_TOTAL - H_FRONT_PORCH)) &
                                   ((V_SYNC + V_BACK_PORCH) <= vcntr) & (vcntr < (V_TOTAL - V_FRONT_PORCH)))

    m.d.comb += self.o_video_rgb.eq(Mux(self.o_video_de & (hcntr[0:5] == 0), 0xff0000, 0x000000))

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
