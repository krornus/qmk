#include <err.h>
#include <time.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "qmk.h"

typedef struct color color_t;

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

static inline color_t color(bool on)
{
    if (on) {
        return (color_t){
            .r = 214,
            .g = 244,
            .b = 255,
            .a = 0xff
        };
    } else {
        return (color_t){
            .r = 0x00,
            .g = 0x00,
            .b = 0x00,
            .a = 0xff
        };
    }
}

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;

static SDL_Window *win;
static SDL_Renderer *ren = NULL;
static bool dirty = true;

extern bool oled_task_kb(void) __attribute__ ((weak, alias ("_oled_task_kb")));
extern bool oled_task_user(void) __attribute__ ((weak, alias ("_oled_task_user")));
extern bool process_record_user(uint16_t keycode, keyrecord_t *record) __attribute__ ((weak, alias ("_process_record_user")));

void init(void)
{
    SDL_Surface *surface;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        err(1, "failed to initialize sdl");
    }

    win = SDL_CreateWindow(
        "OLED",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!win) {
        err(1, "failed to create window");
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        err(1, "failed to create renderer");
    }

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);

    surface = SDL_GetWindowSurface(win);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x00, 0x00, 0x00));
    SDL_UpdateWindowSurface(win);
}

void destroy(void)
{
    SDL_DestroyWindow(win);
    SDL_Quit();
}

static void setcolor(color_t c)
{
    static color_t color = {0};

    if (memcmp(&c, &color, sizeof(color)) != 0) {
        SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
        color = c;
    }
}

void oled_clear(void)
{
    dirty = true;

    setcolor(color(0));
    SDL_RenderClear(ren);
}

static void flush(void)
{
    SDL_RenderPresent(ren);
}

static void putpixel(uint16_t x, uint16_t y, color_t c)
{
    setcolor(c);
    SDL_RenderDrawPoint(ren, x, y);
}

void oled_write_pixel(uint8_t x, uint8_t y, bool on)
{
    dirty = true;

    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        putpixel(x, y, color(on));
    }
}

uint16_t timer_read(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == -1) {
        err(1, "clock_gettime");
    }

    return (ts.tv_sec * 1000 + (ts.tv_nsec / 1000000)) & 0xffff;
}

uint16_t timer_elapsed(uint16_t last)
{
    uint16_t t;
    t = timer_read();
    return t - last;
}

bool _oled_task_kb(void)
{
    return oled_task_user();
}

bool _oled_task_user(void)
{
    return true;
}

bool _process_record_user(uint16_t keycode, keyrecord_t *record)
{
    (void)keycode;
    (void)record;
    return true;
}

int main()
{
    keyrecord_t r;
    SDL_Event ev;
    bool quit;

    init();
    oled_task_user();
    flush();

    quit = false;
    while (!quit) {
        dirty = false;
        oled_task_user();
        if (dirty) {
            flush();
        }

        while(SDL_PollEvent(&ev) != 0) {
            if(ev.type == SDL_QUIT) {
                quit = true;
            } else if(ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
                memset(&r, 0, sizeof(r));
                r.event.key.col = rand() % 8;
                r.event.key.row = rand() % 4;
                r.event.pressed = ev.type == SDL_KEYDOWN;
                r.keycode = ev.key.keysym.sym;

                process_record_user(ev.key.keysym.sym, &r);

                switch (ev.key.keysym.sym) {
                    case SDLK_q:
                        quit = true;
                        break;
                }
            }
        }
    }

    destroy();
}
