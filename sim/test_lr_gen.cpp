#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <vector>
#include <algorithm>
using namespace std;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;
typedef int32_t   i32;
typedef int16_t   i16;
typedef int8_t    i8;

struct Vec2 {
	i32 x, y;
};

typedef int32_t i32;

vector<uint32_t> draw_triangle(Vec2 v1, Vec2 v2, Vec2 v3)
{
	// Ordering v1 v2 v3 in counter clockwise
	if ((v2.y - v1.y) * (v3.x - v2.x) - (v2.x - v1.x) * (v3.y - v2.y) > 0) std::swap(v2, v3);

	// Xs and Ys * 16
	// const i32 y1 = std::round(v1.y * 16.0f);
	// const i32 y2 = std::round(v2.y * 16.0f);
	// const i32 y3 = std::round(v3.y * 16.0f);

	// const i32 x1 = std::round(v1.x * 16.0f);
	// const i32 x2 = std::round(v2.x * 16.0f);
	// const i32 x3 = std::round(v3.x * 16.0f);

	const i32 y1 = v1.y;
	const i32 y2 = v2.y;
	const i32 y3 = v3.y;

	const i32 x1 = v1.x;
	const i32 x2 = v2.x;
	const i32 x3 = v3.x;

	// Step functions *16
	const i32 dx21 = x2 - x1;
	const i32 dx32 = x3 - x2;
	const i32 dx13 = x1 - x3;

	const i32 dy21 = y2 - y1;
	const i32 dy32 = y3 - y2;
	const i32 dy13 = y1 - y3;

	// Step functions * 16 * 16
	const i32 fdx21 = dx21 << 4;
	const i32 fdx32 = dx32 << 4;
	const i32 fdx13 = dx13 << 4;

	const i32 fdy21 = dy21 << 4;
	const i32 fdy32 = dy32 << 4;
	const i32 fdy13 = dy13 << 4;

	// minx, miny, maxx, maxy
	const i32 minxf = std::min({x1, x2, x3});
	const i32 minx = std::max((minxf + 0xF) >> 4, 0);

	const i32 minyf = std::min({y1, y2, y3});
	const i32 miny = std::max((minyf + 0xF) >> 4, 0);
	
	const i32 maxxf = std::max({x1, x2, x3});
	const i32 maxx = std::min((maxxf + 0xF) >> 4, WINDOW_WIDTH);

	const i32 maxyf = std::max({y1, y2, y3});
	const i32 maxy = std::min((maxyf + 0xF) >> 4, WINDOW_HEIGHT);

	// Equations * 16 * 16
	i32 eq1y = dx21*((miny << 4) - y1) - dy21 * ((minx << 4) - x1);
	i32 eq2y = dx32*((miny << 4) - y2) - dy32 * ((minx << 4) - x2);
	i32 eq3y = dx13*((miny << 4) - y3) - dy13 * ((minx << 4) - x3);

	// Correct for fill convention
    // if(dy21 < 0 || (dy21 == 0 && dx21 > 0)) eq1y++;
    // if(dy32 < 0 || (dy32 == 0 && dx32 > 0)) eq2y++;
    // if(dy13 < 0 || (dy13 == 0 && dx13 > 0)) eq3y++;

	vector<uint32_t> points;

	int state = 0;
	i32 lastx = 0;
	for (i32 y = miny; y < maxy; y++) {
		i32 eq1x = eq1y;
		i32 eq2x = eq2y;
		i32 eq3x = eq3y;
		state = 0;
		for (i32 x = minx; x < maxx; x++) {
			if (eq1x > 0 &&
				eq2x > 0 &&
				eq3x > 0) {
				if (state == 0) {
					points.emplace_back(((y << 16) | x));
					lastx = x;
					state = 1;
				}
				else if (state ==1 )lastx = x;
			}
			else if (state == 1) {
				points.emplace_back(((y << 16) | lastx));
				state = 2;
			}
			eq1x -= fdy21;
			eq2x -= fdy32;
			eq3x -= fdy13;
		}
		eq1y += fdx21;
		eq2y += fdx32;
		eq3y += fdx13;
	}
	return points;
}

int main() {
	int32_t x1 = 10.0f * 16.0f;
	int32_t x2 = 50.0f * 16.0f;
	int32_t x3 = 10.0f * 16.0f;

	int32_t y1 = 10.0f * 16.0f;
	int32_t y2 = 10.0f * 16.0f;
	int32_t y3 = 50.0f * 16.0f;

	vector<uint32_t> points = draw_triangle({x1, y1}, {x2, y2}, {x3, y3});

	for (auto p: points) printf("%u ", p);
	printf("\n");

	return 0;
}
