#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include "VFifo.h"
#include "verilated.h"
#include <random>
#include <assert.h>
#include <time.h>

const int SIMULATION_STEP = 1000;
const int START_SIGNAL_TIME = 4;

void tick(VFifo *tb) {
	tb->eval();
	tb->clock = 1;
	tb->eval();
	tb->clock = 0;
	tb->eval();
}

int main(int argc, char **argv) {
	srand(time(0));
	Verilated::commandArgs(argc, argv);
	VFifo *tb = new VFifo;

	std::queue<unsigned int> q;

	for (int k = 0; k < SIMULATION_STEP; k++) {
		int b = rand() % 2;
		unsigned int v = rand();
		unsigned int queue_out = 0;
		if (b) {
			if (q.size() < 1024) q.push(v);
		}
		else {
			if (q.size() > 0) {
				queue_out = q.front();
				q.pop();
			}
		}
		tb->wrreq = b;
		tb->rdreq = !b;
		tb->data = v;

		if (b && !tb->full) fprintf(stderr, "Wrinting value = %u\n", v);

		tick(tb);

		assert(tb->full == (q.size() == 1024));
		assert(tb->empty == (q.size() == 0));

		if (!b && !tb->empty) assert(tb->q == queue_out);

		if (!b && !tb->empty)  fprintf(stderr, "Readed value = %u\n", tb->q);
	}
	delete tb;
	return 0;
}
