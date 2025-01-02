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

`default_nettype none

// $ stty -F /dev/ttyUSB0 115200 raw
// $ cat /dev/ttyUSB0

module ila_simple(clk, rst, i_trig, i_sample, o_uart_tx);

  parameter WIDTH; //number of bits per sample
  parameter DEPTH; //number of samples (log2)
  parameter CLK_FREQ; // Freq of clk (for UART pre-scaler)

  input clk;
  input rst;
  input i_trig;
  input [WIDTH-1:0] i_sample;
  output o_uart_tx;

  wire [7:0] hexdigits[0:15];
  assign hexdigits[0] = "0";
  assign hexdigits[1] = "1";
  assign hexdigits[2] = "2";
  assign hexdigits[3] = "3";
  assign hexdigits[4] = "4";
  assign hexdigits[5] = "5";
  assign hexdigits[6] = "6";
  assign hexdigits[7] = "7";
  assign hexdigits[8] = "8";
  assign hexdigits[9] = "9";
  assign hexdigits[10] = "a";
  assign hexdigits[11] = "b";
  assign hexdigits[12] = "c";
  assign hexdigits[13] = "d";
  assign hexdigits[14] = "e";
  assign hexdigits[15] = "f";

  reg [DEPTH - 1:0] ram_addr;
  wire [WIDTH - 1:0] ram_rdata;
  wire ram_we;

  spram #(
      .aw(DEPTH),
      .dw(WIDTH)
  ) u_ram (
      .clk (clk),
      .rst (rst),
      .ce  (1'b1),
      .oe  (1'b1),
      .addr(ram_addr),
      .do  (ram_rdata),
      .di  (i_sample),
      .we  (ram_we)
  );

  wire uart_busy;

  ila_simple_uart #(
    .CLK_FREQ(CLK_FREQ)
  ) u_uart (
    .clk(clk),
    .rst(rst),
    .i_data(tx_byte),
    .i_data_strb(tx_byte_strb),
    .o_busy(uart_busy),
    .o_txd(o_uart_tx)
  );

  localparam WIDTH4 = (WIDTH + 3) & ~3; // Round up to multiple of 4
  localparam DEPTH4 = (DEPTH + 3) & ~3; // Round up to multiple of 4

  localparam [3:0] S_WAIT = 4'd0;
  localparam [3:0] S_SAMPLE = 4'd1;
  localparam [3:0] S_PRINT = 4'd2;
  localparam [3:0] S_PRINT_ADDR_0 = 4'd3;
  localparam [3:0] S_PRINT_ADDR_1 = 4'd4;
  localparam [3:0] S_PRINT_SPACE_0 = 4'd5;
  localparam [3:0] S_PRINT_SPACE_1 = 4'd6;
  localparam [3:0] S_PRINT_SAMPLE_0 = 4'd7;
  localparam [3:0] S_PRINT_SAMPLE_1 = 4'd8;
  localparam [3:0] S_PRINT_NEWLINE_0 = 4'd9;
  localparam [3:0] S_PRINT_NEWLINE_1 = 4'd10;
  localparam [3:0] S_PRINT_NEWLINE_2 = 4'd11;
  localparam [3:0] S_PRINT_NEWLINE_3 = 4'd12;

  reg [3:0] state;
  reg [4:0] cntr;

  reg [DEPTH4-1:0] addr_shift;
  reg [WIDTH4-1:0] sample_shift;
  reg [7:0] tx_byte;
  reg tx_byte_strb;

  assign ram_we = (state == S_WAIT && i_trig) || (state == S_SAMPLE);

  always @(posedge clk) begin
    tx_byte_strb <= 0;
    if (rst) begin
      ram_addr <= 0;
      state <= S_WAIT;
    end
    else begin
      case (state)
        S_WAIT: begin
          if (i_trig) begin
            ram_addr <= ram_addr + 1;
            state <= S_SAMPLE;
          end
        end
        S_SAMPLE: begin
          ram_addr <= ram_addr + 1;
          if (ram_addr == {DEPTH{1'b1}}) begin
            state <= S_PRINT;
          end
        end
        S_PRINT: begin
          addr_shift <= ram_addr;
          cntr <= 0;
          state <= S_PRINT_ADDR_0;
        end
        S_PRINT_ADDR_0: begin
          tx_byte <= hexdigits[addr_shift[DEPTH4-1-:4]];
          tx_byte_strb <= 1;
          addr_shift <= addr_shift << 4;
          cntr <= cntr + 1;
          state <= S_PRINT_ADDR_1;
        end
        S_PRINT_ADDR_1: begin
          if (!uart_busy) begin
            if (cntr == DEPTH4 / 4) begin
              state <= S_PRINT_SPACE_0;
            end
            else begin
              state <= S_PRINT_ADDR_0;
            end
          end
        end
        S_PRINT_SPACE_0: begin
          tx_byte <= " ";
          tx_byte_strb <= 1;
          sample_shift <= ram_rdata;
          cntr <= 0;
          state <= S_PRINT_SPACE_1;
        end
        S_PRINT_SPACE_1: begin
          if (!uart_busy) begin
            state <= S_PRINT_SAMPLE_0;
          end
        end
        S_PRINT_SAMPLE_0: begin
          tx_byte <= hexdigits[sample_shift[WIDTH4-1-:4]];
          tx_byte_strb <= 1;
          sample_shift <= sample_shift << 4;
          cntr <= cntr + 1;
          state <= S_PRINT_SAMPLE_1;
        end
        S_PRINT_SAMPLE_1: begin
          if (!uart_busy) begin
            if (cntr == WIDTH4 / 4) begin
              state <= S_PRINT_NEWLINE_0;
            end
            else begin
              state <= S_PRINT_SAMPLE_0;
            end
          end
        end
        S_PRINT_NEWLINE_0: begin
          tx_byte <= "\r";
          tx_byte_strb <= 1;
          state <= S_PRINT_NEWLINE_1;
        end
        S_PRINT_NEWLINE_1: begin
          if (!uart_busy) begin
            state <= S_PRINT_NEWLINE_2;
          end
        end
        S_PRINT_NEWLINE_2: begin
          tx_byte <= "\n";
          tx_byte_strb <= 1;
          state <= S_PRINT_NEWLINE_3;
        end
        S_PRINT_NEWLINE_3: begin
          if (!uart_busy) begin
            ram_addr <= ram_addr + 1;
            state <= S_PRINT;
          end
        end
      endcase
    end
  end

endmodule

module ila_simple_uart(clk, rst, i_data, i_data_strb, o_busy, o_txd);
  parameter CLK_FREQ;

  input clk;
  input rst;
  input [7:0] i_data;
  input i_data_strb;
  output o_busy;
  output reg o_txd;

  reg [7:0] cntr;
  reg [2:0] bit_cntr;
  reg [7:0] shift;
  reg [1:0] state;

  localparam integer CLK_DIV = CLK_FREQ / 115200;

  localparam [1:0] S_IDLE  = 2'd0;
  localparam [1:0] S_START = 2'd1;
  localparam [1:0] S_DATA  = 2'd2;
  localparam [1:0] S_STOP  = 2'd3;

  assign o_busy = !(state == S_IDLE) || i_data_strb;

  always @(posedge clk) begin
    if (rst) begin
      cntr <= 0;
      state <= S_IDLE;
    end
    else begin
      o_txd <= 1'b1;
      cntr <= cntr + 1;
      case (state)
        S_IDLE: begin
          if (i_data_strb) begin
            shift <= i_data;
            cntr <= 0;
            state <= S_START;
          end
        end
        S_START: begin
          o_txd <= 1'b0;
          if (cntr == CLK_DIV) begin
            cntr <= 0;
            bit_cntr <= 0;
            state <= S_DATA;
          end
        end
        S_DATA: begin
          o_txd <= shift[0];
          if (cntr == CLK_DIV) begin
            cntr <= 0;
            shift <= {1'b0, shift[7:1]};
            bit_cntr <= bit_cntr + 1;
            if (bit_cntr == 7) begin
              state <= S_STOP;
            end
          end
        end
        S_STOP: begin
          if (cntr == CLK_DIV) begin
            state <= S_IDLE;
          end
        end
      endcase
    end
  end
endmodule
