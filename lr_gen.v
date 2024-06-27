`default_nettype none

`define SH 0 // Waiting
`define SI 1 // Initial
`define SLX 2
`define SRX 3

// TODO(ruan): Add check and state for FULL FIFO
// TODO(ruan): Boundries

module lr_gen(
			  input wire		 clk,
			  input wire		 start,
			  input wire [63:0]	 v1, // Two 16 bits floats
			  input wire [63:0]	 v2,
			  input wire [63:0]	 v3,
			  output wire		 done,
			  output wire		 fifo_write,
			  output wire [31:0] p,

			  // Temporary outputs for debug
			  output reg [3:0]	 state
			  );

   wire signed [31:0]			 minxf, minyf, maxxf, maxyf;
   wire signed [31:0]			 minx, miny, maxx, maxy;
   wire signed [31:0]			 dx21, dx32, dx13, dy21, dy32, dy13;
   wire signed [31:0]			 fdx21, fdx32, fdx13, fdy21, fdy32, fdy13;
   wire signed					 order_expr;
   wire signed [31:0]			 eq1init, eq2init, eq3init;
   wire signed [31:0]			 eq1xinc, eq2xinc, eq3xinc;
   wire signed [31:0]			 eq1yinc, eq2yinc, eq3yinc;
   wire signed [31:0]			 x1w, x2w, x3w, y1w, y2w, y3w;
   wire							 s;
   wire							 point_inside;

   reg signed [31:0]			 x1, x2, x3;
   reg signed [31:0]			 y1, y2, y3;
   reg signed [31:0]			 next_x1, next_x2, next_x3;
   reg signed [31:0]			 next_y1, next_y2, next_y3;
   reg signed [31:0]			 eq1y, eq2y, eq3y, eq1x, eq2x, eq3x;
   reg signed [31:0]			 next_eq1y, next_eq2y, next_eq3y, next_eq1x, next_eq2x, next_eq3x;
   reg signed [15:0]			 lastx;
   reg signed [15:0]			 next_lastx;
   reg signed [31:0]			 pr;
   reg signed [31:0]			 next_pr;
   reg							 doner, fifo_writer;
   reg signed [31:0]			 x, y;
   reg signed [31:0]			 next_x, next_y;
   reg [3:0]					 next_state;
   reg							 next_doner, next_fifo_writer;

   initial begin
      state = `SH;
      doner = 1;
      fifo_writer = 0;
   end

   assign p = pr;
   assign done = doner;
   assign fifo_write = fifo_writer;

   assign x1w = v1[31:0];
   assign x2w = v2[31:0];
   assign x3w = v3[31:0];

   assign y1w = v1[63:32];
   assign y2w = v2[63:32];
   assign y3w = v3[63:32];

   pos_edge_det p0(
				   .sig(start),
				   .clk(clk),
				   .pe(s)
				   );

   assign minxf = (x1 < x2 ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3));
   assign minx = (minxf + 32'h0F) >> 4;
   assign minyf = (y1 < y2 ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3));
   assign miny = (minyf + 32'h0F) >> 4;

   assign maxxf = (x1 > x2 ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3));
   assign maxx = (maxxf + 32'h0F) >> 4;
   assign maxyf = (y1 > y2 ? (y1 > y3 ? y1 : y3) : (y2 > y3 ? y2 : y3));
   assign maxy = (maxyf + 32'h0F) >> 4;

   assign dx21 = x2 - x1;
   assign dx32 = x3 - x2;
   assign dx13 = x1 - x3;

   assign dy21 = y2 - y1;
   assign dy32 = y3 - y2;
   assign dy13 = y1 - y3;

   assign fdx21 = dx21 << 4;
   assign fdx32 = dx32 << 4;
   assign fdx13 = dx13 << 4;

   assign fdy21 = dy21 << 4;
   assign fdy32 = dy32 << 4;
   assign fdy13 = dy13 << 4;

   assign eq1xinc = eq1x - fdy21;
   assign eq2xinc = eq2x - fdy32;
   assign eq3xinc = eq3x - fdy13;

   assign eq1yinc = eq1y + fdx21;
   assign eq2yinc = eq2y + fdx32;
   assign eq3yinc = eq3y + fdx13;

   assign order_expr = (y2 - y1) * (x3 - x2) - (x2 - x1) * (y3 - y2) > 0;
   assign eq1init = dx21*((miny << 4) - y1) - dy21 * ((minx << 4) - x1);
   assign eq2init = dx32*((miny << 4) - y2) - dy32 * ((minx << 4) - x2);
   assign eq3init = dx13*((miny << 4) - y3) - dy13 * ((minx << 4) - x3);

   assign point_inside = (eq1x > 0 && eq2x > 0 && eq3x > 0);

   // State related logic
   always @(*)  begin
	  if      (state == `SI) next_state = `SLX;
	  else if (state == `SLX) begin
		 if      (y >= maxy)    next_state = `SH;
		 else if (point_inside) next_state = `SRX;
		 else                   next_state = `SLX;
	  end
	  else if (state == `SRX) begin
		 if (point_inside && (x < maxx)) next_state = `SRX;
		 else                            next_state = `SLX;
	  end
	  else next_state = state;
   end


   always @(*) begin
	  // Done related logic
	  if (state == `SLX && y >= maxy) next_doner = 1;
	  else if (state == `SH)               next_doner = 0;
	  else                            next_doner = 0;
	  // Fifo write related logic
	  if      (state == `SLX && point_inside)                next_fifo_writer = 1;
	  else if (state == `SRX && !(point_inside && x < maxx)) next_fifo_writer = 1;
	  else                                                   next_fifo_writer = 0;
   end 

   // xs and ys related logic
   always @(*) begin
	  next_x1 = x1w;
	  next_y1 = y1w;
	  if (order_expr) begin
		 next_x2 = x3w;
		 next_x3 = x2w;
		 next_y2 = y3w;
		 next_y3 = y2w;
	  end
	  else begin
		 next_x2 = x2w;
		 next_x3 = x3w;
		 next_y2 = y2w;
		 next_y3 = y3w;
	  end
   end

   // x, y related logic
   always @(*) begin
	  if (state == `SI) begin 
		 next_x = minx;
		 next_y = miny;

		 next_eq1y = eq1init;
		 next_eq2y = eq2init;
		 next_eq3y = eq3init;

		 next_eq1x = eq1init;
		 next_eq2x = eq2init;
		 next_eq3x = eq3init;
	  end
	  else if ((state == `SLX && (x >= maxx)) ||
			   (state == `SRX && !(point_inside && (x < maxx)))) begin
		 next_x = minx;
		 next_y = y + 1;

		 next_eq1y = eq1yinc;
		 next_eq2y = eq2yinc;
		 next_eq3y = eq3yinc;

		 next_eq1x = eq1yinc;
		 next_eq2x = eq2yinc;
		 next_eq3x = eq3yinc;
	  end
	  else begin
		 next_x = x + 1;
		 next_y = y;

		 next_eq1y = eq1y;
		 next_eq2y = eq2y;
		 next_eq3y = eq3y;

		 next_eq1x = eq1xinc;
		 next_eq2x = eq2xinc;
		 next_eq3x = eq3xinc;
	  end
   end

   always @(*) begin
	  if (point_inside) next_lastx = x[15:0];
	  else              next_lastx = lastx;
   end

   always @(*) begin
	  if      (state == `SLX && point_inside) next_pr = {y[15:0], x[15:0]};
	  else if (state == `SRX && !(point_inside && x < maxx)) next_pr = {y[15:0], lastx[15:0]};
	  else next_pr = pr;
   end

   always @(posedge clk) begin
	  state <= next_state;
	  doner <= next_doner;
	  fifo_writer <= next_fifo_writer;

	  x1 <= next_x1;
	  x2 <= next_x2;
	  x3 <= next_x3;
	  y1 <= next_y1;
	  y2 <= next_y2;
	  y3 <= next_y3;

	  x <= next_x;
	  y <= next_y;

	  eq1y <= next_eq1y;
	  eq2y <= next_eq2y;
	  eq3y <= next_eq3y;

	  eq1x <= next_eq1x;
	  eq2x <= next_eq2x;
	  eq3x <= next_eq3x;

	  lastx <= next_lastx;
	  pr <= next_pr;

      if (s) begin
		 state <= `SI;
		 doner <= 0;
		 fifo_writer <= 0;
      end
   end
endmodule
