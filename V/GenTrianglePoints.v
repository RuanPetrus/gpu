`default_nettype none

`define S_IDLE 0
`define S_START 1
`define S_ORDER 2
`define S_INIT_EQ 3
`define S_FIND_POINT 4

`define MAX(a, b) ((a) > (b) ? (a) : (b))
`define MIN(a, b) ((a) < (b) ? (a) : (b))

module GenTrianglePoints(
						 i_clk, i_start, i_v1, i_v2, i_v3,
						 o_write, o_done, o_point,
						 state, x, y
);
   parameter SCREEN_WIDTH = 640;
   parameter SCREEN_HEIGHT = 480;

   input		 i_clk, i_start;
   input [63:0]	 i_v1, i_v2, i_v3; // {y, x} fixed_point, 28:4 bits each
   output reg	 o_write, o_done;
   output reg [31:0] o_point;
   // {y, x} int 16 bits each

   reg signed [31:0]				  x1, x2, x3;
   reg signed [31:0]				  y1, y2, y3;
   reg signed [31:0]				  next_x1, next_x2, next_x3;
   reg signed [31:0]				  next_y1, next_y2, next_y3;
   wire								  order_expr;

   output reg [3:0]						  state;
   reg [3:0]						  next_state;

   wire								  point_inside;
   reg								  next_o_write;
   reg								  next_o_done;
   reg [31:0]						  next_o_point;

   wire signed [31:0]				  minxf, minyf, maxxf, maxyf;
   wire signed [31:0]				  minx, miny, maxx, maxy;
   wire signed [31:0]				  dx21, dx32, dx13, dy21, dy32, dy13;
   wire signed [31:0]				  fdx21, fdx32, fdx13, fdy21, fdy32, fdy13;
   wire signed [31:0]				  eq1init, eq2init, eq3init;
   wire signed [31:0]				  eq1xinc, eq2xinc, eq3xinc;
   wire signed [31:0]				  eq1yinc, eq2yinc, eq3yinc;

   reg signed [31:0]				  eq1y, eq2y, eq3y, eq1x, eq2x, eq3x;
   reg signed [31:0]				  next_eq1y, next_eq2y, next_eq3y, next_eq1x, next_eq2x, next_eq3x;

   output reg signed [31:0]				  x, y;
   reg signed [31:0]				  next_x, next_y;

   always @(posedge i_clk) begin
	  state <= next_state;
	  o_done <= next_o_done;
	  o_write <= next_o_write;

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

	  o_point <= next_o_point;
	  
      if (i_start && state == `S_IDLE) begin
		 state <= `S_START;
		 o_done <= 0;
		 o_write <= 0;
		 x1 <= i_v1[31:0];
		 x2 <= i_v2[31:0];
		 x3 <= i_v3[31:0];
		 y1 <= i_v1[63:32];
		 y2 <= i_v2[63:32];
		 y3 <= i_v3[63:32];
      end
   end

   // State related logic
   initial state = `S_IDLE;
   always @(*)  begin
	  if      (state == `S_START)   next_state = `S_ORDER;
	  else if (state == `S_ORDER)   next_state = `S_INIT_EQ;
	  else if (state == `S_INIT_EQ) next_state = `S_FIND_POINT;
	  else if (state == `S_FIND_POINT) begin
		 if (x +1 == maxx && y + 1 == maxy) next_state = `S_IDLE;
		 else                               next_state = `S_FIND_POINT;
	  end
	  else next_state = state;
   end

   // Write point logic
   initial o_done = 1;
   initial o_write = 0;

   assign point_inside = (eq1x > 0 && eq2x > 0 && eq3x > 0);

   always @(*) begin
	  // Done
	  if (state == `S_IDLE || (state == `S_FIND_POINT && x +1 == maxx && y + 1 == maxy))
		next_o_done = 1;
	  else next_o_done = 0;

	  // Write
	  if (state == `S_FIND_POINT && point_inside) next_o_write = 1;
	  else next_o_write = 0;

	  // Point
	  if (state == `S_FIND_POINT && point_inside) next_o_point = {y[15:0], x[15:0]};
	  else next_o_point = o_point;
   end 

   // x, y related logic
   assign minxf = `MIN(`MIN(x1, x2), x3);
   assign minx = `MAX((minxf + 32'sh0F) >>> 4, 0);

   assign minyf = `MIN(`MIN(y1, y2), y3);
   assign miny = `MAX((minyf + 32'sh0F) >>> 4, 0);

   assign maxxf = `MAX(`MAX(x1, x2), x3);
   assign maxx = `MIN((maxxf + 32'sh0F) >>> 4, SCREEN_WIDTH);
   
   assign maxyf = `MAX(`MAX(y1, y2), y3);
   assign maxy = `MIN((maxyf + 32'sh0F) >>> 4, SCREEN_HEIGHT);

   assign dx21 = x2 - x1;
   assign dx32 = x3 - x2;
   assign dx13 = x1 - x3;

   assign dy21 = y2 - y1;
   assign dy32 = y3 - y2;
   assign dy13 = y1 - y3;

   assign fdx21 = dx21 <<< 4;
   assign fdx32 = dx32 <<< 4;
   assign fdx13 = dx13 <<< 4;

   assign fdy21 = dy21 <<< 4;
   assign fdy32 = dy32 <<< 4;
   assign fdy13 = dy13 <<< 4;

   assign eq1xinc = eq1x - fdy21;
   assign eq2xinc = eq2x - fdy32;
   assign eq3xinc = eq3x - fdy13;

   assign eq1yinc = eq1y + fdx21;
   assign eq2yinc = eq2y + fdx32;
   assign eq3yinc = eq3y + fdx13;

   assign eq1init = dx21*((miny <<< 4) - y1) - dy21 * ((minx <<< 4) - x1);
   assign eq2init = dx32*((miny <<< 4) - y2) - dy32 * ((minx <<< 4) - x2);
   assign eq3init = dx13*((miny <<< 4) - y3) - dy13 * ((minx <<< 4) - x3);

   always @(*) begin
	  if (state == `S_INIT_EQ) begin 
		 next_x = minx;
		 next_y = miny;

		 next_eq1y = eq1init;
		 next_eq2y = eq2init;
		 next_eq3y = eq3init;

		 next_eq1x = eq1init;
		 next_eq2x = eq2init;
		 next_eq3x = eq3init;
	  end
	  else if (state == `S_FIND_POINT && x + 1 == maxx) begin
		 next_x = minx;
		 next_y = y + 1;

		 next_eq1y = eq1yinc;
		 next_eq2y = eq2yinc;
		 next_eq3y = eq3yinc;

		 next_eq1x = eq1yinc;
		 next_eq2x = eq2yinc;
		 next_eq3x = eq3yinc;
	  end
	  else if (state == `S_FIND_POINT) begin
		 next_x = x + 1;
		 next_y = y;

		 next_eq1y = eq1y;
		 next_eq2y = eq2y;
		 next_eq3y = eq3y;

		 next_eq1x = eq1xinc;
		 next_eq2x = eq2xinc;
		 next_eq3x = eq3xinc;
	  end
	  else begin
		 next_x = x;
		 next_y = y;

		 next_eq1y = eq1y;
		 next_eq2y = eq2y;
		 next_eq3y = eq3y;

		 next_eq1x = eq1x;
		 next_eq2x = eq2x;
		 next_eq3x = eq3x;
	  end
   end

   // xs and ys related logic ordering
   assign order_expr = (y2 - y1) * (x3 - x2) - (x2 - x1) * (y3 - y2) > 0;

   always @(*) begin
	  next_x1 = x1;
	  next_y1 = y1;
	  if (state == `S_ORDER && order_expr) begin
		 next_x2 = x3;
		 next_x3 = x2;
		 next_y2 = y3;
		 next_y3 = y2;
	  end
	  else begin
		 next_x2 = x2;
		 next_x3 = x3;
		 next_y2 = y2;
		 next_y3 = y3;
	  end
   end

endmodule
