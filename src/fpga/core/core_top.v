//
// User core top-level
//
// Instantiated by the real top-level: apf_top
//

`default_nettype none

module core_top (

    //
    // physical connections
    //

    ///////////////////////////////////////////////////
    // clock inputs 74.25mhz. not phase aligned, so treat these domains as asynchronous

    input wire clk_74a,  // mainclk1
    input wire clk_74b,  // mainclk1
`ifdef __VERILATOR__
    input wire clk_32mhz,
`endif

    ///////////////////////////////////////////////////
    // cartridge interface
    // switches between 3.3v and 5v mechanically
    // output enable for multibit translators controlled by pic32

    // GBA AD[15:8]
    inout  wire [7:0] cart_tran_bank2,
    output wire       cart_tran_bank2_dir,

    // GBA AD[7:0]
    inout  wire [7:0] cart_tran_bank3,
    output wire       cart_tran_bank3_dir,

    // GBA A[23:16]
    inout  wire [7:0] cart_tran_bank1,
    output wire       cart_tran_bank1_dir,

    // GBA [7] PHI#
    // GBA [6] WR#
    // GBA [5] RD#
    // GBA [4] CS1#/CS#
    //     [3:0] unwired
    inout  wire [7:4] cart_tran_bank0,
    output wire       cart_tran_bank0_dir,

    // GBA CS2#/RES#
    inout  wire cart_tran_pin30,
    output wire cart_tran_pin30_dir,
    // when GBC cart is inserted, this signal when low or weak will pull GBC /RES low with a special circuit
    // the goal is that when unconfigured, the FPGA weak pullups won't interfere.
    // thus, if GBC cart is inserted, FPGA must drive this high in order to let the level translators
    // and general IO drive this pin.
    output wire cart_pin30_pwroff_reset,

    // GBA IRQ/DRQ
    inout  wire cart_tran_pin31,
    output wire cart_tran_pin31_dir,

    // infrared
    input  wire port_ir_rx,
    output wire port_ir_tx,
    output wire port_ir_rx_disable,

    // GBA link port
    inout  wire port_tran_si,
    output wire port_tran_si_dir,
    inout  wire port_tran_so,
    output wire port_tran_so_dir,
    inout  wire port_tran_sck,
    output wire port_tran_sck_dir,
    inout  wire port_tran_sd,
    output wire port_tran_sd_dir,

    ///////////////////////////////////////////////////
    // cellular psram 0 and 1, two chips (64mbit x2 dual die per chip)

    output wire [21:16] cram0_a,
    inout  wire [ 15:0] cram0_dq,
    input  wire         cram0_wait,
    output wire         cram0_clk,
    output wire         cram0_adv_n,
    output wire         cram0_cre,
    output wire         cram0_ce0_n,
    output wire         cram0_ce1_n,
    output wire         cram0_oe_n,
    output wire         cram0_we_n,
    output wire         cram0_ub_n,
    output wire         cram0_lb_n,

    output wire [21:16] cram1_a,
    inout  wire [ 15:0] cram1_dq,
    input  wire         cram1_wait,
    output wire         cram1_clk,
    output wire         cram1_adv_n,
    output wire         cram1_cre,
    output wire         cram1_ce0_n,
    output wire         cram1_ce1_n,
    output wire         cram1_oe_n,
    output wire         cram1_we_n,
    output wire         cram1_ub_n,
    output wire         cram1_lb_n,

    ///////////////////////////////////////////////////
    // sdram, 512mbit 16bit

    output wire [12:0] dram_a,
    output wire [ 1:0] dram_ba,
    inout  wire [15:0] dram_dq,
    output wire [ 1:0] dram_dqm,
    output wire        dram_clk,
    output wire        dram_cke,
    output wire        dram_ras_n,
    output wire        dram_cas_n,
    output wire        dram_we_n,

    ///////////////////////////////////////////////////
    // sram, 1mbit 16bit

    output wire [16:0] sram_a,
    inout  wire [15:0] sram_dq,
    output wire        sram_oe_n,
    output wire        sram_we_n,
    output wire        sram_ub_n,
    output wire        sram_lb_n,

    ///////////////////////////////////////////////////
    // vblank driven by dock for sync in a certain mode

    input wire vblank,

    ///////////////////////////////////////////////////
    // i/o to 6515D breakout usb uart

    output wire dbg_tx,
    input  wire dbg_rx,

    ///////////////////////////////////////////////////
    // i/o pads near jtag connector user can solder to

    output wire user1,
    input  wire user2,

    ///////////////////////////////////////////////////
    // RFU internal i2c bus

    inout  wire aux_sda,
    output wire aux_scl,

    ///////////////////////////////////////////////////
    // RFU, do not use
    output wire vpll_feed,


    //
    // logical connections
    //

    ///////////////////////////////////////////////////
    // video, audio output to scaler
    output wire [23:0] video_rgb,
    output wire        video_rgb_clock,
    output wire        video_rgb_clock_90,
    output wire        video_de,
    output wire        video_skip,
    output wire        video_vs,
    output wire        video_hs,

    output wire audio_mclk,
    input  wire audio_adc,
    output wire audio_dac,
    output wire audio_lrck,

    ///////////////////////////////////////////////////
    // bridge bus connection
    // synchronous to clk_74a
    output wire        bridge_endian_little,
    input  wire [31:0] bridge_addr,
    input  wire        bridge_rd,
    output reg  [31:0] bridge_rd_data,
    input  wire        bridge_wr,
    input  wire [31:0] bridge_wr_data,

    ///////////////////////////////////////////////////
    // controller data
    //
    // key bitmap:
    //   [0]    dpad_up
    //   [1]    dpad_down
    //   [2]    dpad_left
    //   [3]    dpad_right
    //   [4]    face_a
    //   [5]    face_b
    //   [6]    face_x
    //   [7]    face_y
    //   [8]    trig_l1
    //   [9]    trig_r1
    //   [10]   trig_l2
    //   [11]   trig_r2
    //   [12]   trig_l3
    //   [13]   trig_r3
    //   [14]   face_select
    //   [15]   face_start
    // joy values - unsigned
    //   [ 7: 0] lstick_x
    //   [15: 8] lstick_y
    //   [23:16] rstick_x
    //   [31:24] rstick_y
    // trigger values - unsigned
    //   [ 7: 0] ltrig
    //   [15: 8] rtrig
    //
    input wire [15:0] cont1_key,
    input wire [15:0] cont2_key,
    input wire [15:0] cont3_key,
    input wire [15:0] cont4_key,
    input wire [31:0] cont1_joy,
    input wire [31:0] cont2_joy,
    input wire [31:0] cont3_joy,
    input wire [31:0] cont4_joy,
    input wire [15:0] cont1_trig,
    input wire [15:0] cont2_trig,
    input wire [15:0] cont3_trig,
    input wire [15:0] cont4_trig

);

  // not using the IR port, so turn off both the LED, and
  // disable the receive circuit to save power
  assign port_ir_tx              = 0;
  assign port_ir_rx_disable      = 1;

  // bridge endianness
  assign bridge_endian_little    = 0;

  // cart is unused, so set all level translators accordingly
  // directions are 0:IN, 1:OUT
  assign cart_tran_bank3         = 8'hzz;
  assign cart_tran_bank3_dir     = 1'b0;
  assign cart_tran_bank2         = 8'hzz;
  assign cart_tran_bank2_dir     = 1'b0;
  assign cart_tran_bank1         = 8'hzz;
  assign cart_tran_bank1_dir     = 1'b0;
  // assign cart_tran_bank0         = 4'hf;
  assign cart_tran_bank0_dir     = 1'b1;
  assign cart_tran_pin30         = 1'b0;  // reset or cs2, we let the hw control it by itself
  assign cart_tran_pin30_dir     = 1'bz;
  assign cart_pin30_pwroff_reset = 1'b0;  // hardware can control this
  assign cart_tran_pin31         = 1'bz;  // input
  assign cart_tran_pin31_dir     = 1'b0;  // input

  // link port is input only
  assign port_tran_so            = 1'bz;
  assign port_tran_so_dir        = 1'b0;  // SO is output only
  assign port_tran_si            = 1'bz;
  assign port_tran_si_dir        = 1'b0;  // SI is input only
  assign port_tran_sck           = 1'bz;
  assign port_tran_sck_dir       = 1'b0;  // clock direction can change
  assign port_tran_sd            = 1'bz;
  assign port_tran_sd_dir        = 1'b0;  // SD is input and not used

  // tie off the rest of the pins we are not using
  assign cram1_a                 = 'h0;
  assign cram1_dq                = {16{1'bZ}};
  assign cram1_clk               = 0;
  assign cram1_adv_n             = 1;
  assign cram1_cre               = 0;
  assign cram1_ce0_n             = 1;
  assign cram1_ce1_n             = 1;
  assign cram1_oe_n              = 1;
  assign cram1_we_n              = 1;
  assign cram1_ub_n              = 1;
  assign cram1_lb_n              = 1;

  assign dram_a                  = 'h0;
  assign dram_ba                 = 'h0;
  assign dram_dq                 = {16{1'bZ}};
  assign dram_dqm                = 'h0;
  assign dram_clk                = 'h0;
  assign dram_cke                = 'h0;
  assign dram_ras_n              = 'h1;
  assign dram_cas_n              = 'h1;
  assign dram_we_n               = 'h1;

  assign sram_a                  = 'h0;
  assign sram_dq                 = {16{1'bZ}};
  assign sram_oe_n               = 1;
  assign sram_we_n               = 1;
  assign sram_ub_n               = 1;
  assign sram_lb_n               = 1;

  assign dbg_tx                  = 1'bZ;
  assign user1                   = 1'bZ;
  assign aux_scl                 = 1'bZ;
  assign vpll_feed               = 1'bZ;

  always @(*) begin
    casex (bridge_addr)
      32'hF80020xx: begin
        bridge_rd_data <= dataslot_table_rd_data;
      end
      default: begin
        bridge_rd_data <= cmd_bridge_rd_data;
      end
    endcase
  end

  //
  // host/target command handler
  //
  wire reset_n;  // driven by host commands, can be used as core-wide reset
  wire [31:0] cmd_bridge_rd_data;

  // bridge host commands
  // synchronous to clk_74a
  wire        status_boot_done = pll_core_locked;
  wire        status_setup_done = pll_core_locked;  // rising edge triggers a target command
  wire        status_running = reset_n;  // we are running as soon as reset_n goes high

  wire        osnotify_inmenu;

  // bridge target commands
  // synchronous to clk_74a
  core_bridge_cmd icb (
      .clk                 (clk_74a),
      .reset_n             (reset_n),
      .bridge_endian_little(bridge_endian_little),
      .bridge_addr         (bridge_addr),
      .bridge_rd           (bridge_rd),
      .bridge_rd_data      (cmd_bridge_rd_data),
      .bridge_wr           (bridge_wr),
      .bridge_wr_data      (bridge_wr_data),

      .status_boot_done (status_boot_done),
      .status_setup_done(status_setup_done),
      .status_running   (status_running),

      .osnotify_inmenu(osnotify_inmenu),

      .i_cpu_clk(clk_8mhz),
      .i_cpu_req(cpu_mem_valid && cpu_mem_addr[31:28] == 4'h4),
      .o_cpu_ack_pulse(bridge_ack_pulse),

      .i_cpu_addr (cpu_mem_addr),
      .i_cpu_wdata(cpu_mem_wdata),
      .i_cpu_wstrb(cpu_mem_wstrb),
      .o_cpu_rdata(bridge_rdata)
  );

  //
  // audio i2s silence generator
  // see other examples for actual audio generation
  //

  assign audio_mclk = audgen_mclk;
  assign audio_dac  = audgen_dac;
  assign audio_lrck = audgen_lrck;

  // generate MCLK = 12.288mhz with fractional accumulator
  reg [21:0] audgen_accum = 0;
  reg        audgen_mclk;
  parameter [20:0] CYCLE_48KHZ = 21'd122880 * 2;
  always @(posedge clk_74a) begin
    audgen_accum <= audgen_accum + CYCLE_48KHZ;
    if (audgen_accum >= 21'd742500) begin
      audgen_mclk  <= ~audgen_mclk;
      audgen_accum <= audgen_accum - 21'd742500 + CYCLE_48KHZ;
    end
  end

  // generate SCLK = 3.072mhz by dividing MCLK by 4
  reg  [1:0] aud_mclk_divider;
  wire       audgen_sclk = aud_mclk_divider[1]  /* synthesis keep*/;
  reg        audgen_lrck_1;
  always @(posedge audgen_mclk) begin
    aud_mclk_divider <= aud_mclk_divider + 1'b1;
  end

  // shift out audio data as I2S
  // 32 total bits per channel, but only 16 active bits at the start and then 16 dummy bits
  //
  reg [ 4:0] audgen_lrck_cnt;
  reg        audgen_lrck;
  reg        audgen_dac;
  reg [31:0] audgen_shift;
  always @(negedge audgen_sclk) begin
    audgen_dac <= audgen_shift[31];
    audgen_shift <= {audgen_shift[30:0], 1'b0};
    // 48khz * 64
    audgen_lrck_cnt <= audgen_lrck_cnt + 1'b1;
    if (audgen_lrck_cnt == 31) begin
      // switch channels
      audgen_lrck  <= ~audgen_lrck;
      // XXX: audgen_shift <= {sid_wave, 16'h0};  // XXX: sid_wave coming from different clock domain!
    end
  end


  ///////////////////////////////////////////////


  wire clk_8mhz;
  wire clk_8mhz_90deg;
  wire pll_core_locked;

`ifndef __VERILATOR__
  wire clk_32mhz;
  mf_pllbase mp1 (
      .refclk(clk_74a),
      .rst   (0),

      .outclk_0(clk_8mhz),
      .outclk_1(clk_8mhz_90deg),
      .outclk_2(clk_32mhz),

      .locked(pll_core_locked)
  );
`else
  assign clk_8mhz = clk_74a;
  assign clk_8mhz_90deg = clk_74a;
  assign pll_core_locked = 1;
`endif

  // XXX: This is not the right way to do clock domain crossing (don't put
  // syncs on the bus signals)!
  wire [15:0] cont1_key_s;
  wire [15:0] cont2_key_s;
  wire [15:0] cont3_key_s;
  wire [15:0] cont4_key_s;
  wire [31:0] cont1_joy_s;
  wire [31:0] cont2_joy_s;
  wire [31:0] cont3_joy_s;
  wire [31:0] cont4_joy_s;
  wire [15:0] cont1_trig_s;
  wire [15:0] cont2_trig_s;
  wire [15:0] cont3_trig_s;
  wire [15:0] cont4_trig_s;
  synch_3 #(.WIDTH(32)) s_cont1_key (cont1_key, cont1_key_s, clk_8mhz);
  synch_3 #(.WIDTH(32)) s_cont2_key (cont2_key, cont2_key_s, clk_8mhz);
  synch_3 #(.WIDTH(32)) s_cont3_key (cont3_key, cont3_key_s, clk_8mhz);
  synch_3 #(.WIDTH(32)) s_cont4_key (cont4_key, cont4_key_s, clk_8mhz);
  synch_3 #(.WIDTH(32)) s_cont1_joy (cont1_joy, cont1_joy_s, clk_8mhz);
  synch_3 #(.WIDTH(32)) s_cont2_joy (cont2_joy, cont2_joy_s, clk_8mhz);
  synch_3 #(.WIDTH(32)) s_cont3_joy (cont3_joy, cont3_joy_s, clk_8mhz);
  synch_3 #(.WIDTH(32)) s_cont4_joy (cont4_joy, cont4_joy_s, clk_8mhz);
  synch_3 #(.WIDTH(16)) s_cont1_trig (cont1_trig, cont1_trig_s, clk_8mhz);
  synch_3 #(.WIDTH(16)) s_cont2_trig (cont2_trig, cont2_trig_s, clk_8mhz);
  synch_3 #(.WIDTH(16)) s_cont3_trig (cont3_trig, cont3_trig_s, clk_8mhz);
  synch_3 #(.WIDTH(16)) s_cont4_trig (cont4_trig, cont4_trig_s, clk_8mhz);

  wire cpu_mem_valid;
  wire cpu_mem_instr;
  reg cpu_mem_ready;
  wire [31:0] cpu_mem_addr;
  wire [31:0] cpu_mem_wdata;
  wire [3:0] cpu_mem_wstrb;
  reg [31:0] cpu_mem_rdata;
  wire [31:0] ram_rdata;

  wire [31:0] ram_addr;
  wire [31:0] ram_wdata;
  wire [3:0] ram_wstrb;

  wire [31:0] bridge_rdata;
  wire [31:0] bridge_dpram_rdata;

  assign ram_addr  = cpu_mem_addr;
  assign ram_wdata = cpu_mem_wdata;
  assign ram_wstrb = cpu_mem_wstrb;

  always @* begin
    casex (cpu_mem_addr)
      32'h0xxx_xxxx: cpu_mem_rdata = ram_rdata;
      32'h2000_0000: cpu_mem_rdata = cont1_key_s;
      32'h2000_0004: cpu_mem_rdata = cont2_key_s;
      32'h2000_0008: cpu_mem_rdata = cont3_key_s;
      32'h2000_000c: cpu_mem_rdata = cont4_key_s;
      32'h2000_0010: cpu_mem_rdata = cont1_joy_s;
      32'h2000_0014: cpu_mem_rdata = cont2_joy_s;
      32'h2000_0018: cpu_mem_rdata = cont3_joy_s;
      32'h2000_001c: cpu_mem_rdata = cont4_joy_s;
      32'h2000_0020: cpu_mem_rdata = cont1_trig_s;
      32'h2000_0024: cpu_mem_rdata = cont2_trig_s;
      32'h2000_0028: cpu_mem_rdata = cont3_trig_s;
      32'h2000_002c: cpu_mem_rdata = cont4_trig_s;
      32'h4xxx_xxxx: cpu_mem_rdata = bridge_rdata;
      32'h7xxx_xxxx: cpu_mem_rdata = bridge_dpram_rdata;
      32'h9xxx_xxxx: cpu_mem_rdata = dataslot_table_rd_data_cpu;
      32'hffxx_xxxx: cpu_mem_rdata = {2{mymig_cpu_mem_rdata}};
      default: cpu_mem_rdata = 0;
    endcase
  end

  wire bridge_ack_pulse;

  // Synchronize the 'reset_n' signal for safe use in the clk_8mhz domain (as
  // well as the related clk_32mhz domain).
  wire rst;
  synch_3 s_reset_n (~reset_n, rst, clk_8mhz);

  always @(posedge clk_8mhz) begin
    if (rst) cpu_mem_ready <= 0;
    else begin
      casex (cpu_mem_addr)
        32'h0xxx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'h2xxx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'h3xxx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'h4xxx_xxxx: cpu_mem_ready <= bridge_ack_pulse;
        32'h5000_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'h51xx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'h5xxx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'h7xxx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'h9xxx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid;
        32'hffxx_xxxx: cpu_mem_ready <= ~cpu_mem_ready & cpu_mem_valid & mymig_cpu_ack;
        default: cpu_mem_ready <= 0;
      endcase
    end
  end

  // The following signals are in clk_74a domain
  reg bridge_wr_req;
  wire bridge_wr_ack;
  reg [31:0] bridge_addr_r;
  reg [31:0] bridge_wr_data_r;

  // Write pulse synchronized to clk_8mhz domain
  wire bridge_wr_pulse;

  synch_3 s_bridge_wr (
      .i(bridge_wr_req),
      .rise(bridge_wr_pulse),
      .clk(clk_8mhz)
  );

  synch_3 s_bridge_wr2 (
      .i(bridge_wr_pulse),
      .rise(bridge_wr_ack),
      .clk(clk_74a)
  );

  always @(posedge clk_74a) begin
    if (bridge_wr) begin
      bridge_wr_req <= 1;
      bridge_addr_r <= bridge_addr;
      bridge_wr_data_r <= {bridge_wr_data[7:0], bridge_wr_data[15:8], bridge_wr_data[23:16], bridge_wr_data[31:24]};
    end
    else if (bridge_wr_ack) begin
      bridge_wr_req <= 0;
    end
  end

  wire bridge_wr_access;
  assign bridge_wr_access = bridge_wr_pulse && bridge_addr_r[31:28] == 4'h5;

  // 128KB CPU RAM
  genvar gi;
  generate
    for (gi = 0; gi < 4; gi = gi + 1) begin : ram
      spram #(
          .aw(15),
          .dw(8)
      ) u_ram (
          .clk (clk_8mhz),
          .rst (1'b0),
          .ce  (bridge_wr_access || (cpu_mem_valid && cpu_mem_addr[31:28] == 4'h0)),
          .oe  (1'b1),
          .addr(bridge_wr_access ? bridge_addr_r[31:2] : ram_addr[31:2]),
          .do  (ram_rdata[(gi+1)*8-1:gi*8]),
          .di  (bridge_wr_access ? bridge_wr_data_r[(gi+1)*8-1:gi*8] : ram_wdata[(gi+1)*8-1:gi*8]),
          .we  (bridge_wr_access ? 1'b1 : ram_wstrb[gi])
      );
    end
  endgenerate

  picorv32 #(
      .COMPRESSED_ISA(1),
      .ENABLE_IRQ(1),
      .ENABLE_MUL(1),
      .ENABLE_DIV(1)
  ) u_cpu (
      .clk(clk_8mhz),
      .resetn(~rst),
      .mem_valid(cpu_mem_valid),
      .mem_instr(cpu_mem_instr),
      .mem_ready(cpu_mem_ready),
      .mem_addr(cpu_mem_addr),
      .mem_wdata(cpu_mem_wdata),
      .mem_wstrb(cpu_mem_wstrb),
      .mem_rdata(cpu_mem_rdata)
  );


  //


  bram_block_dp #(
      .DATA(32),
      .ADDR(8) // 1024 bytes in total
  ) u_bridge_dpram (
      .a_clk(clk_74a),
      .a_wr(bridge_wr && bridge_addr[31:28] == 4'h7),
      .a_addr(bridge_addr[31:2]),
      .a_din({
        bridge_wr_data[7:0], bridge_wr_data[15:8], bridge_wr_data[23:16], bridge_wr_data[31:24]
      }),
      .a_dout(  /* NC */),

      .b_clk (clk_8mhz),
      .b_wr  (1'b0),
      .b_addr(cpu_mem_addr[31:2]),
      .b_din (32'h0),
      .b_dout(bridge_dpram_rdata)
  );

  wire [31:0] dataslot_table_rd_data;
  wire [31:0] dataslot_table_rd_data_cpu;
  bram_block_dp #(
      .DATA(32),
      .ADDR(6)
  ) u_bridge_dataslot_table (
      .a_clk (clk_74a),
      .a_wr  (bridge_wr && bridge_addr[31:8] == 24'hF80020),
      .a_addr(bridge_addr[31:2]),
      .a_din (bridge_wr_data),
      .a_dout(dataslot_table_rd_data),

      .b_clk (clk_8mhz),
      .b_wr  (1'b0),
      .b_addr(cpu_mem_addr[31:2]),
      .b_din (32'h0),
      .b_dout(dataslot_table_rd_data_cpu)
  );

  reg [2:0] video_hs_delay;
  wire video_hs_wire;
  always @(posedge clk_8mhz) video_hs_delay <= {video_hs_wire, video_hs_delay[2:1]};

  wire [23:0] video_rgb_wire;
  assign video_rgb = video_de ? video_rgb_wire : 0;

  assign video_rgb_clock = clk_8mhz;
  assign video_rgb_clock_90 = clk_8mhz_90deg;
  assign video_skip = 0;
  assign video_hs = video_hs_delay[0];

  wire [15:0] mymig_cpu_mem_rdata;
  wire mymig_cpu_ack;

  wire [19:0] chip_ram_addr;
  wire [15:0] chip_ram_rdata;
  wire [15:0] chip_ram_wdata;
  wire chip_ram_we;

  mymig_top u_mymig(
    .clk(clk_8mhz),
    .rst(rst),
    // Video
    .o_video_rgb(video_rgb_wire),
    .o_video_de(video_de),
    .o_video_hsync(video_hs_wire),
    .o_video_vsync(video_vs),
    // CPU
    .i_cpu_addr(cpu_mem_addr[23:0]),
    .i_cpu_data(cpu_mem_wstrb == 4'b1100 ? cpu_mem_wdata[31:0] : cpu_mem_wdata[15:0]),
    .i_cpu_req(cpu_mem_addr[31:24] == 8'hff && cpu_mem_valid),
    .i_cpu_we(cpu_mem_wstrb != 0),
    .o_cpu_data(mymig_cpu_mem_rdata),
    .o_cpu_ack(mymig_cpu_ack),
    // Chip RAM
    .o_chip_ram_addr(chip_ram_addr),
    .i_chip_ram_data(chip_ram_rdata),
    .o_chip_ram_data(chip_ram_wdata),
    .o_chip_ram_we(chip_ram_we)
  );

  // 128KB CHIP RAM
  spram #(
      .aw(16),
      .dw(16)
  ) u_chip_ram (
      .clk (clk_32mhz),
      .rst (rst),
      .ce  (1'b1),
      .oe  (1'b1),
      .addr(chip_ram_addr),
      .do  (chip_ram_rdata),
      .di  (chip_ram_wdata),
      .we  (chip_ram_we)
  );

  wire uart_tx;
  assign cart_tran_bank0[6]  = uart_tx;
  ila_simple #(
    .WIDTH(72),
    .DEPTH(9),
    .CLK_FREQ(8e6)
  ) u_ila(
    .clk(clk_8mhz),
    .rst(~pll_core_locked),
//    .i_trig(cpu_mem_valid),
//    .i_sample({cpu_mem_rdata, cpu_mem_addr}),
    .i_trig(bridge_wr_pulse),
    .i_sample({{8{bridge_wr_pulse}}, bridge_wr_data_r, bridge_addr_r}),
    .o_uart_tx(uart_tx)
  );

endmodule
