# C to Risc
This project aims to be a interface between C and UnB custom RISC-V processor.
Take a look at file *main.c* inside *src* folder to see an example

# Dependencies
You need `riscv64-unknown-elf-gcc`, and `python3`

# How to run
Just run `make dump`, this will compile the project and generate both .mif files and .bin file inside build

# Adding your source code to make file
The simple way is to add your files src files to the the variable _SOURCES_ inside Makefile