#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

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

#include "game.c"

GameState game = {0};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void) window;
	(void) scancode;
	(void) mods;

    if      (key == GLFW_KEY_W && action == GLFW_PRESS)   game.control.w = 1;
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE) game.control.w = 0;

	else if (key == GLFW_KEY_S && action == GLFW_PRESS)   game.control.s = 1;
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE) game.control.s = 0;

	else if (key == GLFW_KEY_A && action == GLFW_PRESS)   game.control.a = 1;
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE) game.control.a = 0;

	else if (key == GLFW_KEY_D && action == GLFW_PRESS)   game.control.d = 1;
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE) game.control.d = 0;

	else if (key == GLFW_KEY_Q && action == GLFW_PRESS)   game.control.q = 1;
	else if (key == GLFW_KEY_Q && action == GLFW_RELEASE) game.control.q = 0;

	else if (key == GLFW_KEY_E && action == GLFW_PRESS)   game.control.e = 1;
	else if (key == GLFW_KEY_E && action == GLFW_RELEASE) game.control.e = 0;

	else if (key == GLFW_KEY_U && action == GLFW_PRESS)   game.control.u = 1;
	else if (key == GLFW_KEY_U && action == GLFW_RELEASE) game.control.u = 0;

	else if (key == GLFW_KEY_I && action == GLFW_PRESS)   game.control.i = 1;
	else if (key == GLFW_KEY_I && action == GLFW_RELEASE) game.control.i = 0;
}


i32 main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_STRIDE, WINDOW_HEIGHT, "Hello World", NULL, NULL);
    if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

	game_init(&game);
	double last_time = glfwGetTime();

	static u32 canvas[WINDOW_STRIDE*WINDOW_HEIGHT];

    while (!glfwWindowShouldClose(window)) {
		double current_time = glfwGetTime();
		double dt = current_time - last_time;
		last_time = current_time;

		/* Poll for and process events */
		glfwPollEvents();
		game_update(&game, dt);

		glClearColor(1, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		game_draw(&game, canvas);

		glDrawPixels(WINDOW_STRIDE,
					 WINDOW_HEIGHT,
					 GL_RGBA,
					 GL_UNSIGNED_INT_8_8_8_8,
					 canvas);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

	}
    glfwTerminate();
    return 0;
}
