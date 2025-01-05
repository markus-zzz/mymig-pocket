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

int main(void) {

  uint16_t *chip_ram = (uint16_t *)CHIP_RAM;

  uint16_t *p = &chip_ram[0x100];

  struct CopperMove move = {.reg = COLOR00, .data = 0x000};
  struct CopperWait wait = {.ve = 0xff, .vp = 100, .he = 0xff, .hp = 0};

  // Black
  *p++ = copper_move_0(&move);
  *p++ = copper_move_1(&move);
  // Red
  *p++ = copper_wait_0(&wait);
  *p++ = copper_wait_1(&wait);
  move.data = 0xf00;
  *p++ = copper_move_0(&move);
  *p++ = copper_move_1(&move);
  // Green
  wait.vp = 120;
  *p++ = copper_wait_0(&wait);
  *p++ = copper_wait_1(&wait);
  move.data = 0x0f0;
  *p++ = copper_move_0(&move);
  *p++ = copper_move_1(&move);
  // Blue
  wait.vp = 140;
  *p++ = copper_wait_0(&wait);
  *p++ = copper_wait_1(&wait);
  move.data = 0x00f;
  *p++ = copper_move_0(&move);
  *p++ = copper_move_1(&move);
  // Black
  wait.vp = 160;
  *p++ = copper_wait_0(&wait);
  *p++ = copper_wait_1(&wait);
  move.data = 0x000;
  *p++ = copper_move_0(&move);
  *p++ = copper_move_1(&move);
  // EOL
  wait.ve = 0xff;
  wait.vp = 0xff;
  wait.he = 0xff;
  wait.hp = 0xff;
  *p++ = copper_wait_0(&wait);
  *p++ = copper_wait_1(&wait);

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = 0x100;

  *CHIP_REG(COPJMP1) = 0;

  return 0;
}
