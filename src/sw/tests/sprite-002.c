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

  *COLOR17 = 0xf00;
  *COLOR18 = 0x0f0;
  *COLOR19 = 0x00f;

  uint16_t *p = &chip_ram[0x200];

  // Sprite #1 in DMA list
  {
    struct SPR spr0 = {.start_h = 145, .start_v = 150, .stop_v = 153, .attach = 0};
    *p++ = sprpos(&spr0);
    *p++ = sprctl(&spr0);
    for (int i = 0; i < (spr0.stop_v - spr0.start_v); i++) {
      *p++ = 0xf0f0;
      *p++ = 0xf0f0;
    }
  }
  // Sprite #2 in DMA list
  {
    struct SPR spr0 = {.start_h = 189, .start_v = 155, .stop_v = 168, .attach = 0};
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
  // SPR0PTH = 0x0
  q = copper_move(q, 0x120, 0x0000);
  // SPR0PTL = 0x200
  q = copper_move(q, 0x122, 0x0200);
  // EOL
  q = copper_wait(q, 0xff, 0xff, 0xff, 0xff);

  *COP1LCH = 0;
  *COP1LCL = 0x100;

  *COPJMP1 = 0;

  return 0;
}
