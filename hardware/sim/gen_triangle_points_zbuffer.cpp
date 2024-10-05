#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "VGenTrianglePointsZbuffer.h"
#include "verilated.h"

const int SIMULATION_STEP = 1700;
const int START_SIGNAL_TIME = 4;


void tick(VGenTrianglePointsZbuffer *tb) {
	tb->eval();
	tb->i_clk = 1;
	tb->eval();
	tb->i_clk = 0;
	tb->eval();
}

std::vector<uint32_t> circuit_points(
									 int argc, char** argv,
									 int32_t x1, int32_t x2, int32_t x3,
									 int32_t y1, int32_t y2, int32_t y3) {
	Verilated::commandArgs(argc, argv);
	VGenTrianglePointsZbuffer *tb = new VGenTrianglePointsZbuffer;

	uint64_t v1 = ((uint64_t) ((uint32_t) y1) << 32) | ((uint32_t) x1);
	uint64_t v2 = ((uint64_t) ((uint32_t) y2) << 32) | ((uint32_t) x2);
	uint64_t v3 = ((uint64_t) ((uint32_t) y3) << 32) | ((uint32_t) x3);

	fprintf(stderr, "v1: %lx\n", v1);
	fprintf(stderr, "v2: %lx\n", v2);
	fprintf(stderr, "v3: %lx\n", v3);

	tb->i_v1 = v1;
	tb->i_v2 = v2;
	tb->i_v3 = v3;

	std::vector<uint32_t> points;

	bool start = false;
	bool finished = false;
	for (int k = 0; !finished; k++) {

		if (START_SIGNAL_TIME == k) tb->i_start = 1, start = true;
		else                        tb->i_start = 0;

		tick(tb);

		fprintf(stderr, "-------- Step %d -----\n", k);
		fprintf(stderr, "Done: %d\n",       tb->o_done);
		fprintf(stderr, "Fifo Write: %d\n", tb->o_write);
		fprintf(stderr, "Fifo point x: %d\n", (tb->o_point & 0xFFFF));
		fprintf(stderr, "Fifo point y: %d\n", (tb->o_point >> 16));

		// fprintf(stderr, "-------- Debug outputs -------\n");
		// fprintf(stderr, "state: %d\n",       tb->state);
		// fprintf(stderr, "x: %d\n",       (int32_t) tb->x);
		// fprintf(stderr, "y: %d\n",       (int32_t) tb->y);

		fprintf(stderr, "\n");

		if (tb->o_write) points.emplace_back(tb->o_point);
		if (start && tb->o_done) finished = true;
	}
	delete tb;
return points;
}

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

std::vector<uint32_t> algorithm_points(int32_t x1, int32_t x2, int32_t x3,
									 int32_t y1, int32_t y2, int32_t y3) {
	// Ordering v1 v2 v3 in counter clockwise
	if ((y2 - y1) * (x3 - x2) - (x2 - x1) * (y3 - y2) > 0) {
		std::swap(x2, x3);
		std::swap(y2, y3);
	}

	// Step functions *16
	const int32_t dx21 = x2 - x1;
	const int32_t dx32 = x3 - x2;
	const int32_t dx13 = x1 - x3;

	const int32_t dy21 = y2 - y1;
	const int32_t dy32 = y3 - y2;
	const int32_t dy13 = y1 - y3;

	// Step functions * 16 * 16
	const int32_t fdx21 = dx21 << 4;
	const int32_t fdx32 = dx32 << 4;
	const int32_t fdx13 = dx13 << 4;

	const int32_t fdy21 = dy21 << 4;
	const int32_t fdy32 = dy32 << 4;
	const int32_t fdy13 = dy13 << 4;

	// minx, miny, maxx, maxy
	const int32_t minxf = std::min({x1, x2, x3});
	const int32_t minx = std::max((minxf + 0xF) >> 4, 0);

	const int32_t minyf = std::min({y1, y2, y3});
	const int32_t miny = std::max((minyf + 0xF) >> 4, 0);
	
	const int32_t maxxf = std::max({x1, x2, x3});
	const int32_t maxx = std::min((maxxf + 0xF) >> 4, WINDOW_WIDTH);

	const int32_t maxyf = std::max({y1, y2, y3});
	const int32_t maxy = std::min((maxyf + 0xF) >> 4, WINDOW_HEIGHT);

	// Equations * 16 * 16
	int32_t eq1y = dx21*((miny << 4) - y1) - dy21 * ((minx << 4) - x1);
	int32_t eq2y = dx32*((miny << 4) - y2) - dy32 * ((minx << 4) - x2);
	int32_t eq3y = dx13*((miny << 4) - y3) - dy13 * ((minx << 4) - x3);

	// Correct for fill convention
    // if(dy21 < 0 || (dy21 == 0 && dx21 > 0)) eq1y++;
    // if(dy32 < 0 || (dy32 == 0 && dx32 > 0)) eq2y++;
    // if(dy13 < 0 || (dy13 == 0 && dx13 > 0)) eq3y++;


	std::vector<uint32_t> points;

	for (int32_t y = miny; y < maxy; y++) {
		int32_t eq1x = eq1y;
		int32_t eq2x = eq2y;
		int32_t eq3x = eq3y;
		for (int32_t x = minx; x < maxx; x++) {
			if (eq1x > 0 &&
				eq2x > 0 &&
				eq3x > 0) {
				points.push_back((y << 16) | x);
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

int main(int argc, char **argv) {
	int32_t x1 = 1.0f/4.0f * WINDOW_WIDTH * 16.0f;
	int32_t x2 = 3.0f/4.0f * WINDOW_WIDTH * 16.0f;
	int32_t x3 = 2.0f/4.0f * WINDOW_WIDTH * 16.0f;

	int32_t y1 = 3.0f/4.0f * WINDOW_HEIGHT * 16.0f;
	int32_t y2 = 3.0f/4.0f * WINDOW_HEIGHT * 16.0f;
	int32_t y3 = 1.0f/4.0f * WINDOW_HEIGHT * 16.0f;

	std::vector<uint32_t> cp = circuit_points(argc, argv, x1, x2, x3,
											   y1, y2, y3);

	std::vector<uint32_t> ap = algorithm_points(x1, x2, x3,
												y1, y2, y3);

	fprintf(stderr, "Number of circuit_points: %lx\n", cp.size());
	fprintf(stderr, "Number of algo_points: %lx\n", ap.size());

	fprintf(stderr, "Last circuit x: %x\n", cp[cp.size()-1] & 0xFFFF);
	fprintf(stderr, "Last alog x: %x\n", ap[ap.size()-1] & 0xFFFF);

	// fprintf(stderr, "Circuit_Points:\n"); 
	// for (auto p: cp) fprintf(stderr, "%x ", p);
	// fprintf(stderr, "\n");
	// 
	// fprintf(stderr, "Algo_Points:\n"); 
	// for (auto p: ap) fprintf(stderr, "%x ", p);
	// fprintf(stderr, "\n");

	assert(cp.size() == ap.size());
	for (int i = 0; i < (int)ap.size(); i++) assert(ap[i] == cp[i]);
	fprintf(stdout, "TEST PASSED: All points are equal\n");

	return 0;
}
