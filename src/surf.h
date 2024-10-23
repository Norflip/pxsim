#ifndef _DRAW_H
#define _DRAW_H
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>

typedef struct pixel {
    uint8_t g, b, r;
} pixel_t;

#define TEXT_MAX 1024

#define umin(a, b) ((a) < (b) ? (a) : (b))
#define umax(a, b) ((a) > (b) ? (a) : (b))

#define PXRED ((pixel_t){0, 0, 255})
#define PXGREEN ((pixel_t){0, 255, 0})
#define PXBLUE ((pixel_t){255, 0, 0})
#define PXWHITE ((pixel_t){255, 255, 255})

typedef struct bitmap {
    int width, height;
    pixel_t* pixels;
} surface_t;

surface_t createSurface(int w, int h);
void freeSurface(surface_t* s);
void surface_clear(surface_t* s);
void surface_blit(surface_t* s, HDC hdc);
void blitSurfaceEx(surface_t* s, HDC hdc, int x, int y, int w, int h);

void surfaceUVDemo(surface_t* s);

#define set_pixel draw_pixel
void draw_pixel(surface_t* s, int x, int y, pixel_t p);
void draw_line(surface_t* s, int x0, int y0, int x1, int y1, pixel_t p);
void draw_rect(surface_t* s, int x, int y, int w, int h, pixel_t p, bool fill);
void draw_circle(surface_t* s, int x, int y, int r, pixel_t p, bool fill);
void draw_text(surface_t* s, int x, int y, pixel_t p, const char* text, ...);

#endif  // _DRAW_H