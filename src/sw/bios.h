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

#pragma once

#include <stdint.h>

#define NULL ((void *)0)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define IRQ_ENABLE() irq_mask(0)
#define IRQ_DISABLE() irq_mask(-1)

#define CHIP_RAM (volatile uint16_t *)0xff000000
#define CHIP_REG 0xffdff000

#define COLOR00 ((volatile uint16_t *)(CHIP_REG + 0x180))
#define COLOR01 ((volatile uint16_t *)(CHIP_REG + 0x182))
#define COLOR02 ((volatile uint16_t *)(CHIP_REG + 0x184))
#define COLOR03 ((volatile uint16_t *)(CHIP_REG + 0x186))
#define COLOR04 ((volatile uint16_t *)(CHIP_REG + 0x188))
#define COLOR05 ((volatile uint16_t *)(CHIP_REG + 0x18a))
#define COLOR06 ((volatile uint16_t *)(CHIP_REG + 0x18c))
#define COLOR07 ((volatile uint16_t *)(CHIP_REG + 0x18e))
#define COLOR08 ((volatile uint16_t *)(CHIP_REG + 0x190))
#define COLOR09 ((volatile uint16_t *)(CHIP_REG + 0x192))
#define COLOR10 ((volatile uint16_t *)(CHIP_REG + 0x194))
#define COLOR11 ((volatile uint16_t *)(CHIP_REG + 0x196))
#define COLOR12 ((volatile uint16_t *)(CHIP_REG + 0x198))
#define COLOR13 ((volatile uint16_t *)(CHIP_REG + 0x19a))
#define COLOR14 ((volatile uint16_t *)(CHIP_REG + 0x19c))
#define COLOR15 ((volatile uint16_t *)(CHIP_REG + 0x19e))
#define COLOR16 ((volatile uint16_t *)(CHIP_REG + 0x1a0))
#define COLOR17 ((volatile uint16_t *)(CHIP_REG + 0x1a2))
#define COLOR18 ((volatile uint16_t *)(CHIP_REG + 0x1a4))
#define COLOR19 ((volatile uint16_t *)(CHIP_REG + 0x1a6))
#define COLOR20 ((volatile uint16_t *)(CHIP_REG + 0x1a8))
#define COLOR21 ((volatile uint16_t *)(CHIP_REG + 0x1aa))
#define COLOR22 ((volatile uint16_t *)(CHIP_REG + 0x1ac))
#define COLOR23 ((volatile uint16_t *)(CHIP_REG + 0x1ae))
#define COLOR24 ((volatile uint16_t *)(CHIP_REG + 0x1b0))
#define COLOR25 ((volatile uint16_t *)(CHIP_REG + 0x1b2))
#define COLOR26 ((volatile uint16_t *)(CHIP_REG + 0x1b4))
#define COLOR27 ((volatile uint16_t *)(CHIP_REG + 0x1b6))
#define COLOR28 ((volatile uint16_t *)(CHIP_REG + 0x1b8))
#define COLOR29 ((volatile uint16_t *)(CHIP_REG + 0x1ba))
#define COLOR30 ((volatile uint16_t *)(CHIP_REG + 0x1bc))
#define COLOR31 ((volatile uint16_t *)(CHIP_REG + 0x1be))

#define COP1LCH ((volatile uint16_t *)(CHIP_REG + 0x080))
#define COP1LCL ((volatile uint16_t *)(CHIP_REG + 0x082))
#define COP2LCH ((volatile uint16_t *)(CHIP_REG + 0x084))
#define COP2LCL ((volatile uint16_t *)(CHIP_REG + 0x086))
#define COPJMP1 ((volatile uint16_t *)(CHIP_REG + 0x088))
#define COPJMP2 ((volatile uint16_t *)(CHIP_REG + 0x08A))

#define SPR0POS ((volatile uint16_t *)(CHIP_REG + 0x140))
#define SPR0CTL ((volatile uint16_t *)(CHIP_REG + 0x142))
#define SPR0DATA ((volatile uint16_t *)(CHIP_REG + 0x144))
#define SPR0DATB ((volatile uint16_t *)(CHIP_REG + 0x146))

#define SPR1POS ((volatile uint16_t *)(CHIP_REG + 0x148))
#define SPR1CTL ((volatile uint16_t *)(CHIP_REG + 0x14a))
#define SPR1DATA ((volatile uint16_t *)(CHIP_REG + 0x14c))
#define SPR1DATB ((volatile uint16_t *)(CHIP_REG + 0x14e))

#define SPR2POS ((volatile uint16_t *)(CHIP_REG + 0x150))
#define SPR2CTL ((volatile uint16_t *)(CHIP_REG + 0x152))
#define SPR2DATA ((volatile uint16_t *)(CHIP_REG + 0x154))
#define SPR2DATB ((volatile uint16_t *)(CHIP_REG + 0x156))

#define SPR3POS ((volatile uint16_t *)(CHIP_REG + 0x158))
#define SPR3CTL ((volatile uint16_t *)(CHIP_REG + 0x15a))
#define SPR3DATA ((volatile uint16_t *)(CHIP_REG + 0x15c))
#define SPR3DATB ((volatile uint16_t *)(CHIP_REG + 0x15e))

#define SPR4POS ((volatile uint16_t *)(CHIP_REG + 0x160))
#define SPR4CTL ((volatile uint16_t *)(CHIP_REG + 0x162))
#define SPR4DATA ((volatile uint16_t *)(CHIP_REG + 0x164))
#define SPR4DATB ((volatile uint16_t *)(CHIP_REG + 0x166))

#define SPR5POS ((volatile uint16_t *)(CHIP_REG + 0x168))
#define SPR5CTL ((volatile uint16_t *)(CHIP_REG + 0x16a))
#define SPR5DATA ((volatile uint16_t *)(CHIP_REG + 0x16c))
#define SPR5DATB ((volatile uint16_t *)(CHIP_REG + 0x16e))

#define SPR6POS ((volatile uint16_t *)(CHIP_REG + 0x170))
#define SPR6CTL ((volatile uint16_t *)(CHIP_REG + 0x172))
#define SPR6DATA ((volatile uint16_t *)(CHIP_REG + 0x174))
#define SPR6DATB ((volatile uint16_t *)(CHIP_REG + 0x176))

#define SPR7POS ((volatile uint16_t *)(CHIP_REG + 0x178))
#define SPR7CTL ((volatile uint16_t *)(CHIP_REG + 0x17a))
#define SPR7DATA ((volatile uint16_t *)(CHIP_REG + 0x17c))
#define SPR7DATB ((volatile uint16_t *)(CHIP_REG + 0x17e))

struct SPR {
  uint16_t start_h;
  uint16_t start_v;
  uint16_t stop_v;
  uint8_t attach;
};

static inline uint16_t sprpos(const struct SPR *spr) {
  return ((spr->start_v & 0xff) << 8) | ((spr->start_h >> 1) & 0xff);
}

static inline uint16_t sprctl(const struct SPR *spr) {
  return ((spr->stop_v & 0xff) << 8) | ((spr->attach & 0x1) << 7) |
         (((spr->start_v >> 8) & 0x1) << 2) |
         (((spr->stop_v >> 8) & 0x1) << 1) | (spr->start_h & 0x1);
}

extern volatile uint32_t timer_ticks;

void irq_mask(uint32_t mask);
void timer_start(uint32_t timeout);

static inline uint32_t bits_get(uint32_t in, uint32_t pos, uint32_t width) {
  uint32_t mask = (1 << width) - 1;
  return (in >> pos) & mask;
}

static inline uint32_t bits_set(uint32_t in, uint32_t pos, uint32_t width,
                                uint32_t val) {
  uint32_t mask = (1 << width) - 1;
  val &= mask;
  in &= ~(mask << pos);
  return in | (val << pos);
}

static inline uint16_t swap16(uint16_t x) { return (x << 8) | (x >> 8); }

static inline uint32_t swap32(uint32_t x) {
  return ((uint32_t)swap16(x) << 16) | ((uint32_t)swap16(x >> 16));
}

static inline uint16_t *copper_move(uint16_t *instr_addr, uint16_t reg,
                                    uint16_t data) {
  instr_addr[0] = ((reg & 0x1ff) >> 1) << 1;
  instr_addr[1] = data;
  return &instr_addr[2];
}

static inline uint16_t *copper_wait(uint16_t *instr_addr, uint8_t ve,
                                    uint8_t vp, uint8_t he, uint8_t hp) {
  instr_addr[0] = ((uint16_t)vp << 8) | ((uint16_t)(hp & 0x7f) << 1) | 1;
  instr_addr[1] =
      ((uint16_t)(ve & 0x7f) << 8) | ((uint16_t)(he & 0x7f) << 1) | 0;
  return &instr_addr[2];
}
