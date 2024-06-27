# Cpp flags
CPPFLAGS= -Wall -Wextra -Wshadow -std=c++11 -O2 -g

all: build/lr_sim build/lr_test 

build/lr_test: test_lr_gen.cpp
	@mkdir -p $(dir $@)
	g++ $(CPPFLAGS) -o $@ $^

build/lr_sim: sim_lr_gen.cpp lr_gen.v
	verilator --cc --exe --build -j 0 -Wall  $^
	@mkdir -p $(dir $@)
	cp obj_dir/Vlr_gen build/lr_sim

.PHONY: all clean	


clean:
	rm -rf build
	rm -rf obj_dir
