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
#include "chipregs.h"

#define NULL ((void *)0)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define IRQ_ENABLE() irq_mask(0)
#define IRQ_DISABLE() irq_mask(-1)

#define CHIP_RAM (volatile uint16_t *)0xff000000
#define CHIP_REG(reg) ((volatile uint16_t *)(0xffdff000 + reg))

#define COP_MOVE(ptr, ...)                                                     \
  do {                                                                         \
    struct CopperMove move = (struct CopperMove){__VA_ARGS__};                 \
    *(ptr)++ = copper_move_0(&move);                                           \
    *(ptr)++ = copper_move_1(&move);                                           \
  } while (0)

#define COP_WAIT(ptr, ...)                                                     \
  do {                                                                         \
    struct CopperWait wait = (struct CopperWait){__VA_ARGS__};                 \
    *(ptr)++ = copper_wait_0(&wait);                                           \
    *(ptr)++ = copper_wait_1(&wait);                                           \
  } while (0)


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


struct CopperMove {
  uint16_t reg;
  uint16_t data;
};

static inline uint16_t copper_move_0(const struct CopperMove *cprmov) {
  return ((cprmov->reg & 0x1ff) >> 1) << 1;
}

static inline uint16_t copper_move_1(const struct CopperMove *cprmov) {
  return cprmov->data;
}

struct CopperWait {
  uint8_t ve;
  uint8_t vp;
  uint8_t he;
  uint8_t hp;
};

static inline uint16_t copper_wait_0(const struct CopperWait *wait) {
  return ((uint16_t)wait->vp << 8) | ((uint16_t)(wait->hp & 0x7f) << 1) | 1;
}

static inline uint16_t copper_wait_1(const struct CopperWait *wait) {
  return ((uint16_t)(wait->ve & 0x7f) << 8) | ((uint16_t)(wait->he & 0x7f) << 1) | 0;
}
