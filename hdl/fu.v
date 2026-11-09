// functional units: 8-bit, results wrap modulo 256
module adder (input [7:0] a, b, output [7:0] y);
    assign y = a + b;
endmodule

module subtractor (input [7:0] a, b, output [7:0] y);
    assign y = a - b;
endmodule

module multiplier (input [7:0] a, b, output [7:0] y);
    assign y = a * b;
endmodule
