#include "std/defs.h"
#include "std/memory.h"

void _enter(void)  __attribute__ ((naked, section(".text.metal.init.enter"), optimize("O0")));
void _enter(void)
{
    // Setup SP and GP
    // The locations are defined in the linker script
    __asm__ volatile  (
					   ".option push;"
					   ".option norelax;"
					   "la    gp, __global_pointer$;"
					   ".option pop;"
					   "la    sp, __stack_pointer$;"
					   "jal   zero, _start;"
					   : /* output: none %0 */
					   : /* input: none */
					   : /* clobbers: none */); 
    // This point will not be executed, _start() will be called with no return.
}

extern i32 main(void);
extern u32 __bss_start;
extern u32 __bss_end;

// At this point we have a stack and global pointer, but no access to global variables.
void _start(void) __attribute__ ((noreturn, optimize("O0")));
void _start(void)
{

    // Init memory regions
    // Clear the .bss section (global variables with no initial values)
	u8 *bss = (u8 *) &__bss_start;
	u8 *bss_end = (u8 *) &__bss_end;
	u32 bss_size = bss_end - bss;
	memset(bss, 0, bss_size);
    int rc = main();
    _Exit(rc);
}

void _Exit(int exit_code) __attribute__ ((noreturn, noinline, optimize("O0")));
void _Exit(int exit_code)
{
    (void)exit_code;
    // Halt
    while (1) {}
}
