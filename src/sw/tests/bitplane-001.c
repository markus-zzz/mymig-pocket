/*
 * Copyright (C) 2025 Markus Lavin (https://www.zzzconsulting.se/)
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

// Test Bitplane DMA

#define BP_DIM_X 288

static inline void set_pixel(uint16_t *bp, uint16_t x, uint16_t y) {
  bp += y * BP_DIM_X / 16;
  bp += x / 16;
  *bp |= 1 << (15 - x % 16);
}

int main(void) {
  uint16_t *chip_ram = (uint16_t *)CHIP_RAM;

  uint16_t *p0 = &chip_ram[0x1000];
  uint16_t *p1 = &chip_ram[0x2000];
#if 0
  for (int i = 0; i < 0x1000; i++) {
    p0[i] = 0x0ff0 | i;
    p1[i] = 0xf00f | i;
  }
#else
  p0[300] = p1[300] = 1;
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      set_pixel(p0, 50 + i, 50 + j);
    }
  }
#endif

  *CHIP_REG(COLOR01) = 0xfff;
  *CHIP_REG(COLOR02) = 0x3ff;
  *CHIP_REG(COLOR03) = 0xff7;
  *CHIP_REG(COLOR17) = 0xf00;
  *CHIP_REG(COLOR18) = 0x0f0;
  *CHIP_REG(COLOR19) = 0x00f;

  uint16_t *q = &chip_ram[0x100];

  COP_MOVE(q, .reg = BPL1PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL1PTL, .data = 0x1000);
  COP_MOVE(q, .reg = BPL2PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL2PTL, .data = 0x2000);
  COP_WAIT(q, .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff); // EOL

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = 0x100;

  *CHIP_REG(COPJMP1) = 0;

  *CHIP_REG(BPLCON0) = 2 << 12;
  *CHIP_REG(DIWSTRT) = 0x2080;
  *CHIP_REG(DIWSTOP) = 0x22a0;
  *CHIP_REG(DDFSTRT) = 0x80 >> 1;
  *CHIP_REG(DDFSTOP) = 0x1a0 >> 1;

  return 0;
}
