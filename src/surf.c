#include "surf.h"

#include "font.h"

surface_t createSurface(int width, int height) {
    surface_t s = {0};
    s.width = width;
    s.height = height;
    s.pixels = malloc(sizeof(surface_t) + width * height * sizeof(pixel_t));
    surface_clear(&s);
    return s;
}

void freeSurface(surface_t* s) {
    free(s->pixels);
    s->pixels = NULL;
    s->width = 0;
    s->height = 0;
}

void surface_clear(surface_t* s) {
    memset(s->pixels, 0, s->width * s->height * sizeof(pixel_t));
}

void surface_blit(surface_t* s, HDC hdc) {
    // get the window size
    RECT rc;
    GetClientRect(WindowFromDC(hdc), &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    blitSurfaceEx(s, hdc, 0, 0, width, height);
}

void blitSurfaceEx(surface_t* s, HDC hdc, int x, int y, int w, int h) {
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = s->width;
    bmi.bmiHeader.biHeight = -s->height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(hdc, x, y, w, h, 0, 0, s->width, s->height, s->pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
}

void surfaceUVDemo(surface_t* s) {
    // UV demo
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            s->pixels[y * s->width + x] = (pixel_t){
                0,
                (uint8_t)(x * 255 / s->width),
                (uint8_t)(y * 255 / s->height),
            };
        }
    }
}

void draw_pixel(surface_t* s, int x, int y, pixel_t p) {
    if (x < 0 || y < 0 || x >= s->width || y >= s->height)
        return;
    s->pixels[y * s->width + x] = p;
}

void draw_line(surface_t* s, int x0, int y0, int x1, int y1, pixel_t p) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (set_pixel(s, x0, y0, p), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_rect(surface_t* s, int x, int y, int w, int h, pixel_t p, bool fill) {
    if (fill) {
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                draw_pixel(s, x + i, y + j, p);
            }
        }
    } else {
        for (int i = 0; i < w; i++) {
            draw_pixel(s, x + i, y, p);
            draw_pixel(s, x + i, y + h - 1, p);
        }
        for (int j = 0; j < h; j++) {
            draw_pixel(s, x, y + j, p);
            draw_pixel(s, x + w - 1, y + j, p);
        }
    }
}

void draw_circle(surface_t* s, int x, int y, int r, pixel_t p, bool fill) {
    if (fill) {
        for (int j = -r; j <= r; j++) {
            for (int i = -r; i <= r; i++) {
                if (i * i + j * j <= r * r) {
                    draw_pixel(s, x + i, y + j, p);
                }
            }
        }
    } else {
        for (int i = 0; i < r; i++) {
            int j = (int)(sqrt(r * r - i * i) + 0.5);
            draw_pixel(s, x + i, y + j, p);
            draw_pixel(s, x - i, y + j, p);
            draw_pixel(s, x + i, y - j, p);
            draw_pixel(s, x - i, y - j, p);
            draw_pixel(s, x + j, y + i, p);
            draw_pixel(s, x - j, y + i, p);
            draw_pixel(s, x + j, y - i, p);
            draw_pixel(s, x - j, y - i, p);
        }
    }
}

static void draw_char(surface_t* s, int x, int y, pixel_t p, char c) {
    for (int dx = 0; dx < 5; dx++) {
        for (int dy = 0; dy < FONT_HEIGHT; dy++) {
            if (fontdata[(int)c][dx] & (1 << dy)) {
                draw_pixel(s, x + dx, y + dy, p);
            }
        }
    }
}

void draw_text(surface_t* s, int x, int y, pixel_t p, const char* text, ...) {

    char buffer[TEXT_MAX] = {0};
    va_list args;
    va_start(args, text);
    int len = vsnprintf(buffer, TEXT_MAX, text, args);
    va_end(args);

    if(len <= 0)
        return;

    int dx = 0;
    int dy = 0;

    for (int i = 0; i < len; i++) {
        char c = buffer[i];
        if (c == '\0')
            break;

        if (c == '\n') {
            dx = 0;
            dy += FONT_HEIGHT + INTERCHAR_SPACE;
            continue;
        }

        if (c == '\t' || c == '\r') {
            dx += (FONT_WIDTH + INTERCHAR_SPACE) * 4;
            continue;
        }

        if (c < 0x20 || c >= 0x7f) {
            // dx += FONT_WIDTH + INTERCHAR_SPACE;
            continue;
        }

        c -= ASCII_OFFSET;
        for (int row = 0; row < FONT_HEIGHT; row++) {
            for (int col = 0; col < FONT_WIDTH; col++) {
                if (fontdata[c][col] & (1 << (FONT_HEIGHT - row))) {
                    set_pixel(s, x + dx + col, y + dy + row, p);
                }
            }
        }
        dx += FONT_WIDTH + INTERCHAR_SPACE;
    }
}
