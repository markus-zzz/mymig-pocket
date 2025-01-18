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

// np.int8(np.sin(2*np.pi*np.linspace(0, 1, 256, endpoint=False))*127)

const int8_t sinetable[] = {
    0,    3,    6,    9,    12,   15,   18,   21,   24,   27,   30,   33,
    36,   39,   42,   45,   48,   51,   54,   57,   59,   62,   65,   67,
    70,   73,   75,   78,   80,   82,   85,   87,   89,   91,   94,   96,
    98,   100,  102,  103,  105,  107,  108,  110,  112,  113,  114,  116,
    117,  118,  119,  120,  121,  122,  123,  123,  124,  125,  125,  126,
    126,  126,  126,  126,  127,  126,  126,  126,  126,  126,  125,  125,
    124,  123,  123,  122,  121,  120,  119,  118,  117,  116,  114,  113,
    112,  110,  108,  107,  105,  103,  102,  100,  98,   96,   94,   91,
    89,   87,   85,   82,   80,   78,   75,   73,   70,   67,   65,   62,
    59,   57,   54,   51,   48,   45,   42,   39,   36,   33,   30,   27,
    24,   21,   18,   15,   12,   9,    6,    3,    0,    -3,   -6,   -9,
    -12,  -15,  -18,  -21,  -24,  -27,  -30,  -33,  -36,  -39,  -42,  -45,
    -48,  -51,  -54,  -57,  -59,  -62,  -65,  -67,  -70,  -73,  -75,  -78,
    -80,  -82,  -85,  -87,  -89,  -91,  -94,  -96,  -98,  -100, -102, -103,
    -105, -107, -108, -110, -112, -113, -114, -116, -117, -118, -119, -120,
    -121, -122, -123, -123, -124, -125, -125, -126, -126, -126, -126, -126,
    -127, -126, -126, -126, -126, -126, -125, -125, -124, -123, -123, -122,
    -121, -120, -119, -118, -117, -116, -114, -113, -112, -110, -108, -107,
    -105, -103, -102, -100, -98,  -96,  -94,  -91,  -89,  -87,  -85,  -82,
    -80,  -78,  -75,  -73,  -70,  -67,  -65,  -62,  -59,  -57,  -54,  -51,
    -48,  -45,  -42,  -39,  -36,  -33,  -30,  -27,  -24,  -21,  -18,  -15,
    -12,  -9,   -6,   -3};

static uint8_t ticks = 0;

void setup_copper() {
  uint16_t *chip_ram = (uint16_t *)CHIP_RAM;

  uint16_t *p = &chip_ram[0x100];

  int8_t mod = sinetable[ticks] / 8;

  // Black
  COP_MOVE(p, .reg = COLOR00, .data = 0x000);
  // Red
  COP_WAIT(p, .ve = 0xff, .vp = 100 + mod, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0xf00);
  // Green
  COP_WAIT(p, .ve = 0xff, .vp = 120, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0x0f0);
  // Blue
  COP_WAIT(p, .ve = 0xff, .vp = 140, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0x00f);
  // Black
  COP_WAIT(p, .ve = 0xff, .vp = 160 + mod, .he = 0xff, .hp = 0);
  COP_MOVE(p, .reg = COLOR00, .data = 0x000);
  // Generate copper interrupt
  COP_MOVE(p, .reg = INTREQ, .data = 0x8010);
  // EOL
  COP_WAIT(p, .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff);
}

int main(void) {

  irq_mask(0);
  ticks = 0;
  setup_copper();

  *CHIP_REG(INTENA) = 0xc010; // Enable interrupts (master enable and copper)

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = 0x100;

  *CHIP_REG(COPJMP1) = 0;

  return 0;
}

uint32_t *irq(uint32_t *regs, uint32_t irqs) {
  *CHIP_REG(INTREQ) = 0x0010; // Ack/clear the copper interrupt
  ticks++;
  setup_copper();
  return regs;
}
