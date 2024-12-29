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

  *COLOR17 = 0xf00;
  *COLOR18 = 0x0f0;
  *COLOR19 = 0x00f;

  *SPR0POS = 0x8284;
  *SPR0CTL = 0xdd00;
  *SPR0DATA = 0xf00f;
  *SPR0DATB = 0x00ff;

  *SPR1POS = 0xa284;
  *SPR1CTL = 0xff00;
  *SPR1DATA = 0x0f0f;
  *SPR1DATB = 0x00ff;

  return 0;
}
