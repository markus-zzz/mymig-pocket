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

  // Black
  COP_MOVE(p, .reg = COLOR00, .data = 0x000);
  // Red
  COP_WAIT(p, .ve = 0xff, .vp = 100, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0xf00);
  // Green
  COP_WAIT(p, .ve = 0xff, .vp = 120, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0x0f0);
  // Blue
  COP_WAIT(p, .ve = 0xff, .vp = 140, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0x00f);
  // Black
  COP_WAIT(p, .ve = 0xff, .vp = 160, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0x000);
  // EOL
  COP_WAIT(p, .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff);

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = 0x100;

  *CHIP_REG(COPJMP1) = 0;

  return 0;
}
