# Cpp flags
CPPFLAGS= -Wall -Wextra -Wshadow -std=c++11 -O2 -g
VFLAGS = -Wall -j 0

VINCLUDE_DIR=V

all: build/sim_gen_triangle_boundries build/sim_fifo build/sim_gen_triangle_points

build/lr_test: sim/test_lr_gen.cpp
	@mkdir -p $(dir $@)
	g++ $(CPPFLAGS) -o $@ $^

build/sim_gen_triangle_boundries: sim/gen_triangle_boundries.cpp V/GenTriangleBoundries.v
	verilator --cc --exe --build $(VFLAGS) -CFLAGS "$(CPPFLAGS)" -I$(VINCLUDE_DIR) $^
	@mkdir -p $(dir $@)
	cp obj_dir/VGenTriangleBoundries $@

build/sim_fifo: sim/fifo.cpp V/Fifo.v
	verilator --cc --exe --build $(VFLAGS) -CFLAGS "$(CPPFLAGS)" -I$(VINCLUDE_DIR) $^
	@mkdir -p $(dir $@)
	cp obj_dir/VFifo $@

build/sim_gen_triangle_points: sim/gen_triangle_points.cpp V/GenTrianglePoints.v
	verilator --cc --exe --build $(VFLAGS) -CFLAGS "$(CPPFLAGS)" -I$(VINCLUDE_DIR) $^
	@mkdir -p $(dir $@)
	cp obj_dir/VGenTrianglePoints $@

.PHONY: all clean	

clean:
	rm -rf build
	rm -rf obj_dir
