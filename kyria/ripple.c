#include "ripple.h"

/* maximum number of ripples */
#define RIPPLE_MAX_COUNT 5
/* initial radius of a ripple */
#define RIPPLE_RADIUS 2
/* step size of each ring */
#define RIPPLE_STEP 10
/* maximum number of steps before disappearing */
#define RIPPLE_STEP_MAX 5
/* base probability a ripple pixel gets dappled */
#define RIPPLE_DAPPLE 30

typedef struct ripple ripple_t;

struct ripple {
    uint8_t x;
    uint8_t y;
    uint8_t radius;
    uint8_t step;
    uint8_t iteration;
};

static uint8_t rippndx = 0;
static ripple_t ripples[RIPPLE_MAX_COUNT];

static inline void pixel(int x, int y, bool on)
{
    if (x < 0 || x > UINT8_MAX) {
        return;
    }

    if (y < 0 || y > UINT8_MAX) {
        return;
    }

    oled_write_pixel((uint8_t)x, (uint8_t)y, on);
}

static inline void putquads(int xc, int yc, int x, int y, int dapple)
{
    bool paint;

    paint = (rand() % 100) > dapple;

    pixel(xc+x, yc+y, paint);
    pixel(xc-x, yc+y, paint);
    pixel(xc+x, yc-y, paint);
    pixel(xc-x, yc-y, paint);
    pixel(xc+y, yc+x, paint);
    pixel(xc-y, yc+x, paint);
    pixel(xc+y, yc-x, paint);
    pixel(xc-y, yc-x, paint);
}

static void putcircle(int xc, int yc, int r, int dapple)
{
    int x, y, d;

    /* bresenham’s circle drawing algorithm */
    x = 0;
    y = r;

    d = 3 - 2*r;

    while (y >= x) {
        putquads(xc, yc, x, y, dapple);
        x++;

        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

static bool putripple(ripple_t *r)
{
    uint16_t rad;

    if (!r->iteration || r->iteration > RIPPLE_STEP_MAX) {
        return false;
    }

    rad = r->radius;

    for (size_t i = 0; i < r->iteration; i++) {
        putcircle(r->x, r->y, rad, i*RIPPLE_DAPPLE);
        rad = rad + r->step;
    }

    r->radius += RIPPLE_STEP;
    r->iteration++;

    return true;
}

static void addripple(int x, int y)
{
    ripples[rippndx] = (ripple_t){
        .x = x,
        .y = y,
        .radius = RIPPLE_RADIUS,
        .step = RIPPLE_STEP,
        .iteration = 1,
    };

    rippndx = (rippndx + 1) % RIPPLE_MAX_COUNT;
}

static void putripples(void)
{
    int count;

    count = 0;
    for (size_t i = 0; i < RIPPLE_MAX_COUNT; i++) {
        count += putripple(&ripples[i]);
    }

    static bool clear = true;

   if (!count) {
       if (!clear) {
           oled_clear();
           clear = true;
       }
   } else {
       clear = false;
   }
}

void process_record_ripples(keyrecord_t *record)
{
    if (record->event.pressed) {
        addripple(record->event.key.col * 16, record->event.key.row * 16);
    }
}

void oled_write_ripples(void)
{
    static uint16_t key_timer;

    oled_clear();

    if (timer_elapsed(key_timer) > 100) {
        key_timer = timer_read();
        putripples();
    }
}
