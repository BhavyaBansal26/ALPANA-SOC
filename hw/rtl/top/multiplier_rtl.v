// ============================================================================
// multiplier_rtl.v : drop-in replacement for the Xilinx mult_gen IP "multiplier"
//   32x32 unsigned -> 64-bit product, latency = 2 clocks (= MULSTAGES in
//   mkmuldiv_e32.v, which samples P at rg_count == 2).
//   A/B are only valid in the get_inputs cycle, so they are registered on the
//   first edge; the product is registered on the second edge.
//   Plain RTL: Vivado maps it to DSP48E1s, other FPGAs/ASIC tools to their own
//   multipliers. Remove the "multiplier" IP from the project when using this.
// ============================================================================
module multiplier (
    input             CLK,
    input      [31:0] A,
    input      [31:0] B,
    output reg [63:0] P
);
    reg [31:0] a_r, b_r;
    always @(posedge CLK) begin
        a_r <= A;          // stage 1: capture operands
        b_r <= B;
        P   <= a_r * b_r;  // stage 2: product
    end
endmodule
