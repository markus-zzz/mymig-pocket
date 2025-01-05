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

  *CHIP_REG(COLOR17) = 0xf00;
  *CHIP_REG(COLOR18) = 0x0f0;
  *CHIP_REG(COLOR19) = 0x00f;

  struct SPR spr0 = {
      .start_h = 145, .start_v = 150, .stop_v = 180, .attach = 0};
  *CHIP_REG(SPR0POS) = sprpos(&spr0);
  *CHIP_REG(SPR0CTL) = sprctl(&spr0);
  *CHIP_REG(SPR0DATA) = 0xf00f;
  *CHIP_REG(SPR0DATB) = 0x00ff;

  struct SPR spr1 = {
      .start_h = 145, .start_v = 130, .stop_v = 170, .attach = 0};
  *CHIP_REG(SPR1POS) = sprpos(&spr1);
  *CHIP_REG(SPR1CTL) = sprctl(&spr1);
  *CHIP_REG(SPR1DATA) = 0x0f0f;
  *CHIP_REG(SPR1DATB) = 0x00ff;

  return 0;
}
