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
import enum
from video import *
from copper import Copper
from chipregs import *
from sprites import *
from bitplanes import *

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

    # IRQ
    self.o_irq = Signal()

    self.ports = [
      self.o_video_rgb, self.o_video_hsync, self.o_video_vsync, self.o_video_de,
      self.i_cpu_addr, self.i_cpu_data, self.o_cpu_data, self.i_cpu_req, self.i_cpu_we, self.o_cpu_ack,
      self.o_chip_ram_addr, self.i_chip_ram_data, self.o_chip_ram_data, self.o_chip_ram_we,
      self.o_irq,
    ]

  def elaborate(self, platform):
    m = Module()

    m.submodules.u_video = u_video = VideoTiming()
    m.submodules.u_copper = u_copper = Copper()
    m.submodules.u_bitplanes = u_bitplanes = Bitplanes()
    u_sprites = []
    for idx in range(8):
      sprite = Sprite(ChipReg.SPR0POS + idx * (ChipReg.SPR1POS-ChipReg.SPR0POS))
      m.submodules['sprite_{}'.format(idx)] = sprite
      u_sprites.append(sprite)


    chip_reg_addr = Signal(9)
    chip_reg_rdata = Signal(16)
    chip_reg_wdata = Signal(16)
    chip_reg_wen = Signal()

    m.d.comb += [
      u_copper.i_chip_ram_data.eq(self.i_chip_ram_data),
      u_copper.i_vsync.eq(u_video.o_vsync),
      u_copper.i_hpos.eq(u_video.o_hpos),
      u_copper.i_vpos.eq(u_video.o_vpos),
      u_copper.i_chip_reg_addr.eq(chip_reg_addr),
      u_copper.i_chip_reg_data.eq(chip_reg_wdata),
      u_copper.i_chip_reg_wen.eq(chip_reg_wen),
      u_bitplanes.i_chip_reg_addr.eq(chip_reg_addr),
      u_bitplanes.i_chip_reg_data.eq(chip_reg_wdata),
      u_bitplanes.i_chip_reg_wen.eq(chip_reg_wen),
    ]
    for sprite in u_sprites:
      m.d.comb += [
        sprite.i_chip_reg_addr.eq(chip_reg_addr),
        sprite.i_chip_reg_data.eq(chip_reg_wdata),
        sprite.i_chip_reg_wen.eq(chip_reg_wen),
        sprite.i_hpos.eq(u_video.o_hpos),
        sprite.i_vpos.eq(u_video.o_vpos),
      ]

    #
    # BPLCON0 register
    #
    bplcon0 = Signal(BPLCON0_Layout)
    with m.If((chip_reg_addr == ChipReg.BPLCON0) & chip_reg_wen):
      m.d.sync += bplcon0.as_value().eq(chip_reg_wdata)

    #
    # DIWSTRT and DIWSTOP registers
    #
    diwstrt = Signal(DIW_Layout)
    diwstop = Signal(DIW_Layout)
    with m.If((chip_reg_addr == ChipReg.DIWSTRT) & chip_reg_wen):
      m.d.sync += diwstrt.as_value().eq(chip_reg_wdata)
    with m.If((chip_reg_addr == ChipReg.DIWSTOP) & chip_reg_wen):
      m.d.sync += diwstop.as_value().eq(chip_reg_wdata)

    #
    # DDFSTRT and DDFSTOP registers
    #
    ddfstrt = Signal(DDF_Layout)
    ddfstop = Signal(DDF_Layout)
    with m.If((chip_reg_addr == ChipReg.DDFSTRT) & chip_reg_wen):
      m.d.sync += ddfstrt.as_value().eq(chip_reg_wdata)
    with m.If((chip_reg_addr == ChipReg.DDFSTOP) & chip_reg_wen):
      m.d.sync += ddfstop.as_value().eq(chip_reg_wdata)

    #
    # BPLxPTL and BPLxPTH registers
    #
    bplxpt_array = Array([Signal(18) for _ in range(6)])

    #
    # SPRxPTL and SPRxPTH registers
    #
    sprxpt_array = Array([Signal(18) for _ in range(8)])

    #
    # Graphics DMA
    #
    gfx_dma_chip_ram_req = Signal() # Highest prority and always granted so no need for ack
    gfx_dma_chip_ram_addr = Signal(20)

    gfx_dma_chip_reg_addr = Signal(9)
    gfx_dma_chip_reg_wdata = Signal(16)
    gfx_dma_chip_reg_wen = Signal()

    sprite_idx = Signal(range(8))
    bitplane_idx = Signal(range(16))

    class SpriteState(enum.Enum):
      IDLE = 0
      LOAD_POSCTL = 1
      WAIT = 2
      LOAD_DAT = 3
    sprite_states = Array([Signal(SpriteState, reset=SpriteState.IDLE) for _ in range(8)])
    for idx, ss in enumerate(sprite_states):
      s = Signal(ss.shape(), name='sprite_state_{}'.format(idx))
      m.d.comb += s.eq(ss)

    with m.FSM(reset='idle') as gfx_dma_fsm:

      with m.State('idle'):
        with m.If(u_video.o_hpos == 10):
          m.d.sync += [
            sprite_idx.eq(0),
          ]
          m.next = 'prepare'

      with m.State('prepare'):
        for idx, ss in enumerate(sprite_states):
          with m.If(sprxpt_array[idx] != 0):
            with m.Switch(ss):
              with m.Case(SpriteState.IDLE):
                m.d.sync += [
                  ss.eq(SpriteState.LOAD_POSCTL),
                ]
              with m.Case(SpriteState.WAIT):
                with m.If(u_sprites[idx].o_vstart_match & ~u_sprites[idx].o_vstop_match):
                  m.d.sync += [
                    ss.eq(SpriteState.LOAD_DAT),
                  ]
              with m.Case(SpriteState.LOAD_DAT):
                with m.If(u_sprites[idx].o_vstop_match):
                  m.d.sync += [
                    ss.eq(SpriteState.LOAD_POSCTL),
                  ]
        m.next = 'sprite_dma_0'

      with m.State('sprite_dma_0'):
        with m.If(sprite_states[sprite_idx] == SpriteState.LOAD_POSCTL):
          m.d.comb += [
            gfx_dma_chip_ram_req.eq(1),
            gfx_dma_chip_ram_addr.eq(sprxpt_array[sprite_idx]),
            gfx_dma_chip_reg_wen.eq(1),
            gfx_dma_chip_reg_addr.eq(ChipReg.SPR0POS + sprite_idx * (ChipReg.SPR1POS - ChipReg.SPR0POS)),
            gfx_dma_chip_reg_wdata.eq(self.i_chip_ram_data),
          ]
        with m.If(sprite_states[sprite_idx] == SpriteState.LOAD_DAT):
          m.d.comb += [
            gfx_dma_chip_ram_req.eq(1),
            gfx_dma_chip_ram_addr.eq(sprxpt_array[sprite_idx]),
            gfx_dma_chip_reg_wen.eq(1),
            gfx_dma_chip_reg_addr.eq(ChipReg.SPR0DATA + sprite_idx * (ChipReg.SPR1DATA - ChipReg.SPR0DATA)),
            gfx_dma_chip_reg_wdata.eq(self.i_chip_ram_data),
          ]
        with m.If((sprite_states[sprite_idx] == SpriteState.LOAD_POSCTL) |
                  (sprite_states[sprite_idx] == SpriteState.LOAD_DAT)):
          m.d.sync += [
            sprxpt_array[sprite_idx].eq(sprxpt_array[sprite_idx] + 1),
          ]
        m.next = 'sprite_dma_1'

      with m.State('sprite_dma_1'):
        with m.If(sprite_states[sprite_idx] == SpriteState.LOAD_POSCTL):
          m.d.comb += [
            gfx_dma_chip_ram_req.eq(1),
            gfx_dma_chip_ram_addr.eq(sprxpt_array[sprite_idx]),
            gfx_dma_chip_reg_wen.eq(1),
            gfx_dma_chip_reg_addr.eq(ChipReg.SPR0CTL + sprite_idx * (ChipReg.SPR1CTL - ChipReg.SPR0CTL)),
            gfx_dma_chip_reg_wdata.eq(self.i_chip_ram_data),
          ]
          m.d.sync += [
            sprite_states[sprite_idx].eq(SpriteState.WAIT),
          ]
        with m.If(sprite_states[sprite_idx] == SpriteState.LOAD_DAT):
          m.d.comb += [
            gfx_dma_chip_ram_req.eq(1),
            gfx_dma_chip_ram_addr.eq(sprxpt_array[sprite_idx]),
            gfx_dma_chip_reg_wen.eq(1),
            gfx_dma_chip_reg_addr.eq(ChipReg.SPR0DATB + sprite_idx * (ChipReg.SPR1DATB - ChipReg.SPR0DATB)),
            gfx_dma_chip_reg_wdata.eq(self.i_chip_ram_data),
          ]
        with m.If((sprite_states[sprite_idx] == SpriteState.LOAD_POSCTL) |
                  (sprite_states[sprite_idx] == SpriteState.LOAD_DAT)):
          m.d.sync += [
            sprxpt_array[sprite_idx].eq(sprxpt_array[sprite_idx] + 1),
          ]
        m.d.sync += [
          sprite_idx.eq(sprite_idx + 1),
        ]
        with m.If(sprite_idx == 7):
          m.next = 'bitplane_dma_0'
        with m.Else():
          m.next = 'sprite_dma_0'

      with m.State('bitplane_dma_0'):
        with m.If((u_video.o_vpos >= diwstrt.v0_v7) & (u_video.o_vpos < Cat(diwstop.v0_v7, C(1,1)))):
          with m.If(u_video.o_hpos == Cat(C(0,3), ddfstrt.h3_h8)):
            m.d.sync += [
              bitplane_idx.eq(15),
            ]
            m.next = 'bitplane_dma_1'
        with m.If(u_video.o_hpos == 0): # Restart on new line
          m.next = 'idle'

      with m.State('bitplane_dma_1'):
        m.d.sync += [
          bitplane_idx.eq(bitplane_idx - 1),
        ]
        with m.If(bitplane_idx < bplcon0.bpu):
          m.d.comb += [
            gfx_dma_chip_ram_req.eq(1),
            gfx_dma_chip_ram_addr.eq(bplxpt_array[bitplane_idx]),
            gfx_dma_chip_reg_wen.eq(1),
            gfx_dma_chip_reg_addr.eq(ChipReg.BPL1DAT + bitplane_idx * (ChipReg.BPL2DAT - ChipReg.BPL1DAT)),
            gfx_dma_chip_reg_wdata.eq(self.i_chip_ram_data),
          ]
          m.d.sync += [
            bplxpt_array[bitplane_idx].eq(bplxpt_array[bitplane_idx] + 1),
          ]
        with m.If(u_video.o_hpos == Cat(C(0,3), ddfstop.h3_h8)):
          m.next = 'idle'

    # All sprites go back to IDLE on vsync
    with m.If(u_video.o_vsync):
      for ss in sprite_states:
        m.d.sync += [
          ss.eq(SpriteState.IDLE),
        ]

    #
    # BPLxPTL and BPLxPTH registers
    #
    # Generate address decode logic for writes
    for idx, bplpt in enumerate(bplxpt_array):
      with m.If((chip_reg_addr == (ChipReg.BPL1PTH + idx * (ChipReg.BPL2PTH - ChipReg.BPL1PTH))) & chip_reg_wen):
        m.d.sync += bplpt[16:].eq(chip_reg_wdata)
      with m.If((chip_reg_addr == (ChipReg.BPL1PTL + idx * (ChipReg.BPL2PTL - ChipReg.BPL1PTL))) & chip_reg_wen):
        m.d.sync += bplpt[0:16].eq(chip_reg_wdata)
      s = Signal(bplpt.shape(), name='bpl{}pt'.format(idx))
      m.d.comb += s.eq(bplpt)

    #
    # SPRxPTL and SPRxPTH registers
    #
    # Generate address decode logic for writes
    for idx, sprpt in enumerate(sprxpt_array):
      with m.If((chip_reg_addr == (ChipReg.SPR0PTH + idx * (ChipReg.SPR1PTH - ChipReg.SPR0PTH))) & chip_reg_wen):
        m.d.sync += sprpt[16:].eq(chip_reg_wdata)
      with m.If((chip_reg_addr == (ChipReg.SPR0PTL + idx * (ChipReg.SPR1PTL - ChipReg.SPR0PTL))) & chip_reg_wen):
        m.d.sync += sprpt[0:16].eq(chip_reg_wdata)
      s = Signal(sprpt.shape(), name='spr{}pt'.format(idx))
      m.d.comb += s.eq(sprpt)



    #
    # Chip RAM arbitration - begin
    #
    with m.If(gfx_dma_chip_ram_req):
      m.d.comb += [
        self.o_chip_ram_addr.eq(gfx_dma_chip_ram_addr),
        # Highest prio and always granted, so no ack
      ]
    with m.Elif(u_copper.o_chip_ram_req): # Copper (R)
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

    #
    # Chip REG arbitration - begin
    #
    with m.If(gfx_dma_chip_reg_wen):
      m.d.comb += [
        chip_reg_addr.eq(gfx_dma_chip_reg_addr),
        chip_reg_wdata.eq(gfx_dma_chip_reg_wdata),
        chip_reg_wen.eq(1),
        # Highest prio and always granted, so no ack
      ]
    with m.Elif(u_copper.o_chip_reg_wen):
      m.d.comb += [
        chip_reg_addr.eq(u_copper.o_chip_reg_addr),
        chip_reg_wdata.eq(u_copper.o_chip_reg_data),
        chip_reg_wen.eq(u_copper.o_chip_reg_wen),
        u_copper.i_chip_reg_ack.eq(1),
      ]
    with m.Elif((self.i_cpu_addr[12:24] == 0xdff) & self.i_cpu_req):
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


    m.d.comb += [
      self.o_video_hsync.eq(u_video.o_hsync),
      self.o_video_vsync.eq(u_video.o_vsync),
      self.o_video_de.eq(u_video.o_de),
    ]

    color = Signal(5)
    m.d.comb += color.eq(0) # Default background color

    # XXX: Should be only if bitplanes/playfields are enabled
    m.d.comb += color.eq(u_bitplanes.o_color)

    # XXX: Sprites and Playfields to override color
    # XXX: Sprite priority is actually opposite order (0 is highest)
    for idx in range(0, 8, 2):
      esprite = u_sprites[idx] # Even sprite
      osprite = u_sprites[idx+1] # Odd sprite

      with m.If(osprite.o_attach): # Sprite in attach mode
        acolor = Signal(4, name='acolor_{}_{}'.format(idx, idx+1))
        m.d.comb += acolor.eq(Cat(esprite.o_color, osprite.o_color))
        with m.If(sprite.o_attach & (acolor != 0)):
          m.d.comb += color.eq(acolor + 16)

      with m.Else(): # Sprite in indepedent mode
        with m.If(esprite.o_color != 0):
          m.d.comb += color.eq(esprite.o_color + 16 + (idx >> 1) * 4)
        with m.If(osprite.o_color != 0):
          m.d.comb += color.eq(osprite.o_color + 16 + (idx >> 1) * 4)

    # Registers COLOR00-COLOR31
    color_palette = Array([Signal(12) for _ in range(32)])
    # Generate address decode logic for writes
    for idx, color_reg in enumerate(color_palette):
      with m.If((chip_reg_addr == (ChipReg.COLOR00 + idx * 2)) & chip_reg_wen):
        m.d.sync += color_reg.eq(chip_reg_wdata)
      s = Signal(color_reg.shape(), name='color_reg_{}'.format(idx))
      m.d.comb += s.eq(color_reg)

    color_rgb = Signal(12)
    m.d.comb += color_rgb.eq(color_palette[color])
    m.d.comb += self.o_video_rgb.eq(Cat(C(0,4), color_rgb[0:4], C(0,4), color_rgb[4:8], C(0,4), color_rgb[8:12]))

    #
    # INTEN and INTREQ
    #
    intena = Signal(15)
    intreq = Signal(14)
    m.d.comb += self.o_irq.eq(intena[14] & (intena & intreq).any())
    with m.If((chip_reg_addr == ChipReg.INTENA) & chip_reg_wen):
      with m.If(chip_reg_wdata[15]): # Set
        m.d.sync += intena.eq(intena | chip_reg_wdata)
      with m.Else(): # Clear
        m.d.sync += intena.eq(intena & ~chip_reg_wdata)
    with m.If((chip_reg_addr == ChipReg.INTREQ) & chip_reg_wen):
      with m.If(chip_reg_wdata[15]): # Set
        m.d.sync += intreq.eq(intreq | chip_reg_wdata)
      with m.Else(): # Clear
        m.d.sync += intreq.eq(intreq & ~chip_reg_wdata)
    # HW updates have higher priority than SW updates

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
