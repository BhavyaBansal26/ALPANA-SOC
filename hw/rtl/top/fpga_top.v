
module fpga_top
  (
  // ---- JTAG ports ------- //
   // ---- UART ports --------//
    input         uart0_SIN,
    output        uart0_SOUT,

//   // ---- I2C ports --------//
    inout         i2c0_sda,
    inout         i2c0_scl,
    inout         i2c1_sda,
    inout         i2c1_scl,

//   // ---- SPI ports ---------//
   output       spi0_mosi,
// output       spi0_sclk,// if external flash enable this or disable if onboard flash
   output       spi0_nss,
   input        spi0_miso,
   /*output       spi1_mosi,
   output       spi1_sclk,
   output       spi1_nss,
   input        spi1_miso,*/

  // ---- io_cell_ports(Pin Muxed) ---------//
    inout io7_cell,
    inout io8_cell,
    inout io9_cell,
    inout io10_cell,
    inout io12_cell,
    inout io13_cell,
    inout io16_cell,
    inout io17_cell,
    inout io18_cell,
    inout io19_cell,
    inout io20_cell,


    // ---- GPIO ports --------//
    inout gpio_4,
    inout gpio_7,
    inout gpio_8,
    inout gpio_14,
    inout gpio_15,
    inout gpio_16,
    inout gpio_17,
    inout gpio_18,
    inout gpio_19,
    inout gpio_20,
    inout gpio_21,
    inout gpio_22,
    inout gpio_23,
    inout gpio_24,
    inout gpio_25,
    inout gpio_26,
    inout gpio_27,
    inout gpio_28,
    inout gpio_29,
    inout gpio_30,
    inout gpio_31,

    // ---- External interrupts ports --------//
    input [1:0] interrupts,


    // ---- DDR2 (Nexys4 DDR, MT47H64M16HR-25E, 128 MB) ----//
    inout  [15:0] ddr2_dq,
    inout  [1:0]  ddr2_dqs_n,
    inout  [1:0]  ddr2_dqs_p,
    output [12:0] ddr2_addr,
    output [2:0]  ddr2_ba,
    output        ddr2_ras_n,
    output        ddr2_cas_n,
    output        ddr2_we_n,
    output [0:0]  ddr2_ck_p,
    output [0:0]  ddr2_ck_n,
    output [0:0]  ddr2_cke,
    output [0:0]  ddr2_cs_n,
    output [1:0]  ddr2_dm,
    output [0:0]  ddr2_odt,

    // ---- System Reset ------//
    input         sys_rst,	//Active Low

    // ---- System Clock ------//
    input         sys_clk
   );

   // Wire instantiations

   wire                            soc_reset;      // reset to the SoC
   wire                            core_clk;       // clock to the SoC
   wire                            locked;         // indicates pll is stable
   wire                            spi0_sclk;      // SPI0 clock -> on-board flash via STARTUPE2

   wire i2c0_scl_out, i2c0_scl_in, i2c0_scl_out_en;
   wire i2c0_sda_out, i2c0_sda_in, i2c0_sda_out_en;
   wire i2c1_scl_out, i2c1_scl_in, i2c1_scl_out_en;
   wire i2c1_sda_out, i2c1_sda_in, i2c1_sda_out_en;

   wire io7_cell_in, io7_cell_out, io7_cell_en;
   wire io8_cell_in, io8_cell_out, io8_cell_en;
   wire io9_cell_in, io9_cell_out, io9_cell_en;
   wire io10_cell_in, io10_cell_out, io10_cell_en;
   wire io12_cell_in, io12_cell_out, io12_cell_en;
   wire io13_cell_in, io13_cell_out, io13_cell_en;
   wire io16_cell_in, io16_cell_out, io16_cell_en;
   wire io17_cell_in, io17_cell_out, io17_cell_en;
   wire io18_cell_in, io18_cell_out, io18_cell_en;
   wire io19_cell_in, io19_cell_out, io19_cell_en;
   wire io20_cell_in, io20_cell_out, io20_cell_en;

   wire gpio_4_in, gpio_4_out, gpio_4_en;
   wire gpio_7_in, gpio_7_out, gpio_7_en;
   wire gpio_8_in, gpio_8_out, gpio_8_en;
   wire gpio_14_in, gpio_14_out, gpio_14_en;
   wire gpio_15_in, gpio_15_out, gpio_15_en;
   wire gpio_16_in, gpio_16_out, gpio_16_en;
   wire gpio_17_in, gpio_17_out, gpio_17_en;
   wire gpio_18_in, gpio_18_out, gpio_18_en;
   wire gpio_19_in, gpio_19_out, gpio_19_en;
   wire gpio_20_in, gpio_20_out, gpio_20_en;
   wire gpio_21_in, gpio_21_out, gpio_21_en;
   wire gpio_22_in, gpio_22_out, gpio_22_en;
   wire gpio_23_in, gpio_23_out, gpio_23_en;
   wire gpio_24_in, gpio_24_out, gpio_24_en;
   wire gpio_25_in, gpio_25_out, gpio_25_en;
   wire gpio_26_in, gpio_26_out, gpio_26_en;
   wire gpio_27_in, gpio_27_out, gpio_27_en;
   wire gpio_28_in, gpio_28_out, gpio_28_en;
   wire gpio_29_in, gpio_29_out, gpio_29_en;
   wire gpio_30_in, gpio_30_out, gpio_30_en;
   wire gpio_31_in, gpio_31_out, gpio_31_en;



   // ---- SoC main-memory AXI4 master ----
   wire mem_awvalid;
   wire [31:0] mem_awaddr;
   wire [2:0] mem_awprot;
   wire [7:0] mem_awlen;
   wire [2:0] mem_awsize;
   wire [1:0] mem_awburst;
   wire [3:0] mem_awid;
   wire mem_awready;
   wire mem_wvalid;
   wire [31:0] mem_wdata;
   wire [3:0] mem_wstrb;
   wire mem_wlast;
   wire mem_wready;
   wire mem_bvalid;
   wire [1:0] mem_bresp;
   wire [3:0] mem_bid;
   wire mem_bready;
   wire mem_arvalid;
   wire [31:0] mem_araddr;
   wire [2:0] mem_arprot;
   wire [7:0] mem_arlen;
   wire [2:0] mem_arsize;
   wire [1:0] mem_arburst;
   wire [3:0] mem_arid;
   wire mem_arready;
   wire mem_rvalid;
   wire [1:0] mem_rresp;
   wire [31:0] mem_rdata;
   wire mem_rlast;
   wire [3:0] mem_rid;
   wire mem_rready;
   wire [3:0] mem_wid;

// ---------------------------------------------------------------------------- //
    wire wire_tck_clk;
    wire wire_trst;
    wire wire_capture;
    wire wire_run_test;
    wire wire_sel;
    wire wire_shift;
    wire wire_tdi;
    wire wire_tms;
    wire wire_update;
    wire wire_tdo;

    BSCANE2 #(
      .JTAG_CHAIN(4) // Value for USER command.
    )
    bse2_inst (
      .CAPTURE(wire_capture), // 1-bit output: CAPTURE output from TAP controller.
      .DRCK(), // 1-bit output: Gated TCK output. When SEL is asserted, DRCK toggles when CAPTURE or SHIFT are asserted.
      .RESET(wire_trst), // 1-bit output: Reset output for TAP controller.
      .RUNTEST(wire_run_test), // 1-bit output: Output asserted when TAP controller is in Run Test/Idle state.
      .SEL(wire_sel), // 1-bit output: USER instruction active output.
      .SHIFT(wire_shift), // 1-bit output: SHIFT output from TAP controller.
      .TCK(wire_tck_clk), // 1-bit output: Test Clock output. Fabric connection to TAP Clock pin.
      .TDI(wire_tdi), // 1-bit output: Test Data Input (TDI) output from TAP controller.
      .TMS(wire_tms), // 1-bit output: Test Mode Select output. Fabric connection to TAP.
      .UPDATE(wire_update), // 1-bit output: UPDATE output from TAP controller
      .TDO(wire_tdo) // 1-bit input: Test Data Output (TDO) input for USER function.
    );

   // ---------------------------------------------------------------------------- //
   // ---- reset: release the SoC only after PLL lock AND DDR2 calibration ----
   wire mig_clk_200;          // 200 MHz from clk_wiz_0 clk_out2 -> MIG sys_clk_i
   wire ui_clk;               // MIG user clock (81.25 MHz with 325 MHz DDR2, 4:1)
   wire ui_clk_sync_rst;
   wire mmcm_locked;
   wire init_calib_complete;
   reg  [3:0] rst_sync = 4'b0;
   always @(posedge core_clk or negedge locked)
     if (!locked) rst_sync <= 4'b0;
     else         rst_sync <= {rst_sync[2:0], init_calib_complete};
   assign soc_reset = rst_sync[3];          // active-low RST_N to the SoC
   reg [3:0] ui_rst_n = 4'b0;               // aresetn for the ui_clk side
   always @(posedge ui_clk) ui_rst_n <= {ui_rst_n[2:0], ~ui_clk_sync_rst & mmcm_locked};

   // ---------- Clock divider ----------------//

   // Clocking Wizard IP "clk_wiz_0": 100 MHz in -> core clock out, default ACTIVE-HIGH 'reset' port.
   // (If you instead set Reset Type = Active Low in the IP, connect .resetn(sys_rst).)
   clk_wiz_0 clk_div (
                       .clk_in1(sys_clk),
                       .clk_out1(core_clk),
                       .clk_out2(mig_clk_200),
                       .reset(~sys_rst),
                       .locked(locked) );
   // ----------------------------------------- //


   // ---- Instantiating the E-class SoC (4-stage, RV32IMAFC) -------------//
`ifdef SOC_PLAIN_NAMES
   mkSoc core(
`else
   mkSoc_e32 core(      // _e32 netlist: use the files from fpga_a35/verilog_e32
`endif
       // Main Clock and Reset to the SoC
        .CLK(core_clk),
        .RST_N(soc_reset),

        .CLK_tck_clk(wire_tck_clk),
        .RST_N_trst(~wire_trst),
        .wire_capture_capture_in(wire_capture),
        .wire_run_test_run_test_in(wire_run_test),
        .wire_sel_sel_in(wire_sel),
        .wire_shift_shift_in(wire_shift),
        .wire_tdi_tdi_in(wire_tdi),
        .wire_tms_tms_in(wire_tms),
        .wire_update_update_in(wire_update),
        .wire_tdo(wire_tdo),

      // SPI ports
        .spi0_io_mosi(spi0_mosi),
        .spi0_io_sclk(spi0_sclk),
        .spi0_io_nss(spi0_nss),
	      .spi0_io_miso_dat(spi0_miso),
        /*.spi1_io_mosi(spi1_mosi),
        .spi1_io_sclk(spi1_sclk),
        .spi1_io_nss(spi1_nss),
				.spi1_io_miso_dat(spi1_miso),*/

       // UART port definitions
        .uart0_io_SIN(uart0_SIN),
        .uart0_io_SOUT(uart0_SOUT),

			  //I2C ports
        .i2c0_out_scl_out (i2c0_scl_out),
        .i2c0_out_scl_in_in(i2c0_scl_in),
        .i2c0_out_scl_out_en(i2c0_scl_out_en),
        .i2c0_out_sda_out(i2c0_sda_out),
        .i2c0_out_sda_in_in(i2c0_sda_in),
        .i2c0_out_sda_out_en(i2c0_sda_out_en),

			  //I2C ports
        .i2c1_out_scl_out (i2c1_scl_out),
        .i2c1_out_scl_in_in(i2c1_scl_in),
        .i2c1_out_scl_out_en(i2c1_scl_out_en),
        .i2c1_out_sda_out(i2c1_sda_out),
        .i2c1_out_sda_in_in(i2c1_sda_in),
        .i2c1_out_sda_out_en(i2c1_sda_out_en),

        //pin muxed pins
    	  .iocell_io_io7_cell_in_in(io7_cell_in),
    	  .iocell_io_io8_cell_in_in(io8_cell_in),
    	  .iocell_io_io9_cell_in_in(io9_cell_in),
    	  .iocell_io_io10_cell_in_in(io10_cell_in),
    	  .iocell_io_io12_cell_in_in(io12_cell_in),
    	  .iocell_io_io13_cell_in_in(io13_cell_in),
    	  .iocell_io_io16_cell_in_in(io16_cell_in),
    	  .iocell_io_io17_cell_in_in(io17_cell_in),
    	  .iocell_io_io18_cell_in_in(io18_cell_in),
    	  .iocell_io_io19_cell_in_in(io19_cell_in),
    	  .iocell_io_io20_cell_in_in(io20_cell_in),
    	  .iocell_io_io7_cell_out(io7_cell_out),
    	  .iocell_io_io8_cell_out(io8_cell_out),
    	  .iocell_io_io9_cell_out(io9_cell_out),
    	  .iocell_io_io10_cell_out(io10_cell_out),
    	  .iocell_io_io12_cell_out(io12_cell_out),
    	  .iocell_io_io13_cell_out(io13_cell_out),
    	  .iocell_io_io16_cell_out(io16_cell_out),
    	  .iocell_io_io17_cell_out(io17_cell_out),
    	  .iocell_io_io18_cell_out(io18_cell_out),
    	  .iocell_io_io19_cell_out(io19_cell_out),
    	  .iocell_io_io20_cell_out(io20_cell_out),
    	  .iocell_io_io7_cell_outen(io7_cell_en),
    	  .iocell_io_io8_cell_outen(io8_cell_en),
    	  .iocell_io_io9_cell_outen(io9_cell_en),
    	  .iocell_io_io10_cell_outen(io10_cell_en),
    	  .iocell_io_io12_cell_outen(io12_cell_en),
    	  .iocell_io_io13_cell_outen(io13_cell_en),
    	  .iocell_io_io16_cell_outen(io16_cell_en),
    	  .iocell_io_io17_cell_outen(io17_cell_en),
    	  .iocell_io_io18_cell_outen(io18_cell_en),
    	  .iocell_io_io19_cell_outen(io19_cell_en),
    	  .iocell_io_io20_cell_outen(io20_cell_en),

        //GPIO
        .gpio_4_in(gpio_4_in),
        .gpio_7_in(gpio_7_in),
        .gpio_8_in(gpio_8_in),
        .gpio_14_in(gpio_14_in),
        .gpio_15_in(gpio_15_in),
        .gpio_16_in(gpio_16_in),
        .gpio_17_in(gpio_17_in),
        .gpio_18_in(gpio_18_in),
        .gpio_19_in(gpio_19_in),
        .gpio_20_in(gpio_20_in),
        .gpio_21_in(gpio_21_in),
        .gpio_22_in(gpio_22_in),
        .gpio_23_in(gpio_23_in),
        .gpio_24_in(gpio_24_in),
        .gpio_25_in(gpio_25_in),
        .gpio_26_in(gpio_26_in),
        .gpio_27_in(gpio_27_in),
        .gpio_28_in(gpio_28_in),
        .gpio_29_in(gpio_29_in),
        .gpio_30_in(gpio_30_in),
        .gpio_31_in(gpio_31_in),
        .gpio_4_out(gpio_4_out),
        .gpio_7_out(gpio_7_out),
        .gpio_8_out(gpio_8_out),
        .gpio_14_out(gpio_14_out),
        .gpio_15_out(gpio_15_out),
        .gpio_16_out(gpio_16_out),
        .gpio_17_out(gpio_17_out),
        .gpio_18_out(gpio_18_out),
        .gpio_19_out(gpio_19_out),
        .gpio_20_out(gpio_20_out),
        .gpio_21_out(gpio_21_out),
        .gpio_22_out(gpio_22_out),
        .gpio_23_out(gpio_23_out),
        .gpio_24_out(gpio_24_out),
        .gpio_25_out(gpio_25_out),
        .gpio_26_out(gpio_26_out),
        .gpio_27_out(gpio_27_out),
        .gpio_28_out(gpio_28_out),
        .gpio_29_out(gpio_29_out),
        .gpio_30_out(gpio_30_out),
        .gpio_31_out(gpio_31_out),
        .gpio_4_outen(gpio_4_en),
        .gpio_7_outen(gpio_7_en),
        .gpio_8_outen(gpio_8_en),
        .gpio_14_outen(gpio_14_en),
        .gpio_15_outen(gpio_15_en),
        .gpio_16_outen(gpio_16_en),
        .gpio_17_outen(gpio_17_en),
        .gpio_18_outen(gpio_18_en),
        .gpio_19_outen(gpio_19_en),
        .gpio_20_outen(gpio_20_en),
        .gpio_21_outen(gpio_21_en),
        .gpio_22_outen(gpio_22_en),
        .gpio_23_outen(gpio_23_en),
        .gpio_24_outen(gpio_24_en),
        .gpio_25_outen(gpio_25_en),
        .gpio_26_outen(gpio_26_en),
        .gpio_27_outen(gpio_27_en),
        .gpio_28_outen(gpio_28_en),
        .gpio_29_outen(gpio_29_en),
        .gpio_30_outen(gpio_30_en),
        .gpio_31_outen(gpio_31_en),

        // main memory (DDR2 via clock converter + MIG)
        .mem_master_AWVALID(mem_awvalid),
        .mem_master_AWADDR(mem_awaddr),
        .mem_master_AWPROT(mem_awprot),
        .mem_master_AWLEN(mem_awlen),
        .mem_master_AWSIZE(mem_awsize),
        .mem_master_AWBURST(mem_awburst),
        .mem_master_AWID(mem_awid),
        .mem_master_AWREADY(mem_awready),
        .mem_master_WVALID(mem_wvalid),
        .mem_master_WDATA(mem_wdata),
        .mem_master_WSTRB(mem_wstrb),
        .mem_master_WLAST(mem_wlast),
        .mem_master_WREADY(mem_wready),
        .mem_master_BVALID(mem_bvalid),
        .mem_master_BRESP(mem_bresp),
        .mem_master_BID(mem_bid),
        .mem_master_BREADY(mem_bready),
        .mem_master_ARVALID(mem_arvalid),
        .mem_master_ARADDR(mem_araddr),
        .mem_master_ARPROT(mem_arprot),
        .mem_master_ARLEN(mem_arlen),
        .mem_master_ARSIZE(mem_arsize),
        .mem_master_ARBURST(mem_arburst),
        .mem_master_ARID(mem_arid),
        .mem_master_ARREADY(mem_arready),
        .mem_master_RVALID(mem_rvalid),
        .mem_master_RRESP(mem_rresp),
        .mem_master_RDATA(mem_rdata),
        .mem_master_RLAST(mem_rlast),
        .mem_master_RID(mem_rid),
        .mem_master_RREADY(mem_rready),
        .mem_master_WID(mem_wid),
        .ext_interrupts_i(interrupts)
   );
/*   // ---- (unused) generic GPIO tristate loop -------------//
   genvar index;
   generate
   for(index=0; index<32; index= index+1)
      begin: connect_gpio_tristates
      IOBUF gpio_iobuf_inst (
             .O(gpio_in[index]),
             .IO(gpio[index]),
             .I(gpio_out[index]),
             .T(~gpio_out_en[index])
				 );
      end
   endgenerate*/
//---- IOBUF FOR I2C -----//
   IOBUF i2c0_scl_inst(
             .O(i2c0_scl_in),
             .IO(i2c0_scl),
             .I(i2c0_scl_out),
             .T(~i2c0_scl_out_en)
         );

   IOBUF i2c0_sda_inst(
             .O(i2c0_sda_in),
             .IO(i2c0_sda),
             .I(i2c0_sda_out),
             .T(~i2c0_sda_out_en)
         );

   //---- IOBUF FOR I2C -----//
   IOBUF i2c1_scl_inst(
             .O(i2c1_scl_in),
             .IO(i2c1_scl),
             .I(i2c1_scl_out),
             .T(~i2c1_scl_out_en)
         );

   IOBUF i2c1_sda_inst(
             .O(i2c1_sda_in),
             .IO(i2c1_sda),
             .I(i2c1_sda_out),
             .T(~i2c1_sda_out_en)
         );
  //----IOBUF FOR MUXED PIN----//

   IOBUF io7_cell_inst(
             .O(io7_cell_in),
             .IO(io7_cell),
             .I(io7_cell_out),
             .T(io7_cell_en)
         );

   IOBUF io8_cell_inst(
             .O(io8_cell_in),
             .IO(io8_cell),
             .I(io8_cell_out),
             .T(io8_cell_en)
         );

   IOBUF io9_cell_inst(
             .O(io9_cell_in),
             .IO(io9_cell),
             .I(io9_cell_out),
             .T(io9_cell_en)
         );

   IOBUF io10_cell_inst(
             .O(io10_cell_in),
             .IO(io10_cell),
             .I(io10_cell_out),
             .T(io10_cell_en)
         );

   IOBUF io12_cell_inst(
             .O(io12_cell_in),
             .IO(io12_cell),
             .I(io12_cell_out),
             .T(io12_cell_en)
         );

   IOBUF io13_cell_inst(
             .O(io13_cell_in),
             .IO(io13_cell),
             .I(io13_cell_out),
             .T(io13_cell_en)
         );

   IOBUF io16_cell_inst(
             .O(io16_cell_in),
             .IO(io16_cell),
             .I(io16_cell_out),
             .T(io16_cell_en)
         );

   IOBUF io17_cell_inst(
             .O(io17_cell_in),
             .IO(io17_cell),
             .I(io17_cell_out),
             .T(io17_cell_en)
         );

   IOBUF io18_cell_inst(
             .O(io18_cell_in),
             .IO(io18_cell),
             .I(io18_cell_out),
             .T(io18_cell_en)
         );

   IOBUF io19_cell_inst(
             .O(io19_cell_in),
             .IO(io19_cell),
             .I(io19_cell_out),
             .T(io19_cell_en)
         );

   IOBUF io20_cell_inst(
             .O(io20_cell_in),
             .IO(io20_cell),
             .I(io20_cell_out),
             .T(io20_cell_en)
         );
   //-----IOBUF FOR GPIO-----//

   IOBUF gpio_4_inst(
             .O(gpio_4_in),
             .IO(gpio_4),
             .I(gpio_4_out),
             .T(~gpio_4_en)
         );

   IOBUF gpio_7_inst(
             .O(gpio_7_in),
             .IO(gpio_7),
             .I(gpio_7_out),
             .T(~gpio_7_en)
         );

   IOBUF gpio_8_inst(
             .O(gpio_8_in),
             .IO(gpio_8),
             .I(gpio_8_out),
             .T(~gpio_8_en)
         );

   IOBUF gpio_14_inst(
             .O(gpio_14_in),
             .IO(gpio_14),
             .I(gpio_14_out),
             .T(~gpio_14_en)
         );

   IOBUF gpio_15_inst(.O(gpio_15_in),.IO(gpio_15),.I(gpio_15_out),.T(~gpio_15_en));
   IOBUF gpio_16_inst(.O(gpio_16_in),.IO(gpio_16),.I(gpio_16_out),.T(~gpio_16_en));
   IOBUF gpio_17_inst(.O(gpio_17_in),.IO(gpio_17),.I(gpio_17_out),.T(~gpio_17_en));
   IOBUF gpio_18_inst(.O(gpio_18_in),.IO(gpio_18),.I(gpio_18_out),.T(~gpio_18_en));
   IOBUF gpio_19_inst(.O(gpio_19_in),.IO(gpio_19),.I(gpio_19_out),.T(~gpio_19_en));
   IOBUF gpio_20_inst(.O(gpio_20_in),.IO(gpio_20),.I(gpio_20_out),.T(~gpio_20_en));
   IOBUF gpio_21_inst(.O(gpio_21_in),.IO(gpio_21),.I(gpio_21_out),.T(~gpio_21_en));
   IOBUF gpio_22_inst(.O(gpio_22_in),.IO(gpio_22),.I(gpio_22_out),.T(~gpio_22_en));
   IOBUF gpio_23_inst(.O(gpio_23_in),.IO(gpio_23),.I(gpio_23_out),.T(~gpio_23_en));
   IOBUF gpio_24_inst(.O(gpio_24_in),.IO(gpio_24),.I(gpio_24_out),.T(~gpio_24_en));
   IOBUF gpio_25_inst(.O(gpio_25_in),.IO(gpio_25),.I(gpio_25_out),.T(~gpio_25_en));
   IOBUF gpio_26_inst(.O(gpio_26_in),.IO(gpio_26),.I(gpio_26_out),.T(~gpio_26_en));
   IOBUF gpio_27_inst(.O(gpio_27_in),.IO(gpio_27),.I(gpio_27_out),.T(~gpio_27_en));
   IOBUF gpio_28_inst(.O(gpio_28_in),.IO(gpio_28),.I(gpio_28_out),.T(~gpio_28_en));
   IOBUF gpio_29_inst(.O(gpio_29_in),.IO(gpio_29),.I(gpio_29_out),.T(~gpio_29_en));
   IOBUF gpio_30_inst(.O(gpio_30_in),.IO(gpio_30),.I(gpio_30_out),.T(~gpio_30_en));
   IOBUF gpio_31_inst(.O(gpio_31_in),.IO(gpio_31),.I(gpio_31_out),.T(~gpio_31_en));

// Enable if onboard flash or disable if external flash
	 STARTUPE2#(.PROG_USR("False"),
               .SIM_CCLK_FREQ(0.0))   startupe2_inst1(
               .CFGCLK(),
               .CFGMCLK(),
               .EOS(),
               .PREQ(),
               .CLK(0),
               .GSR(0),
               .GTS(0),
               .KEYCLEARB(0),
               .PACK(0),
               .USRCCLKO(spi0_sclk),
               .USRCCLKTS(0),
               .USRDONEO(1),
               .USRDONETS(1));


   // ---- AXI4 clock-domain crossing: core_clk (80 MHz) -> MIG ui_clk -------------//
   wire [3:0]  m_awid, m_bid, m_arid, m_rid;
   wire [31:0] m_awaddr, m_araddr, m_wdata, m_rdata;
   wire [7:0]  m_awlen, m_arlen;
   wire [2:0]  m_awsize, m_arsize, m_awprot, m_arprot;
   wire [1:0]  m_awburst, m_arburst, m_bresp, m_rresp;
   wire [0:0]  m_awlock, m_arlock;
   wire [3:0]  m_awcache, m_arcache, m_awqos, m_arqos, m_wstrb;
   wire m_awvalid, m_awready, m_wlast, m_wvalid, m_wready, m_bvalid, m_bready;
   wire m_arvalid, m_arready, m_rlast, m_rvalid, m_rready;

   axi_clock_converter_0 mem_cdc (
        .s_axi_aclk(core_clk),   .s_axi_aresetn(soc_reset),
        .s_axi_awid(mem_awid),   .s_axi_awaddr(mem_awaddr), .s_axi_awlen(mem_awlen),
        .s_axi_awsize(mem_awsize), .s_axi_awburst(mem_awburst), .s_axi_awlock(1'b0),
        .s_axi_awcache(4'b0011), .s_axi_awprot(mem_awprot), .s_axi_awregion(4'b0), .s_axi_awqos(4'b0),
        .s_axi_awvalid(mem_awvalid), .s_axi_awready(mem_awready),
        .s_axi_wdata(mem_wdata), .s_axi_wstrb(mem_wstrb), .s_axi_wlast(mem_wlast),
        .s_axi_wvalid(mem_wvalid), .s_axi_wready(mem_wready),
        .s_axi_bid(mem_bid), .s_axi_bresp(mem_bresp), .s_axi_bvalid(mem_bvalid), .s_axi_bready(mem_bready),
        .s_axi_arid(mem_arid),   .s_axi_araddr(mem_araddr), .s_axi_arlen(mem_arlen),
        .s_axi_arsize(mem_arsize), .s_axi_arburst(mem_arburst), .s_axi_arlock(1'b0),
        .s_axi_arcache(4'b0011), .s_axi_arprot(mem_arprot), .s_axi_arregion(4'b0), .s_axi_arqos(4'b0),
        .s_axi_arvalid(mem_arvalid), .s_axi_arready(mem_arready),
        .s_axi_rid(mem_rid), .s_axi_rdata(mem_rdata), .s_axi_rresp(mem_rresp), .s_axi_rlast(mem_rlast),
        .s_axi_rvalid(mem_rvalid), .s_axi_rready(mem_rready),

        .m_axi_aclk(ui_clk),     .m_axi_aresetn(ui_rst_n[3]),
        .m_axi_awid(m_awid),     .m_axi_awaddr(m_awaddr), .m_axi_awlen(m_awlen), .m_axi_awsize(m_awsize),
        .m_axi_awburst(m_awburst), .m_axi_awlock(m_awlock), .m_axi_awcache(m_awcache), .m_axi_awprot(m_awprot),
        .m_axi_awregion(), .m_axi_awqos(m_awqos), .m_axi_awvalid(m_awvalid), .m_axi_awready(m_awready),
        .m_axi_wdata(m_wdata), .m_axi_wstrb(m_wstrb), .m_axi_wlast(m_wlast), .m_axi_wvalid(m_wvalid), .m_axi_wready(m_wready),
        .m_axi_bid(m_bid), .m_axi_bresp(m_bresp), .m_axi_bvalid(m_bvalid), .m_axi_bready(m_bready),
        .m_axi_arid(m_arid),     .m_axi_araddr(m_araddr), .m_axi_arlen(m_arlen), .m_axi_arsize(m_arsize),
        .m_axi_arburst(m_arburst), .m_axi_arlock(m_arlock), .m_axi_arcache(m_arcache), .m_axi_arprot(m_arprot),
        .m_axi_arregion(), .m_axi_arqos(m_arqos), .m_axi_arvalid(m_arvalid), .m_axi_arready(m_arready),
        .m_axi_rid(m_rid), .m_axi_rdata(m_rdata), .m_axi_rresp(m_rresp), .m_axi_rlast(m_rlast),
        .m_axi_rvalid(m_rvalid), .m_axi_rready(m_rready) );

   // ---- DDR2 controller (MIG 7-series, AXI4 slave, 128 MB) --------------------//
   // SoC address 0x8000_0000..0x87FF_FFFF -> DDR2 byte address [26:0]
   mig_7series_0 ddr2_ctrl (
        .ddr2_dq(ddr2_dq), .ddr2_dqs_n(ddr2_dqs_n), .ddr2_dqs_p(ddr2_dqs_p),
        .ddr2_addr(ddr2_addr), .ddr2_ba(ddr2_ba), .ddr2_ras_n(ddr2_ras_n), .ddr2_cas_n(ddr2_cas_n),
        .ddr2_we_n(ddr2_we_n), .ddr2_ck_p(ddr2_ck_p), .ddr2_ck_n(ddr2_ck_n), .ddr2_cke(ddr2_cke),
        .ddr2_cs_n(ddr2_cs_n), .ddr2_dm(ddr2_dm), .ddr2_odt(ddr2_odt),

        .sys_clk_i(mig_clk_200),          // System Clock: No Buffer, 200 MHz; Reference Clock: Use System Clock
        .sys_rst(locked),                 // active-low reset (MIG "System Reset Polarity" = ACTIVE LOW)
        .ui_clk(ui_clk), .ui_clk_sync_rst(ui_clk_sync_rst), .mmcm_locked(mmcm_locked),
        .aresetn(ui_rst_n[3]),
        .app_sr_req(1'b0), .app_ref_req(1'b0),
        .app_sr_active(), .app_ref_ack(),
        .init_calib_complete(init_calib_complete),

        .s_axi_awid(m_awid), .s_axi_awaddr(m_awaddr[26:0]), .s_axi_awlen(m_awlen), .s_axi_awsize(m_awsize),
        .s_axi_awburst(m_awburst), .s_axi_awlock(m_awlock), .s_axi_awcache(m_awcache), .s_axi_awprot(m_awprot),
        .s_axi_awqos(m_awqos), .s_axi_awvalid(m_awvalid), .s_axi_awready(m_awready),
        .s_axi_wdata(m_wdata), .s_axi_wstrb(m_wstrb), .s_axi_wlast(m_wlast), .s_axi_wvalid(m_wvalid), .s_axi_wready(m_wready),
        .s_axi_bready(m_bready), .s_axi_bid(m_bid), .s_axi_bresp(m_bresp), .s_axi_bvalid(m_bvalid),
        .s_axi_arid(m_arid), .s_axi_araddr(m_araddr[26:0]), .s_axi_arlen(m_arlen), .s_axi_arsize(m_arsize),
        .s_axi_arburst(m_arburst), .s_axi_arlock(m_arlock), .s_axi_arcache(m_arcache), .s_axi_arprot(m_arprot),
        .s_axi_arqos(m_arqos), .s_axi_arvalid(m_arvalid), .s_axi_arready(m_arready),
        .s_axi_rready(m_rready), .s_axi_rid(m_rid), .s_axi_rdata(m_rdata), .s_axi_rresp(m_rresp),
        .s_axi_rlast(m_rlast), .s_axi_rvalid(m_rvalid) );

endmodule