#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdint.h>
#include <stdbool.h>
#include<SDL.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

enum cull_method {
	CULL_NONE,
	CULL_BACKFACE
} cull_method;

enum render_method {
	RENDER_WIRE,
	RENDER_WIRE_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIRE,
	RENDER_TEXTURED,
	RENDERED_TEXTURED_WIRE
} render_method;

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern uint32_t* color_buffer;
extern float* z_buffer;
extern SDL_Texture* color_buffer_texture;

extern int window_width;
extern int window_height;

bool InitializeWindow(void);

void draw_pixel(int x, int y, uint32_t color);

void draw_line(int x0, int y0, int x1, int y1, uint32_t color);

void draw_grid(void);

void draw_rect(int x, int y, int height, int width, uint32_t color);

void render_color_buffer(void);

void clear_color_buffer(uint32_t color);

void clear_z_buffer(void);

void destroy_window(void);
#endif // !DISPLAY_H
