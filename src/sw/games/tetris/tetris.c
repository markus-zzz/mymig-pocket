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

#define BP_DIM_X 288

#define GRID_X 10
#define GRID_Y 7 // 20

#define BP1_BASE 0x1000
#define BP2_BASE 0x2000
#define BP3_BASE 0x3000

struct Coord {
  unsigned int x : 4;
  unsigned int y : 4;
};

// The four rotation states for each of the seven pieces with origin in lower
// left corner.

struct Coord piece_I[4][4] = {{{0, 2}, {1, 2}, {2, 2}, {3, 2}},
                              {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
                              {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
                              {{1, 0}, {1, 1}, {1, 2}, {1, 3}}};

uint16_t grid[GRID_X][GRID_Y];

const char *example[] = {
    // clang-format off
  "0000000000",
  "0000000000",
  "0000550600",
  "0000550600",
  "4400000633",
  "0440002603",
  "1111022203",
    // clang-format on
};

static inline void set_pixel(uint16_t *bp, uint16_t x, uint16_t y) {
  bp += y * BP_DIM_X / 16;
  bp += x / 16;
  *bp |= 1 << (15 - x % 16);
}

static inline void set_square(uint16_t *bp, uint16_t x, uint16_t y) {
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      set_pixel(bp, x * 10 + i, y * 10 + j);
    }
  }
}

int main(void) {
  uint16_t *chip_ram = (uint16_t *)CHIP_RAM;

  uint16_t *p1 = &chip_ram[BP1_BASE];
  uint16_t *p2 = &chip_ram[BP2_BASE];
  uint16_t *p3 = &chip_ram[BP3_BASE];

  for (int y = 0; y < GRID_Y; y++) {
    for (int x = 0; x < GRID_X; x++) {
      const char *p = example[y];
      grid[x][y] = p[x] - '0';
    }
  }

  for (int y = 0; y < GRID_Y; y++) {
    for (int x = 0; x < GRID_X; x++) {
      if (grid[x][y]) {
        set_square(p1, x, y);
        if (x == 0 || grid[x][y] != grid[x - 1][y]) { // Shade west side
          for (int i = 0; i < 10; i++)
            set_pixel(p2, 10 * x, 10 * y + i);
        }
        if (y == 0 || grid[x][y] != grid[x][y - 1]) { // Shade north side
          for (int i = 0; i < 10; i++)
            set_pixel(p2, 10 * x + i, 10 * y);
        }
        if (x == GRID_X - 1 ||
            grid[x][y] != grid[x + 1][y]) { // Shade east side
          for (int i = 0; i < 10; i++)
            set_pixel(p3, 10 * x + 9, 10 * y + i);
        }
        if (y == GRID_Y - 1 ||
            grid[x][y] != grid[x][y + 1]) { // Shade south side
          for (int i = 0; i < 10; i++)
            set_pixel(p3, 10 * x + i, 10 * y + 9);
        }
      }
    }
  }

  *CHIP_REG(COLOR01) = 0x289; // 001 - Normal
  *CHIP_REG(COLOR02) = 0x3ff;
  *CHIP_REG(COLOR03) = 0x1bc; // 011 - Light shade
  *CHIP_REG(COLOR04) = 0xfff;
  *CHIP_REG(COLOR05) = 0x145; // 101 - Dark shade
  *CHIP_REG(COLOR06) = 0xfff;
  *CHIP_REG(COLOR07) = 0x289; // 111 - Middle shade

  *CHIP_REG(COLOR17) = 0xf00;
  *CHIP_REG(COLOR18) = 0x0f0;
  *CHIP_REG(COLOR19) = 0x00f;

  uint16_t *q = &chip_ram[0x100];

  COP_MOVE(q, .reg = BPL1PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL1PTL, .data = BP1_BASE);
  COP_MOVE(q, .reg = BPL2PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL2PTL, .data = BP2_BASE);
  COP_MOVE(q, .reg = BPL3PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL3PTL, .data = BP3_BASE);
  COP_WAIT(q, .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff); // EOL

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = 0x100;

  *CHIP_REG(COPJMP1) = 0;

  *CHIP_REG(BPLCON0) = 3 << 12;
  *CHIP_REG(DIWSTRT) = 0x2080;
  *CHIP_REG(DIWSTOP) = 0x22a0;
  *CHIP_REG(DDFSTRT) = 0x80 >> 1;
  *CHIP_REG(DDFSTOP) = 0x1a0 >> 1;

  return 0;
}
