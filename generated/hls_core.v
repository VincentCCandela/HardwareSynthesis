// hls_core.v - generated from the asap schedule by ./main --verilog (8-bit datapath)
// fu_sel: 0=add0 1=sub0 2=mul0
module hls_core (
    input clk, rst,
    input step,           // advance one control step
    input ld,             // load ld_val into input ld_sel (inputs 0-3)
    input [1:0] ld_sel,
    input [7:0] ld_val,
    input [4:0] fu_sel,   // functional unit shown on fu_out
    output reg [4:0] cyc, // control step, 3 = done
    output reg [7:0] fu_out
);
    reg [7:0] in0 = 8'd2; // A0
    reg [7:0] in1 = 8'd3; // B0
    reg [7:0] r0; // register 0: X0 -> Y0
    reg [7:0] r1; // register 1: X0 -> Z0
    reg [7:0] r2; // register 2: Z0 -> Q0
    reg [7:0] add0_a, add0_b, sub0_a, sub0_b, mul0_a, mul0_b;
    wire [7:0] add0_y;
    adder u_add0 (add0_a, add0_b, add0_y);
    wire [7:0] sub0_y;
    subtractor u_sub0 (sub0_a, sub0_b, sub0_y);
    wire [7:0] mul0_y;
    multiplier u_mul0 (mul0_a, mul0_b, mul0_y);

    // multiplexers: the operands of every unit in the current control step
    always @* begin
        {add0_a, add0_b, sub0_a, sub0_b, mul0_a, mul0_b} = 0;
        case (cyc)
            0: begin
                add0_a = in0; add0_b = in1; // X0 = A0 + B0
            end
            1: begin
                add0_a = r0; add0_b = in0; // Y0 = X0 + A0
                sub0_a = r1; sub0_b = in1; // Z0 = X0 - B0
            end
            2: begin
                mul0_a = in1; mul0_b = r2; // Q0 = B0 * Z0
            end
        endcase
    end

    always @*
        case (fu_sel)
            0: fu_out = add0_y;
            1: fu_out = sub0_y;
            2: fu_out = mul0_y;
            default: fu_out = 0;
        endcase

    // sequencer: each step ends a control step and loads the registers enabled then
    always @(posedge clk)
        if (rst) begin
            cyc <= 0;
            in0 <= 8'd2;
            in1 <= 8'd3;
        end else begin
            if (ld)
                case (ld_sel)
                    0: in0 <= ld_val;
                    1: in1 <= ld_val;
                endcase
            if (step && cyc < 3) begin
                cyc <= cyc + 1;
                case (cyc)
                    0: begin r0 <= add0_y; r1 <= add0_y; end
                    1: begin r2 <= sub0_y; end
                endcase
            end
        end
endmodule
