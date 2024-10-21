#include <stdio.h>
#include "VGenTrianglePointsZbuffer.h"
#include "verilated.h"

const int SIMULATION_STEP = 1700;
const int START_SIGNAL_TIME = 4;

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

void tick(VGenTrianglePointsZbuffer *tb) {
	tb->eval();
	tb->i_clk = 1;
	tb->eval();
	tb->i_clk = 0;
	tb->eval();
}

void circuit_draw(
				  int argc, char** argv,
				  int32_t x1, int32_t x2, int32_t x3,
				  int32_t y1, int32_t y2, int32_t y3,
				  int32_t iz1, int32_t iz2, int32_t iz3,
				  uint32_t *canvas, int32_t *zbuffer, uint32_t color) {
	Verilated::commandArgs(argc, argv);
	VGenTrianglePointsZbuffer *tb = new VGenTrianglePointsZbuffer;

	uint64_t v1 = ((uint64_t) ((uint32_t) y1) << 32) | ((uint32_t) x1);
	uint64_t v2 = ((uint64_t) ((uint32_t) y2) << 32) | ((uint32_t) x2);
	uint64_t v3 = ((uint64_t) ((uint32_t) y3) << 32) | ((uint32_t) x3);
	tb->i_v1 = v1;
	tb->i_v2 = v2;
	tb->i_v3 = v3;
	tb->i_iz1 = iz1;
	tb->i_iz2 = iz2;
	tb->i_iz3 = iz3;

	bool start = false;
	bool finished = false;
	int32_t count = 0;
	for (int k = 0; !finished; k++) {
		if (START_SIGNAL_TIME == k) tb->i_start = 1, start = true;
		else                        tb->i_start = 0;

		tb->i_zbuffer_data = zbuffer[tb->o_zbuffer_addr];
		fprintf(stderr, "state: %d\n",       tb->state);
		fprintf(stderr, "o_zbuffer_addr: %d\n",       tb->o_zbuffer_addr);
		tick(tb);

		// fprintf(stderr, "-------- Step %d -----\n", k);
		// fprintf(stderr, "Done: %d\n",       tb->o_done);
		// fprintf(stderr, "Fifo Write: %d\n", tb->o_write);
		// fprintf(stderr, "-------- Debug outputs -------\n");
		// fprintf(stderr, "state: %d\n",       tb->state);
		// fprintf(stderr, "x: %d\n",       (int32_t) tb->x);
		// fprintf(stderr, "y: %d\n",       (int32_t) tb->y);

		// fprintf(stderr, "\n");

		if (tb->o_write) {
			int y = (tb->o_point >> 16) & 0xFFFF;
			int x = (tb->o_point >> 0)  & 0xFFFF;
			zbuffer[y*WINDOW_WIDTH + x] = tb->o_zbuffer_data;
			canvas[y*WINDOW_WIDTH + x] = color;
		}
		if (start && tb->o_done) finished = true;
		count += start;
	}
	printf("Circuit finished drawing");
	printf("Circuit took %d cycles\n", count);
	delete tb;
}

typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;
typedef int64_t   i64;
typedef int32_t   i32;
typedef int16_t   i16;
typedef int8_t    i8;

typedef float f32;
typedef long double f64;

typedef u32* canvas_ptr;
typedef u32 PixelType;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define SWAP(T, a, b) do { T temp = a; a = b; b = temp; } while(0)
#define max3(a, b, c) max(a, max(b, c))
#define min3(a, b, c) min(a, min(b, c))
#define PI 3.14159265358979323846f

#define WINDOW_WIDTH 320
#define WINDOW_STRIDE 320
#define WINDOW_HEIGHT 240

#define GREEN 0x00FF00FF
#define RED 0xFF0000FF
#define BLUE 0x0000FFFF
#define YELLOW 0xFFFF00FF
#define PURPLE 0xFF00FFFF
#define CYAN 0x00FFFFFF

#define WHITE 0xFFFFFFFF

#define PLAYER_SPEED 0.7f
#define ROTATION_SPEED 1.5f
#define DELTA 0.00001f
#define Z_FRACTION_BITS 16
#define NEAR_PLANE 0.20f
#define FAR_PLANE 16.0f
#define INV_NEAR_PLANE ((1 << Z_FRACTION_BITS) * (1.0f/NEAR_PLANE))
#define INV_FAR_PLANE ((1 << Z_FRACTION_BITS) * (1.0f/FAR_PLANE))

typedef struct Vec2 Vec2;
struct Vec2 {
	f32 x, y;
};

typedef struct Vec3 Vec3;
struct Vec3 {
	f32 x, y, z;
};

i32 shift_right_round(i64 a, i32 b) 
{
	return (a + ((1ll << b) -1)) >> b;
}

void draw_triangle(Vec2 v1, Vec2 v2, Vec2 v3, 
				   f32 fz1, f32 fz2, f32 fz3,
				   PixelType color,
				   i32 *zbuffer,
                   canvas_ptr canvas)
{
	// Ordering v1 v2 v3 in counter clockwise
	if ((v2.y - v1.y) * (v3.x - v2.x) - (v2.x - v1.x) * (v3.y - v2.y) > 0) {
		SWAP(Vec2, v2, v3); 
		SWAP(f32, fz2, fz3);
	}

	// Xs and Ys * 16
	i32 y1 = v1.y * 16.0f;
	i32 y2 = v2.y * 16.0f;
	i32 y3 = v3.y * 16.0f;

	i32 x1 = v1.x * 16.0f;
	i32 x2 = v2.x * 16.0f;
	i32 x3 = v3.x * 16.0f;

	i32 z1 = fz1 * (1 << Z_FRACTION_BITS);
	i32 z2 = fz2 * (1 << Z_FRACTION_BITS);
	i32 z3 = fz3 * (1 << Z_FRACTION_BITS);

	if ((z1 <= 0 || z1 > INV_NEAR_PLANE) && 
	 	(z2 <= 0 || z2 > INV_NEAR_PLANE) && 
	 	(z3 <= 0 || z3 > INV_NEAR_PLANE)) return;

	// Step functions *16
	i32 dx21 = x2 - x1;
	i32 dx32 = x3 - x2;
	i32 dx13 = x1 - x3;
	i32 dx31 = x3 - x1;

	i32 dy21 = y2 - y1;
	i32 dy32 = y3 - y2;
	i32 dy13 = y1 - y3;
	i32 dy31 = y3 - y1;

	i32 dz13 = z1 - z3;
	i32 dz23 = z2 - z3;

	// Step functions * 16 * 16
	i32 fdx21 = dx21 << 4;
	i32 fdx32 = dx32 << 4;
	i32 fdx13 = dx13 << 4;

	i32 fdy21 = dy21 << 4;
	i32 fdy32 = dy32 << 4;
	i32 fdy13 = dy13 << 4;

	// minx, miny, maxx, maxy
	i32 minxf = min3(x1, x2, x3);
	i32 minx =  max((minxf + 0xF) >> 4, 0);

	i32 minyf = min3(y1, y2, y3);
	i32 miny =  max((minyf + 0xF) >> 4, 0);
	
	i32 maxxf = max3(x1, x2, x3);
	i32 maxx =  min((maxxf + 0xF) >> 4, WINDOW_WIDTH);

	i32 maxyf = max3(y1, y2, y3);
	i32 maxy = min((maxyf + 0xF) >> 4, WINDOW_HEIGHT);

	canvas_ptr buffer = canvas + miny * WINDOW_STRIDE;
	i32 *zbufy = zbuffer + miny * WINDOW_WIDTH;

	// Equations 24.8
	i32 eq1y = dx21*((miny << 4) - y1) - dy21 * ((minx << 4) - x1);
	i32 eq2y = dx32*((miny << 4) - y2) - dy32 * ((minx << 4) - x2);
	i32 eq3y = dx13*((miny << 4) - y3) - dy13 * ((minx << 4) - x3);

	i32 area = dx21*dy31 - dy21 * dx31; // 24.8
	if (area == 0) area = 1;

	i64 fdzxt =  ((-(i64)dz13*dy32 - (i64)dz23*dy13) << 16)/area;
	i64 fdzyt =  (( (i64)dz13*dx32 + (i64)dz23*dx13) << 16)/area;

	i32 fdzx = shift_right_round(fdzxt, 12);
	i32 fdzy = shift_right_round(fdzyt, 12);

	i64 zd1 = (((i64)eq2y*dz13 + (i64)eq3y*dz23) << 16)/area;
	i32 eqz = z3 + shift_right_round(zd1, 16);

	// Correct for fill convention
    // if(dy21 < 0 || (dy21 == 0 && dx21 > 0)) eq1y++;
    // if(dy32 < 0 || (dy32 == 0 && dx32 > 0)) eq2y++;
    // if(dy13 < 0 || (dy13 == 0 && dx13 > 0)) eq3y++;

	for (i32 y = miny; y < maxy; y++) {
		canvas_ptr xbuffer = buffer + minx;
		i32 *zb =      zbufy + minx;
		i32 eq1x = eq1y;
		i32 eq2x = eq2y;
		i32 eq3x = eq3y;
		i32 eqzx = eqz;
		for (i32 x = minx; x < maxx; x++) {
			if (eq1x > 0 &&
				eq2x > 0 &&
				eq3x > 0) {
				 if (*zb < eqzx && eqzx < INV_NEAR_PLANE) {
					*zb = eqzx;
					*xbuffer = color;
				}
			}
			xbuffer++;
			zb++;
			eq1x -= fdy21;
			eq2x -= fdy32;
			eq3x -= fdy13;
			eqzx += fdzx;
		}
		buffer += WINDOW_STRIDE;
		zbufy += WINDOW_WIDTH;
		eq1y += fdx21;
		eq2y += fdx32;
		eq3y += fdx13;
		eqz += fdzy;
	}
}

uint32_t canvas1[WINDOW_WIDTH*WINDOW_HEIGHT], canvas2[WINDOW_WIDTH*WINDOW_HEIGHT];

int32_t zb1[WINDOW_WIDTH*WINDOW_HEIGHT], zb2[WINDOW_WIDTH*WINDOW_HEIGHT];

int main(int argc, char **argv) {
	f32 fx1 = 1.0f/4.0f * WINDOW_WIDTH;
	f32 fx2 = 3.0f/4.0f * WINDOW_WIDTH;
	f32 fx3 = 2.0f/4.0f * WINDOW_WIDTH;

	int32_t x1 =  fx1 * 16.0f;
	int32_t x2 =  fx2 * 16.0f;
	int32_t x3 =  fx3 * 16.0f;

	f32 fy1 = 3.0f/4.0f * WINDOW_HEIGHT;
	f32 fy2 = 3.0f/4.0f * WINDOW_HEIGHT;
	f32 fy3 = 1.0f/4.0f * WINDOW_HEIGHT;

	int32_t y1 = fy1 * 16.0f;
	int32_t y2 = fy2 * 16.0f;
	int32_t y3 =  fy3 * 16.0f;

	f32 fz1 = 2.0f;
	f32 fz2 = 2.0f;
	f32 fz3 = 2.0f;

	int32_t iz1 = 1/fz1 * (1 << 16);
	int32_t iz2 = 1/fz2 * (1 << 16);
	int32_t iz3 = 1/fz3 * (1 << 16);

	for (int i = 0; i < WINDOW_HEIGHT; i++) 
		for (int j = 0; j < WINDOW_WIDTH; j++) {
			canvas1[i*WINDOW_STRIDE + j] = WHITE;
			canvas2[i*WINDOW_STRIDE + j] = WHITE;
		}
	for (int i= 0; i < WINDOW_WIDTH*WINDOW_HEIGHT; i++) {
		zb1[i] = INV_FAR_PLANE;
		zb2[i] = INV_FAR_PLANE;
	}

	circuit_draw(argc, argv,
				   x1, x2, x3,
				   y1, y2, y3,
				   iz1, iz2, iz3,
				   canvas1, zb1, GREEN
				 );

	draw_triangle(
				  {fx1, fy1}, {fx2, fy2}, {fx3, fy3}, 
				  1/fz1, 1/fz2, 1/fz3,
				  GREEN,
				  zb2,
				  canvas2);

	for (int i = 0; i < WINDOW_WIDTH*WINDOW_HEIGHT; i++) {
		assert(canvas1[i] == canvas2[i]);
		assert(zb1[i] == zb2[i]);
	}

	return 0;
}
