#include "std/math.h"
#include "std/memory.h"
#include "std/assert.h"
#include "std/defs.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define SWAP(T, a, b) do { T temp = a; a = b; b = temp; } while(0)
#define max3(a, b, c) max(a, max(b, c))
#define min3(a, b, c) min(a, min(b, c))

typedef volatile u8* canvas_ptr;
typedef u8 PixelType;

#define GREEN 0b00111000
#define RED 0b00000111
#define BLUE 0b11000000

#define YELLOW (GREEN | RED)
#define PURPLE ((RED | BLUE) - 1)
#define CYAN (GREEN | BLUE)

#define WHITE (RED | GREEN | BLUE)

#define BLACK 0

#define WINDOW_STRIDE 320
#define WINDOW_WIDTH 160
#define WINDOW_HEIGHT 120

#include "game.c"

#define KBDCRL  (*(volatile u32 *)0xFF200000)
#define KBDDATA (*(volatile u32 *)0xFF200004)

#define CURRENT_DISPLAY_FRAME_ADRESS (*(u32 *)0xFF200604)
#define CANVAS0_ADDR 0xFF000000
#define CANVAS1_ADDR 0xFF100000

canvas_ptr canvas;
canvas_ptr back_canvas;

u32 get_time()
{
	u32 value ;
	__asm__ volatile ("csrr    %0, time" 
					  : "=r" (value)  /* output : register */
					  : /* input : none */
					  : /* clobbers: none */);
	return value;
}

bool keyboard_event(char *key)
{
	bool ready = KBDCRL  & nbits_mask(1);
	u8 val     = KBDDATA & nbits_mask(8);
	if (ready) *key = val;
	return ready;
}

void update_control(GameState * game)
{
	game->control = (Control){0};
	char key;
	if (keyboard_event(&key)) {
		if      (key == 'w')   game->control.w = 1;
		else if (key == 's')   game->control.s = 1;
		else if (key == 'a')   game->control.a = 1;
		else if (key == 'd')   game->control.d = 1;
		else if (key == 'q')   game->control.q = 1;
		else if (key == 'e')   game->control.e = 1;
		else if (key == 'u')   game->control.u = 1;
		else if (key == 'i')   game->control.i = 1;
	}
}

void swap_buffer() {
	CURRENT_DISPLAY_FRAME_ADRESS ^= 1;
	SWAP(canvas_ptr, canvas, back_canvas);
}

GameState game;

i32 main()
{
	u32 last_time = get_time();

	CURRENT_DISPLAY_FRAME_ADRESS = 0;
	canvas = ((canvas_ptr)CANVAS0_ADDR);
	back_canvas = ((canvas_ptr)CANVAS1_ADDR);

	for (int i = 0; i < 320*240; i++) canvas[i] = BLACK;
	for (int i = 0; i < 320*240; i++) back_canvas[i] = BLACK;

	game_init(&game);
	game_draw(&game, canvas);
	return 0;
	
    while (true) {
		u32 current_time = get_time();
		f32 dt = (current_time - last_time) * 0.001f;
		last_time = current_time;

	    update_control(&game);
		game_update(&game, dt);
		game_draw(&game, back_canvas);

		swap_buffer();
	}
    return 0;
}
