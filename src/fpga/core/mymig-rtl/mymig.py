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
