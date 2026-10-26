## =================================================================================================


## ---------------------------------------- Clock -------------------------------------------------
set_property -dict { PACKAGE_PIN E3    IOSTANDARD LVCMOS33 } [get_ports { sys_clk }]; #IO_L12P_T1_MRCC_35 Sch=clk100mhz
# removed: clk_wiz_0 IP already creates the input clock (duplicate caused clk_out1_clk_wiz_0_1)
# create_clock -add -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports { sys_clk }];
# removed: net no longer exists (clk_wiz has its own IBUF)
# set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets sys_clk_IBUF]

## JTAG clock from BSCANE2 (debug module) - asynchronous to the core clock
create_clock -name tck -period 100.000 [get_pins bse2_inst/TCK]
set_clock_groups -asynchronous -group [get_clocks tck] -group [get_clocks -of_objects [get_pins clk_div/inst/plle2_adv_inst/CLKOUT0]]

## ---------------------------------------- Reset -------------------------------------------------
set_property -dict { PACKAGE_PIN C12   IOSTANDARD LVCMOS33 } [get_ports { sys_rst }]; #IO_L3P_T0_DQS_AD1P_15 Sch=cpu_resetn

## ---------------------------------------- UART0 (USB-RS232) -------------------------------------
set_property -dict { PACKAGE_PIN C4    IOSTANDARD LVCMOS33 } [get_ports { uart0_SIN }];  #IO_L7P_T1_AD6P_35 Sch=uart_txd_in
set_property -dict { PACKAGE_PIN D4    IOSTANDARD LVCMOS33 } [get_ports { uart0_SOUT }]; #IO_L11N_T1_SRCC_35 Sch=uart_rxd_out

## ---------------------------------------- SPI0 -> on-board Quad-SPI flash -----------------------
## SCLK is driven through STARTUPE2/USRCCLKO (CCLK pin), so it has no PACKAGE_PIN here.
set_property -dict { PACKAGE_PIN K17   IOSTANDARD LVCMOS33 } [get_ports { spi0_mosi }]; #IO_L1P_T0_D00_MOSI_14 Sch=qspi_dq[0]
set_property -dict { PACKAGE_PIN K18   IOSTANDARD LVCMOS33 } [get_ports { spi0_miso }]; #IO_L1N_T0_D01_DIN_14 Sch=qspi_dq[1]
set_property -dict { PACKAGE_PIN L13   IOSTANDARD LVCMOS33 } [get_ports { spi0_nss }];  #IO_L6P_T0_FCS_B_14 Sch=qspi_csn

## ---------------------------------------- I2C0 -> on-board ADT7420 temperature sensor -----------
set_property -dict { PACKAGE_PIN C14   IOSTANDARD LVCMOS33 } [get_ports { i2c0_scl }]; #IO_L1N_T0_AD0N_15 Sch=tmp_scl
set_property -dict { PACKAGE_PIN C15   IOSTANDARD LVCMOS33 } [get_ports { i2c0_sda }]; #IO_L12N_T1_MRCC_15 Sch=tmp_sda
set_property PULLUP true [get_ports { i2c0_scl i2c0_sda }]

## ---------------------------------------- I2C1 -> Pmod JB ---------------------------------------
set_property -dict { PACKAGE_PIN H14   IOSTANDARD LVCMOS33 } [get_ports { i2c1_scl }]; #IO_L15P_T2_DQS_15 Sch=jb[4]
set_property -dict { PACKAGE_PIN H16   IOSTANDARD LVCMOS33 } [get_ports { i2c1_sda }]; #IO_L13P_T2_MRCC_15 Sch=jb[10]
set_property PULLUP true [get_ports { i2c1_scl i2c1_sda }]

## ---------------------------------------- Pin-muxed IO cells -> Pmod JA + JB --------------------
##   io7 GPIO0/UART1-RX   io8 GPIO1/UART1-TX   io9 GPIO2/UART2-RX  io10 GPIO3/UART3-TX/PWM0
##   io12 GPIO5/PWM1      io13 GPIO6/PWM2      io16 GPIO9/PWM3     io17 GPIO10/SPI1-NCS/PWM4
##   io18 GPIO11/SPI1-MOSI/PWM5   io19 GPIO12/SPI1-MISO   io20 GPIO13/SPI1-CLK
set_property -dict { PACKAGE_PIN C17   IOSTANDARD LVCMOS33 } [get_ports { io7_cell }];  #IO_L20N_T3_A19_15 Sch=ja[1]
set_property -dict { PACKAGE_PIN D18   IOSTANDARD LVCMOS33 } [get_ports { io8_cell }];  #IO_L21N_T3_DQS_A18_15 Sch=ja[2]
set_property -dict { PACKAGE_PIN E18   IOSTANDARD LVCMOS33 } [get_ports { io9_cell }];  #IO_L21P_T3_DQS_15 Sch=ja[3]
set_property -dict { PACKAGE_PIN G17   IOSTANDARD LVCMOS33 } [get_ports { io10_cell }]; #IO_L18N_T2_A23_15 Sch=ja[4]
set_property -dict { PACKAGE_PIN D17   IOSTANDARD LVCMOS33 } [get_ports { io12_cell }]; #IO_L16N_T2_A27_15 Sch=ja[7]
set_property -dict { PACKAGE_PIN E17   IOSTANDARD LVCMOS33 } [get_ports { io13_cell }]; #IO_L16P_T2_A28_15 Sch=ja[8]
set_property -dict { PACKAGE_PIN F18   IOSTANDARD LVCMOS33 } [get_ports { io16_cell }]; #IO_L22N_T3_A16_15 Sch=ja[9]
set_property -dict { PACKAGE_PIN G18   IOSTANDARD LVCMOS33 } [get_ports { io17_cell }]; #IO_L22P_T3_A17_15 Sch=ja[10]
set_property -dict { PACKAGE_PIN D14   IOSTANDARD LVCMOS33 } [get_ports { io18_cell }]; #IO_L1P_T0_AD0P_15 Sch=jb[1]
set_property -dict { PACKAGE_PIN F16   IOSTANDARD LVCMOS33 } [get_ports { io19_cell }]; #IO_L14N_T2_SRCC_15 Sch=jb[2]
set_property -dict { PACKAGE_PIN G16   IOSTANDARD LVCMOS33 } [get_ports { io20_cell }]; #IO_L13N_T2_MRCC_15 Sch=jb[3]
set_property PULLDOWN true [get_ports { io7_cell io8_cell io9_cell io10_cell io12_cell io13_cell io16_cell io17_cell io18_cell io19_cell io20_cell }]

## ---------------------------------------- GPIO --------------------------------------------------
## Switches SW0-SW4
set_property -dict { PACKAGE_PIN J15   IOSTANDARD LVCMOS33 } [get_ports { gpio_4 }];  #IO_L24N_T3_RS0_15 Sch=sw[0]
set_property -dict { PACKAGE_PIN L16   IOSTANDARD LVCMOS33 } [get_ports { gpio_7 }];  #IO_L3N_T0_DQS_EMCCLK_14 Sch=sw[1]
set_property -dict { PACKAGE_PIN M13   IOSTANDARD LVCMOS33 } [get_ports { gpio_8 }];  #IO_L6N_T0_D08_VREF_14 Sch=sw[2]
set_property -dict { PACKAGE_PIN R15   IOSTANDARD LVCMOS33 } [get_ports { gpio_14 }]; #IO_L13N_T2_MRCC_14 Sch=sw[3]
set_property -dict { PACKAGE_PIN R17   IOSTANDARD LVCMOS33 } [get_ports { gpio_15 }]; #IO_L12N_T1_MRCC_14 Sch=sw[4]

## LEDs LD0-LD3
set_property -dict { PACKAGE_PIN H17   IOSTANDARD LVCMOS33 } [get_ports { gpio_16 }]; #IO_L18P_T2_A24_15 Sch=led[0]
set_property -dict { PACKAGE_PIN K15   IOSTANDARD LVCMOS33 } [get_ports { gpio_17 }]; #IO_L24P_T3_RS1_15 Sch=led[1]
set_property -dict { PACKAGE_PIN J13   IOSTANDARD LVCMOS33 } [get_ports { gpio_18 }]; #IO_L17N_T2_A25_15 Sch=led[2]
set_property -dict { PACKAGE_PIN N14   IOSTANDARD LVCMOS33 } [get_ports { gpio_19 }]; #IO_L8P_T1_D11_14 Sch=led[3]

## Push buttons
set_property -dict { PACKAGE_PIN N17   IOSTANDARD LVCMOS33 } [get_ports { gpio_20 }]; #IO_L9P_T1_DQS_14 Sch=btnc
set_property -dict { PACKAGE_PIN M18   IOSTANDARD LVCMOS33 } [get_ports { gpio_21 }]; #IO_L4N_T0_D05_14 Sch=btnu
set_property -dict { PACKAGE_PIN P17   IOSTANDARD LVCMOS33 } [get_ports { gpio_22 }]; #IO_L12P_T1_MRCC_14 Sch=btnl
set_property -dict { PACKAGE_PIN M17   IOSTANDARD LVCMOS33 } [get_ports { gpio_23 }]; #IO_L10N_T1_D15_14 Sch=btnr

## Pmod JD
set_property -dict { PACKAGE_PIN H4    IOSTANDARD LVCMOS33 } [get_ports { gpio_24 }]; #IO_L21N_T3_DQS_35 Sch=jd[1]
set_property -dict { PACKAGE_PIN H1    IOSTANDARD LVCMOS33 } [get_ports { gpio_25 }]; #IO_L17P_T2_35 Sch=jd[2]
set_property -dict { PACKAGE_PIN G1    IOSTANDARD LVCMOS33 } [get_ports { gpio_26 }]; #IO_L17N_T2_35 Sch=jd[3]
set_property -dict { PACKAGE_PIN G3    IOSTANDARD LVCMOS33 } [get_ports { gpio_27 }]; #IO_L20N_T3_35 Sch=jd[4]
set_property -dict { PACKAGE_PIN H2    IOSTANDARD LVCMOS33 } [get_ports { gpio_28 }]; #IO_L15P_T2_DQS_35 Sch=jd[7]
set_property -dict { PACKAGE_PIN G4    IOSTANDARD LVCMOS33 } [get_ports { gpio_29 }]; #IO_L20P_T3_35 Sch=jd[8]
set_property -dict { PACKAGE_PIN G2    IOSTANDARD LVCMOS33 } [get_ports { gpio_30 }]; #IO_L15N_T2_DQS_35 Sch=jd[9]
set_property -dict { PACKAGE_PIN F3    IOSTANDARD LVCMOS33 } [get_ports { gpio_31 }]; #IO_L13N_T2_MRCC_35 Sch=jd[10]
set_property PULLDOWN true [get_ports { gpio_24 gpio_25 gpio_26 gpio_27 gpio_28 gpio_29 gpio_30 gpio_31 }]

## ---------------------------------------- External interrupts (PLIC) ----------------------------
set_property -dict { PACKAGE_PIN P18   IOSTANDARD LVCMOS33 } [get_ports { interrupts[0] }]; #IO_L9N_T1_DQS_D13_14 Sch=btnd
set_property -dict { PACKAGE_PIN V10   IOSTANDARD LVCMOS33 } [get_ports { interrupts[1] }]; #IO_L21P_T3_DQS_14 Sch=sw[15]

## ---------------------------------------- Configuration -----------------------------------------
set_property CFGBVS VCCO [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
## PWM synchronisers into the LUT-divided PWM clocks (Bluespec SyncBit/SyncPulse) - CDC by design
set_false_path -to [get_pins -hier -filter {NAME =~ *pwm_cluster/*sync*/dSyncReg1_reg/D}]

# ---- DDR2 integration timing fixes ----------------------------------------
# init_calib_complete (MIG ui_clk) -> rst_sync[0] (core_clk) is a CDC into a
# 4-flop synchronizer: not a real timing path
set_false_path -to [get_pins {rst_sync_reg[0]/D}]
set_property ASYNC_REG TRUE [get_cells {rst_sync_reg[0] rst_sync_reg[1]}]
# soc_reset fans out to the whole SoC: let synth/phys_opt replicate the driver
set_property MAX_FANOUT 64 [get_cells {rst_sync_reg[3]}]
