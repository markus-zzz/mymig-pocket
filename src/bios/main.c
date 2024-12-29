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

#include <stdint.h>

#define TIMER_TIMEOUT 80000
volatile uint32_t timer_ticks;

void irq_mask(uint32_t mask);
void timer_start(uint32_t timeout);

volatile uint16_t *chip_ram = (volatile uint16_t *)0xff000000;
volatile uint16_t *color00 = (volatile uint16_t *)0xffdff180;
volatile uint16_t *color01 = (volatile uint16_t *)0xffdff182;
volatile uint16_t *color02 = (volatile uint16_t *)0xffdff184;
volatile uint16_t *color03 = (volatile uint16_t *)0xffdff186;

volatile uint16_t *color19 = (volatile uint16_t *)0xffdff1a6;


volatile uint16_t *cop1lch = (volatile uint16_t *)0xffdff080;
volatile uint16_t *cop1lcl = (volatile uint16_t *)0xffdff082;
volatile uint16_t *cop2lch = (volatile uint16_t *)0xffdff084;
volatile uint16_t *cop2lcl = (volatile uint16_t *)0xffdff086;
volatile uint16_t *copjmp1 = (volatile uint16_t *)0xffdff088;
volatile uint16_t *copjmp2 = (volatile uint16_t *)0xffdff08A;

volatile uint16_t *spr0pos = (volatile uint16_t *)0xffdff140;
volatile uint16_t *spr0ctl = (volatile uint16_t *)0xffdff142;
volatile uint16_t *spr0data =(volatile uint16_t *)0xffdff144;
volatile uint16_t *spr0datb =(volatile uint16_t *)0xffdff146;

volatile uint16_t *copper_move(volatile uint16_t *instr_addr, uint16_t reg,
                               uint16_t data) {
  instr_addr[0] = ((reg & 0x1ff) >> 1) << 1;
  instr_addr[1] = data;
  return &instr_addr[2];
}

volatile uint16_t *copper_wait(volatile uint16_t *instr_addr, uint8_t ve,
                               uint8_t vp, uint8_t he, uint8_t hp) {
  instr_addr[0] = ((uint16_t)vp << 8) | ((uint16_t)(hp & 0x7f) << 1) | 1;
  instr_addr[1] =
      ((uint16_t)(ve & 0x7f) << 8) | ((uint16_t)(he & 0x7f) << 1) | 0;
  return &instr_addr[2];
}

int main(void) {

  chip_ram[0x100] = 0x23 << 1;
  chip_ram[0x101] = 0xcafe;

  volatile uint16_t *pos = &chip_ram[0x100];
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

  *cop1lch = 0;
  *cop1lcl = 0x100;

  *copjmp1 = 0;

  *color19 = 0xfff;

  *spr0pos = 0x8284;
  *spr0ctl = 0xff00;
  *spr0data = 0xffff;
  *spr0datb = 0xffff;

  return 0;
}

uint32_t *irq(uint32_t *regs, uint32_t irqs) {
  timer_start(TIMER_TIMEOUT);
  timer_ticks++;

  return regs;
}
