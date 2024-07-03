module pos_edge_det(
	input sig, clk,
	output pe
);

reg sig_dly;
initial begin
   sig_dly = 0;
end

always@(posedge clk) begin
	sig_dly <= sig;
end

assign pe = sig & ~sig_dly;

endmodule
