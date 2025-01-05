/*
 * Copyright (C) 2024-2025 Markus Lavin (https://www.zzzconsulting.se/)
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "bios.h"

// Test sprite dma channels

int main(void) {
  uint16_t *chip_ram = (uint16_t *)CHIP_RAM;

  *CHIP_REG(COLOR17) = 0xf00;
  *CHIP_REG(COLOR18) = 0x0f0;
  *CHIP_REG(COLOR19) = 0x00f;

  uint16_t *p = &chip_ram[0x200];

  // Sprite #1 in DMA list
  {
    struct SPR spr0 = {
        .start_h = 145, .start_v = 150, .stop_v = 153, .attach = 0};
    *p++ = sprpos(&spr0);
    *p++ = sprctl(&spr0);
    for (int i = 0; i < (spr0.stop_v - spr0.start_v); i++) {
      *p++ = 0xf0f0;
      *p++ = 0xf0f0;
    }
  }
  // Sprite #2 in DMA list
  {
    struct SPR spr0 = {
        .start_h = 189, .start_v = 155, .stop_v = 168, .attach = 0};
    *p++ = sprpos(&spr0);
    *p++ = sprctl(&spr0);
    for (int i = 0; i < (spr0.stop_v - spr0.start_v); i++) {
      *p++ = 0x0000;
      *p++ = 0xff0f;
    }
  }
  // End of DMA list
  *p++ = 0;
  *p++ = 0;

  uint16_t *q = &chip_ram[0x100];

  {
    struct CopperMove move = {.reg = SPR0PTH, .data = 0x0};
    *q++ = copper_move_0(&move);
    *q++ = copper_move_1(&move);
  }
  {
    struct CopperMove move = {.reg = SPR0PTL, .data = 0x0200};
    *q++ = copper_move_0(&move);
    *q++ = copper_move_1(&move);
  }
  {
    struct CopperWait wait = {
        .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff}; // EOL
    *q++ = copper_wait_0(&wait);
    *q++ = copper_wait_1(&wait);
  }

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = 0x100;

  *CHIP_REG(COPJMP1) = 0;

  return 0;
}
