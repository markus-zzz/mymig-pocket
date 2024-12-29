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

  uint16_t *pos = &chip_ram[0x100];
  // Black
  pos = copper_move(pos, 0x180, 0x000);
  // Red
  pos = copper_wait(pos, 0xff, 100, 0xff, 0);
  pos = copper_move(pos, 0x180, 0xf00);
  // Green
  pos = copper_wait(pos, 0xff, 120, 0xff, 0);
  pos = copper_move(pos, 0x180, 0x0f0);
  // Blue
  pos = copper_wait(pos, 0xff, 140, 0xff, 0);
  pos = copper_move(pos, 0x180, 0x00f);
  // Black
  pos = copper_wait(pos, 0xff, 160, 0xff, 0);
  pos = copper_move(pos, 0x180, 0x000);
  // EOL
  pos = copper_wait(pos, 0xff, 0xff, 0xff, 0xff);

  *COP1LCH = 0;
  *COP1LCL = 0x100;

  *COPJMP1 = 0;

  return 0;
}
