`default_nettype	none

module Fifo(
			clock,
			data,
			rdreq,
			wrreq,
			empty,
			full,
			q
			);
   parameter BW = 32;     // Memory Width
   parameter LGFLEN = 10; // Memory size 2^10 (1024)
  
   input wire clock;
   input wire [(BW-1):0] data;
   input wire			 rdreq;
   input wire			 wrreq;

   output reg			 empty;
   output reg			 full;
   output reg [(BW-1):0] q;

   wire w_wr = wrreq && !full;
   wire	w_rd = rdreq && !empty;
   reg [(BW-1):0] fifo_mem [0:(1<<LGFLEN)-1];
   reg [LGFLEN:0] wr_addr, rd_addr, o_fill;

   initial wr_addr = 0;
   always @(posedge clock)
	 if (w_wr) begin
		wr_addr <= wr_addr + 1;
		fifo_mem[wr_addr[(LGFLEN-1):0]] <= data;
	 end


   initial rd_addr = 0;
   always @(posedge clock)
	 if (w_rd) begin
		rd_addr <= rd_addr + 1;
		q <= fifo_mem[rd_addr[(LGFLEN-1):0]];
	 end

   always @(*)
	 o_fill = wr_addr - rd_addr;

   always @(*)
	 empty = (o_fill == 0);
   
   always @(*)
	 full = (o_fill == {1'b1, {(LGFLEN){1'b0}}});
   
`ifdef FORMAL
   always @(*)
	 assert(o_fill <= {1'b1, {(LGFLEN){1'b0}}});
	   
   always @(*)
	 assert ( o_fill == wr_addr - rd_addr ) ;
   always @(*)
	 assert ( empty == ( o_fill == 0 ) ) ;
   always @(*)
	 assert (full == ( o_fill == { 1'b1 ,{( LGFLEN ) { 1'b0 }}}));

`endif // FORMAL

endmodule
