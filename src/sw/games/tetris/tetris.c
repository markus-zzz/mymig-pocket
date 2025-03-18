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
#include <stddef.h>

#define CONT1_KEY ((volatile uint32_t *)0x20000000)

#define KEYB_BIT_dpad_up 0
#define KEYB_BIT_dpad_down 1
#define KEYB_BIT_dpad_left 2
#define KEYB_BIT_dpad_right 3
#define KEYB_BIT_face_a 4
#define KEYB_BIT_face_b 5
#define KEYB_BIT_face_x 6
#define KEYB_BIT_face_y 7
#define KEYB_BIT_trig_l1 8
#define KEYB_BIT_trig_r1 9
#define KEYB_BIT_trig_l2 10
#define KEYB_BIT_trig_r2 11
#define KEYB_BIT_trig_l3 12
#define KEYB_BIT_trig_r3 13
#define KEYB_BIT_face_select 14
#define KEYB_BIT_face_start 15

#define KEYB_DOWN(bit) (cont1_key & (1 << (KEYB_BIT_##bit)))
#define KEYB_POSEDGE(bit)                                                      \
  ((~cont1_key_p & (1 << (KEYB_BIT_##bit))) &&                                 \
   (cont1_key & (1 << (KEYB_BIT_##bit))))

#define GRID_X 10
#define GRID_Y 20

#define PF_DIM_X 112
#define PF_DIM_Y 200

#define TILE_SIZE 10

void *memset(void *dst, int val, register size_t len) {
  unsigned char *p = (unsigned char *)dst;
  while (len-- > 0)
    *p++ = val;
  return dst;
}

volatile uint32_t ticks = 0;

uint16_t *chipmem_alloc(unsigned words) {
  static uint16_t *next_free_p = (uint16_t *)(CHIP_RAM + 2);
  uint16_t *p = next_free_p;
  next_free_p += words;
  return p;
}

struct PlayField {
  volatile uint16_t *p1;
  volatile uint16_t *p2;
  volatile uint16_t *p3;
};

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

struct Coord piece_J[4][4] = {{{0, 2}, {0, 1}, {1, 1}, {2, 1}},
                              {{2, 2}, {1, 2}, {1, 1}, {1, 0}},
                              {{2, 0}, {2, 1}, {1, 1}, {0, 1}},
                              {{0, 0}, {1, 0}, {1, 1}, {1, 2}}};

struct Coord piece_L[4][4] = {{{2, 2}, {2, 1}, {1, 1}, {0, 1}},
                              {{2, 0}, {1, 0}, {1, 1}, {1, 2}},
                              {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
                              {{0, 2}, {1, 2}, {1, 1}, {1, 0}}};

struct Coord piece_O[4][4] = {{{1, 1}, {1, 2}, {2, 1}, {2, 2}},
                              {{1, 1}, {1, 2}, {2, 1}, {2, 2}},
                              {{1, 1}, {1, 2}, {2, 1}, {2, 2}},
                              {{1, 1}, {1, 2}, {2, 1}, {2, 2}}};

struct Coord piece_S[4][4] = {{{0, 1}, {1, 1}, {1, 2}, {2, 2}},
                              {{2, 0}, {2, 1}, {1, 1}, {1, 2}},
                              {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
                              {{1, 0}, {1, 1}, {0, 1}, {0, 2}}};

struct Coord piece_T[4][4] = {{{0, 1}, {1, 1}, {2, 1}, {1, 2}},
                              {{1, 2}, {1, 1}, {1, 0}, {2, 1}},
                              {{2, 1}, {1, 1}, {0, 1}, {1, 0}},
                              {{1, 0}, {1, 1}, {1, 2}, {0, 1}}};

struct Coord piece_Z[4][4] = {{{0, 2}, {1, 2}, {1, 1}, {2, 1}},
                              {{2, 2}, {2, 1}, {1, 1}, {1, 0}},
                              {{0, 1}, {1, 1}, {1, 0}, {2, 0}},
                              {{1, 2}, {1, 1}, {0, 1}, {0, 0}}};

struct Coord (*pieces[])[4][4] = {&piece_I, &piece_J, &piece_L, &piece_O,
                                  &piece_S, &piece_T, &piece_Z};

// XXX: Add the other pieces

uint16_t grid[GRID_X][GRID_Y];

static uint16_t *sprite_tiles[4] = {0};

static inline void set_pixel(volatile uint16_t *bp, uint16_t x, uint16_t y) {
  bp += y * PF_DIM_X / 16;
  bp += x / 16;
  *bp |= 1 << (15 - x % 16);
}

static inline void set_square(volatile uint16_t *bp, uint16_t x, uint16_t y,
                              uint16_t size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      set_pixel(bp, x * size + i, y * size + j);
    }
  }
}

void playfield_alloc(struct PlayField *pf) {
  pf->p1 = chipmem_alloc(PF_DIM_X * PF_DIM_Y / 16);
  pf->p2 = chipmem_alloc(PF_DIM_X * PF_DIM_Y / 16);
  pf->p3 = chipmem_alloc(PF_DIM_X * PF_DIM_Y / 16);
}

void playfield_clear(struct PlayField *pf) {
  for (int i = 0; i < PF_DIM_X * PF_DIM_Y / 16; i++) {
    pf->p1[i] = 0;
    pf->p2[i] = 0;
    pf->p3[i] = 0;
  }
}

void grid2playfield(struct PlayField *pf) {
  for (int y = 0; y < GRID_Y; y++) {
    for (int x = 0; x < GRID_X; x++) {
      if (grid[x][y]) {
        set_square(pf->p1, x, y, TILE_SIZE);
        if (x == 0 || grid[x][y] != grid[x - 1][y]) { // Shade west side
          for (int i = 0; i < TILE_SIZE; i++)
            set_pixel(pf->p2, TILE_SIZE * x, TILE_SIZE * y + i);
        }
        if (y == 0 || grid[x][y] != grid[x][y - 1]) { // Shade north side
          for (int i = 0; i < TILE_SIZE; i++)
            set_pixel(pf->p2, TILE_SIZE * x + i, TILE_SIZE * y);
        }
        if (x == GRID_X - 1 ||
            grid[x][y] != grid[x + 1][y]) { // Shade east side
          for (int i = 0; i < TILE_SIZE; i++)
            set_pixel(pf->p3, TILE_SIZE * x + TILE_SIZE - 1, TILE_SIZE * y + i);
        }
        if (y == GRID_Y - 1 ||
            grid[x][y] != grid[x][y + 1]) { // Shade south side
          for (int i = 0; i < TILE_SIZE; i++)
            set_pixel(pf->p3, TILE_SIZE * x + i, TILE_SIZE * y + TILE_SIZE - 1);
        }
      }
    }
  }
}

int pieceblocked(struct Coord *piece, int x0, int y0) {
  for (int k = 0; k < 4; k++) {
    int x = x0 + piece[k].x;
    int y = y0 + piece[k].y;

    if (x < 0 || x >= GRID_X || y < 0 || y >= GRID_Y) {
      return 1;
    }

    if (grid[x][y] != 0) {
      return 1;
    }
  }
  return 0;
}

void piece2grid(struct PlayField *pf, struct Coord *piece, uint16_t piece_id,
                int x0, int y0) {
  for (int k = 0; k < 4; k++) {
    int x = x0 + piece[k].x;
    int y = y0 + piece[k].y;
    grid[x][y] = piece_id;
  }
  for (int k = 0; k < 4; k++) {
    int x = x0 + piece[k].x;
    int y = y0 + piece[k].y;
    set_square(pf->p1, x, y, TILE_SIZE);
    if (x == 0 || grid[x][y] != grid[x - 1][y]) { // Shade west side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p2, TILE_SIZE * x, TILE_SIZE * y + i);
    }
    if (y == 0 || grid[x][y] != grid[x][y - 1]) { // Shade north side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p2, TILE_SIZE * x + i, TILE_SIZE * y);
    }
    if (x == GRID_X - 1 || grid[x][y] != grid[x + 1][y]) { // Shade east side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p3, TILE_SIZE * x + TILE_SIZE - 1, TILE_SIZE * y + i);
    }
    if (y == GRID_Y - 1 || grid[x][y] != grid[x][y + 1]) { // Shade south side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p3, TILE_SIZE * x + i, TILE_SIZE * y + TILE_SIZE - 1);
    }
  }
}

void setup_copper_list(uint16_t *q, struct PlayField *pf) {
  COP_MOVE(q, .reg = BPL1PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL1PTL, .data = ((uint16_t)pf->p1) >> 1);
  COP_MOVE(q, .reg = BPL2PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL2PTL, .data = ((uint16_t)pf->p2) >> 1);
  COP_MOVE(q, .reg = BPL3PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL3PTL, .data = ((uint16_t)pf->p3) >> 1);

  COP_MOVE(q, .reg = SPR0PTH, .data = 0x0);
  COP_MOVE(q, .reg = SPR0PTL, .data = ((uint16_t)sprite_tiles[0]) >> 1);
  COP_MOVE(q, .reg = SPR1PTH, .data = 0x0);
  COP_MOVE(q, .reg = SPR1PTL, .data = ((uint16_t)sprite_tiles[1]) >> 1);
  COP_MOVE(q, .reg = SPR2PTH, .data = 0x0);
  COP_MOVE(q, .reg = SPR2PTL, .data = ((uint16_t)sprite_tiles[2]) >> 1);
  COP_MOVE(q, .reg = SPR3PTH, .data = 0x0);
  COP_MOVE(q, .reg = SPR3PTL, .data = ((uint16_t)sprite_tiles[3]) >> 1);

  // Generate copper interrupt
  COP_WAIT(q, .ve = 0xff, .vp = 220, .he = 0xff, .hp = 0);
  COP_MOVE(q, .reg = INTREQ, .data = 0x8010);
  COP_WAIT(q, .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff); // EOL
}

void sprite_tile_init(uint16_t *p) {
  p += 2; // Skip pos and ctl
  *p++ = 0xffc0;
  *p++ = 0xffc0;
  for (int i = 0; i < 8; i++) {
    *p++ = 0x8040;
    *p++ = 0xffc0;
  }
  *p++ = 0xffc0;
  *p++ = 0xffc0;
  // End of DMA list
  *p++ = 0;
  *p++ = 0;
}

void sprite_tile_pos(uint16_t *p, uint16_t x, uint16_t y) {
  struct SPR spr0 = {.start_h = x, .start_v = y, .stop_v = y + 10, .attach = 0};
  *p++ = sprpos(&spr0);
  *p++ = sprctl(&spr0);
}

void piece2sprites(struct PlayField *pf, struct Coord *piece, int x0, int y0) {
  for (int k = 0; k < 4; k++) {
    int x = x0 + piece[k].x;
    int y = y0 + piece[k].y;
    sprite_tile_pos(sprite_tiles[k], 208 + x * TILE_SIZE, 32 + y * TILE_SIZE);
  }
}

int main(void) {

  struct PlayField pf1;

  playfield_alloc(&pf1);
  playfield_clear(&pf1);

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
  *CHIP_REG(COLOR21) = 0xf00;
  *CHIP_REG(COLOR22) = 0x0f0;
  *CHIP_REG(COLOR23) = 0x00f;

  uint16_t *cl1 = chipmem_alloc(0x1000);

  for (int i = 0; i < 4; i++) {
    sprite_tiles[i] = chipmem_alloc(32);
    sprite_tile_init(sprite_tiles[i]);
  }

  setup_copper_list(cl1, &pf1);

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = ((uint16_t)cl1) >> 1;

  *CHIP_REG(COPJMP1) = 0;

  *CHIP_REG(BPLCON0) = 3 << 12;

  uint8_t diw_x_start = 0xf0 & 208;
  uint8_t diw_x_stop = diw_x_start + PF_DIM_X;
  uint8_t diw_y_start = 32;
  uint8_t diw_y_stop = diw_y_start + PF_DIM_Y;

  *CHIP_REG(DIWSTRT) = ((uint16_t)diw_y_start << 8) | diw_x_start;
  *CHIP_REG(DIWSTOP) = ((uint16_t)diw_y_stop << 8) | diw_x_stop;
  *CHIP_REG(DDFSTRT) = (diw_x_start - 16) >> 1;
  *CHIP_REG(DDFSTOP) = (diw_x_start + PF_DIM_X - 16) >> 1;

  irq_mask(0);
  *CHIP_REG(INTENA) = 0xc010; // Enable interrupts (master enable and copper)

  //
  // Game loop
  //
  int piece_x = 2;
  int piece_y = 0;
  uint16_t piece_id = 1;
  struct PlayField *pf = &pf1;
  uint32_t cont1_key_p = 0;
  uint32_t cont1_key = 0;
  uint8_t piece_rot = 0;
  uint32_t wait_tick_drop = ticks;
  struct Coord(*piece2)[4][4] = pieces[0];
  unsigned cntr = 1;

  while (1) {
    cont1_key_p = cont1_key;
    cont1_key = *CONT1_KEY;

    if (KEYB_POSEDGE(face_a) &&
        !pieceblocked((*piece2)[(piece_rot + 1) & 3], piece_x, piece_y)) {
      piece_rot++;
    }
    if (KEYB_POSEDGE(face_b) &&
        !pieceblocked((*piece2)[(piece_rot - 1) & 3], piece_x, piece_y)) {
      piece_rot--;
    }
    struct Coord *piece = (*piece2)[piece_rot & 3];
    if (KEYB_POSEDGE(dpad_left) && !pieceblocked(piece, piece_x - 1, piece_y)) {
      piece_x--;
    }
    if (KEYB_POSEDGE(dpad_right) &&
        !pieceblocked(piece, piece_x + 1, piece_y)) {
      piece_x++;
    }

    piece2sprites(pf, piece, piece_x, piece_y);
    uint32_t wait_tick = ticks + 1;
    while (ticks < wait_tick)
      ;
    if (ticks >= wait_tick_drop) {
      if (pieceblocked(piece, piece_x, piece_y + 1)) {
        if (piece_y == 0) {
          memset(grid, 0, sizeof(grid));
          playfield_clear(pf);
        } else {
          piece2grid(pf, piece, piece_id, piece_x, piece_y);
        }
        // Start new piece
        piece2 = pieces[cntr++ % 7];
        piece_id++;
        piece_x = 2;
        piece_y = 0;
      } else {
        piece_y++;
      }
      wait_tick_drop = ticks + 15;
    }
  }

  return 0;
}

uint32_t *irq(uint32_t *regs, uint32_t irqs) {
  *CHIP_REG(INTREQ) = 0x0010; // Ack/clear the copper interrupt
  ticks++;
  return regs;
}
