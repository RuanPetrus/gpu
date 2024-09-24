#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "VGenTriangleBoundries.h"
#include "verilated.h"

const int SIMULATION_STEP = 1000;
const int START_SIGNAL_TIME = 4;

void tick(VGenTriangleBoundries *tb) {
	tb->eval();
	tb->clk = 1;
	tb->eval();
	tb->clk = 0;
	tb->eval();
}

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	VGenTriangleBoundries *tb = new VGenTriangleBoundries;

	int32_t x1 = 10.0f * 16.0f;
	int32_t x2 = 50.0f * 16.0f;
	int32_t x3 = 10.0f * 16.0f;

	int32_t y1 = 10.0f * 16.0f;
	int32_t y2 = 10.0f * 16.0f;
	int32_t y3 = 50.0f * 16.0f;

	uint64_t v1 = ((uint64_t) y1 << 32) | x1;
	uint64_t v2 = ((uint64_t) y2 << 32) | x2;
	uint64_t v3 = ((uint64_t) y3 << 32) | x3;

	fprintf(stderr, "v1: %lx\n", v1);
	fprintf(stderr, "v2: %lx\n", v2);
	fprintf(stderr, "v3: %lx\n", v3);

	tb->v1 = v1;
	tb->v2 = v3;
	tb->v3 = v2;

	std::vector<uint32_t> points;

	for (int k = 0; k < SIMULATION_STEP; k++) {
		tick(tb);

		if (START_SIGNAL_TIME == k) tb->start = 1;
		else                        tb->start = 0;

		fprintf(stderr, "-------- Step %d -----\n", k);
		fprintf(stderr, "Done: %d\n",       tb->done);
		fprintf(stderr, "Fifo Write: %d\n", tb->fifo_write);
		fprintf(stderr, "Fifo point x: %d\n", (tb->p & 0xFFFF));
		fprintf(stderr, "Fifo point y: %d\n", (tb->p >> 16));

		fprintf(stderr, "-------- Debug outputs -------\n");
		fprintf(stderr, "state: %d\n",       tb->state);
		// fprintf(stderr, "x: %d\n",       tb->x);
		// fprintf(stderr, "y: %d\n",       tb->y);
		fprintf(stderr, "\n");

		if (tb->fifo_write) points.emplace_back(tb->p);
	}
	delete tb;

	fprintf(stderr, "Number of points: %lx\n", points.size());
	fprintf(stderr, "Last x: %x\n", points[points.size()-1] & 0xFFFF);
	for (auto p: points) fprintf(stdout, "%u ", p);
	fprintf(stdout, "\n");
	return 0;
}
