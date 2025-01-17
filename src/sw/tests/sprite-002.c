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

#define SIZE(x) (sizeof(x) / sizeof(x[0]))

const char *heart[] = {"0000000000000000",
                       "0000000110000000",
                       "0000012222100000",
                       "0000000110000000",
                       "0000000000000000",
                       "0011211331121100",
                       "0001111221111000",
                       "0000011221100000",
                       "0000000110000000",
                       "0000000000000000",
                       NULL};

uint16_t *ascii2sprite(unsigned depth, const char *strs[], uint16_t *sprite) {
  for (unsigned i = 0; strs[i] != NULL; i++) {
    uint16_t shifts[4];
    for (unsigned j = 0; j < 16; j++) {
      int d = strs[i][j] - '0';
      for (unsigned k = 0; k < depth; k++) {
        shifts[k] <<= 1;
        if (d & (1 << (depth - 1 - k))) {
          shifts[k] |= 1;
        }
      }
    }
    for (unsigned k = 0; k < depth; k++) {
      *sprite++ = shifts[k];
    }
  }
  return sprite;
}

int main(void) {
  uint16_t *chip_ram = (uint16_t *)CHIP_RAM;

  *CHIP_REG(COLOR17) = 0xf00;
  *CHIP_REG(COLOR18) = 0x0f0;
  *CHIP_REG(COLOR19) = 0x00f;

  uint16_t *p = &chip_ram[0x200];

  // Sprite #1 in DMA list
  {
    struct SPR spr0 = {.start_h = 145,
                       .start_v = 150,
                       .stop_v = 150 + SIZE(heart) - 1,
                       .attach = 0};
    *p++ = sprpos(&spr0);
    *p++ = sprctl(&spr0);
    p = ascii2sprite(2, heart, p);
  }
  // Sprite #2 in DMA list
  {
    struct SPR spr0 = {
        .start_h = 189, .start_v = 165, .stop_v = 168, .attach = 0};
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

  COP_MOVE(q, .reg = SPR0PTH, .data = 0x0);
  COP_MOVE(q, .reg = SPR0PTL, .data = 0x0200);
  COP_WAIT(q, .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff); // EOL

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = 0x100;

  *CHIP_REG(COPJMP1) = 0;

  return 0;
}
