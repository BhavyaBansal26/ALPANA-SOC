# ============================================================================
# create_project.tcl : rebuild the ALPANA_SOC Vivado project from this repo
#   Vivado 2025.2 (other versions: the IPs are upgraded automatically)
#   Usage (Vivado Tcl console, or batch):
#     cd <repo>/hw/scripts
#     vivado -mode batch -source create_project.tcl          (or: source create_project.tcl)
#   Project is created in <repo>/hw/vivado/ALPANA_SOC (git-ignored).
#   Set USE_RTL_MULT 1 to use multiplier_rtl.v instead of the mult_gen IP.
# ============================================================================
set USE_RTL_MULT 0
set HW   [file normalize [file join [file dirname [info script]] ..]]
set PROJ [file join $HW vivado]

create_project ALPANA_SOC $PROJ -part xc7a100tcsg324-1 -force
set_property target_language Verilog [current_project]

# --- RTL -------------------------------------------------------------------
add_files [glob [file join $HW rtl soc *.v]]
add_files [file join $HW rtl top fpga_top.v]
add_files [file join $HW mem boot.mem]

# --- IPs -------------------------------------------------------------------
set ips {clk_wiz_0 mig_7series_0 axi_clock_converter_0}
if {$USE_RTL_MULT} {
  add_files [file join $HW rtl top multiplier_rtl.v]
} else {
  lappend ips multiplier
}
foreach ip $ips {
  # copy the .xci (and mig_a.prj) into the project so the repo copy stays clean
  set dst [file join $PROJ ip_src $ip]
  file mkdir $dst
  foreach f [glob [file join $HW ip $ip *]] { file copy -force $f $dst }
  add_files [file join $dst $ip.xci]
}
upgrade_ip [get_ips] -quiet
generate_target all [get_ips]

# --- constraints + top -------------------------------------------------------
add_files -fileset constrs_1 [file join $HW constraints constraints.xdc]
set_property top fpga_top [current_fileset]
update_compile_order -fileset sources_1

puts "ALPANA_SOC project ready: $PROJ/ALPANA_SOC.xpr"
puts "Next: launch_runs impl_1 -to_step write_bitstream -jobs 8"
