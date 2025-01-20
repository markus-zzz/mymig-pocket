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

#define BP1_BASE 0x1000
#define BP2_BASE 0x2000
#define BP3_BASE 0x3000

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

// XXX: Add the other pieces

uint16_t grid[GRID_X][GRID_Y];

const char *example[] = {
    // clang-format off
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000000000",
  "0000550600",
  "0000550600",
  "4400000633",
  "0440002603",
  "1111022203",
    // clang-format on
};

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

void piece2playfield(struct PlayField *pf, struct Coord *piece, int x0,
                     int y0) {
  for (int k = 0; k < 4; k++) {
    int x = x0 + piece[k].x;
    int y = y0 + piece[k].y;
    if (0 <= x && x < GRID_X && 0 <= y && y < GRID_Y) {
      set_square(pf->p1, x, y, TILE_SIZE);
      // Shade west side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p2, TILE_SIZE * x, TILE_SIZE * y + i);
      // Shade north side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p2, TILE_SIZE * x + i, TILE_SIZE * y);
      // Shade east side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p3, TILE_SIZE * x + TILE_SIZE - 1, TILE_SIZE * y + i);
      // Shade south side
      for (int i = 0; i < TILE_SIZE; i++)
        set_pixel(pf->p3, TILE_SIZE * x + i, TILE_SIZE * y + TILE_SIZE - 1);
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

void piece2grid(struct Coord *piece, uint16_t piece_id, int x0, int y0) {
  for (int k = 0; k < 4; k++) {
    int x = x0 + piece[k].x;
    int y = y0 + piece[k].y;
    grid[x][y] = piece_id;
  }
}

void setup_copper_list(uint16_t *q, struct PlayField *pf) {
  COP_MOVE(q, .reg = BPL1PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL1PTL, .data = ((uint16_t)pf->p1) >> 1);
  COP_MOVE(q, .reg = BPL2PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL2PTL, .data = ((uint16_t)pf->p2) >> 1);
  COP_MOVE(q, .reg = BPL3PTH, .data = 0x0);
  COP_MOVE(q, .reg = BPL3PTL, .data = ((uint16_t)pf->p3) >> 1);
  // Generate copper interrupt
  COP_WAIT(q, .ve = 0xff, .vp = 220, .he = 0xff, .hp = 0);
  COP_MOVE(q, .reg = INTREQ, .data = 0x8010);
  COP_WAIT(q, .ve = 0xff, .vp = 0xff, .he = 0xff, .hp = 0xff); // EOL
}


int main(void) {

  struct PlayField pf1, pf2;

  playfield_alloc(&pf1);
  playfield_clear(&pf1);
  playfield_alloc(&pf2);
  playfield_clear(&pf2);

#if 0
  // Copy example to grid
  for (int y = 0; y < GRID_Y; y++) {
    for (int x = 0; x < GRID_X; x++) {
      const char *p = example[y];
      grid[x][y] = p[x] - '0';
    }
  }
#endif

  // grid2playfield(&pf1);
  // piece2playfield(&pf1, piece_I[2], 5, 5);
  // piece2playfield(&pf1, piece_I[1], 7, 7);

  for (int x = 0; x < GRID_X; x++) {
    set_square(pf1.p1, x, 0, TILE_SIZE);
    set_square(pf1.p1, x, GRID_Y - 1, TILE_SIZE);
  }
  for (int y = 0; y < GRID_Y; y++) {
    set_square(pf1.p1, 0, y, TILE_SIZE);
    set_square(pf1.p1, GRID_X - 1, y, TILE_SIZE);
  }

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

  uint16_t *cl1 = chipmem_alloc(0x1000);
  uint16_t *cl2 = chipmem_alloc(0x1000);

  setup_copper_list(cl1, &pf1);
  setup_copper_list(cl2, &pf2);

  *CHIP_REG(COP1LCH) = 0;
  *CHIP_REG(COP1LCL) = ((uint16_t)cl1) >> 1;

  *CHIP_REG(COPJMP1) = 0;

  *CHIP_REG(BPLCON0) = 3 << 12;

  uint8_t diw_x_start = 0xf0 & 208;
  uint8_t diw_x_stop  = diw_x_start + PF_DIM_X;
  uint8_t diw_y_start = 32;
  uint8_t diw_y_stop  = diw_y_start + PF_DIM_Y;

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
  struct PlayField *pf = &pf2;
  uint32_t cont1_key_p = 0;
  uint32_t cont1_key = 0;
  uint8_t piece_rot = 0;

  while (1) {
    cont1_key_p = cont1_key;
    cont1_key = *CONT1_KEY;

    if (KEYB_POSEDGE(face_a) && !pieceblocked(piece_I[(piece_rot + 1) & 3], piece_x, piece_y)) {
      piece_rot++;
    }
    if (KEYB_POSEDGE(face_b) && !pieceblocked(piece_I[(piece_rot - 1) & 3], piece_x, piece_y)) {
      piece_rot--;
    }
    struct Coord *piece = piece_I[piece_rot & 3];
    if (KEYB_DOWN(dpad_left) && !pieceblocked(piece, piece_x - 1, piece_y)) {
      piece_x--;
    }
    if (KEYB_DOWN(dpad_right) && !pieceblocked(piece, piece_x + 1, piece_y)) {
      piece_x++;
    }

    playfield_clear(pf);
    grid2playfield(pf);
    piece2playfield(pf, piece, piece_x, piece_y);
    uint32_t wait_tick = ticks + 1;
    while (ticks < wait_tick);

    if (pf == &pf1) {
      pf = &pf2;
      *CHIP_REG(COP1LCH) = 0;
      *CHIP_REG(COP1LCL) = ((uint16_t)cl1) >> 1;

    } else {
      pf = &pf1;
      *CHIP_REG(COP1LCH) = 0;
      *CHIP_REG(COP1LCL) = ((uint16_t)cl2) >> 1;
    }

    if (pieceblocked(piece, piece_x, piece_y + 1)) {
      piece2grid(piece, piece_id, piece_x, piece_y);
      // Start new piece
      piece_id++;
      piece_y = 0;
    } else {
      piece_y++;
    }
  }

  return 0;
}

uint32_t *irq(uint32_t *regs, uint32_t irqs) {
  *CHIP_REG(INTREQ) = 0x0010; // Ack/clear the copper interrupt
  ticks++;
  return regs;
}
