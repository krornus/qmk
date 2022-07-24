#ifdef OLED_ENABLE
#include "ripple.h"

#ifndef QMK_EMULATOR
#include "print.h"
#endif

/* maximum number of ripples */
#define RIPPLE_MAX 15
/* time between frames in ms */
#define RIPPLE_FRAMETIME 100
/* period of the ripple in ms */
#define RIPPLE_PERIOD 1000
/* wavelength pf the ripple in px */
#define RIPPLE_WAVELENGTH 10
/* time before ripple ceases */
#define RIPPLE_TIMEOUT 2500

typedef struct ripple ripple_t;

struct ripple {
    uint8_t x;
    uint8_t y;
    uint8_t wavelength;
    uint16_t period;
    uint16_t start;
    uint16_t timeout;
};

static uint8_t rippndx = 0;
static ripple_t ripples[RIPPLE_MAX];

static inline void pixel(int x, int y)
{
    if (x < 0 || x > UINT8_MAX) {
        return;
    }

    if (y < 0 || y > UINT8_MAX) {
        return;
    }

    oled_write_pixel((uint8_t)x, (uint8_t)y, true);
}

static inline void putquads(int xc, int yc, int x, int y, int dapple)
{
    uint8_t pct;

    pct = rand() % 100;
    if (pct < dapple) {
        return;
    }

    pixel(xc+x, yc+y);
    pixel(xc-x, yc+y);
    pixel(xc+x, yc-y);
    pixel(xc-x, yc-y);
    pixel(xc+y, yc+x);
    pixel(xc-y, yc+x);
    pixel(xc+y, yc-x);
    pixel(xc-y, yc-x);
}

/*
 * bresenham’s circle drawing algorithm
 */
static void putcircle(int xc, int yc, int r, int dapple)
{
    int x, y, d;

    x = 0;
    y = r;

    d = 3 - 2 * r;

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

static void addripple(int x, int y)
{
    ripples[rippndx] = (ripple_t){
        .x = x,
        .y = y,
        .wavelength = RIPPLE_WAVELENGTH,
        .period = RIPPLE_PERIOD,
        .start = timer_read(),
        .timeout = RIPPLE_TIMEOUT,
    };

    rippndx = (rippndx + 1) % RIPPLE_MAX;
}

static bool ripple(ripple_t *r)
{
    int dapple;
    uint16_t count;
    uint16_t radius;
    uint16_t elapsed;
    uint16_t timeout;
    float ratio;

    timeout = r->timeout / 2;

    /* elapsed time for the ripple */
    elapsed = timer_elapsed(r->start);
    /* check if the ripple has dissapated */
    if (elapsed > r->timeout) {
        r->start = 0;
        return false;
    }

    /* the number of ripples */
    count = (elapsed + (r->period - 1)) / r->period;
    /* the radius of the innermost ripple
     * v = freq * wavelength
     * v = 1/period * wavelength
     * v = wavelength / period
     * d = t * v
     * d = t * wavelength / period
     * let p = t / period
     * d = p * wavelength */
    ratio = ((float)(elapsed % r->period)) / ((float)r->period);
    radius = r->wavelength * ratio;

    /* if we are past the timeout, stop creating inner circles */
    if (elapsed > timeout) {
        uint16_t max;
        uint16_t removed;

        /* get max number of circles possible */
        max = (timeout + (r->period - 1)) / r->period;
        /* this gives us number of circles to delete */
        removed = count - max;

        count -= removed;
        radius += r->wavelength * removed;
    }

    dapple = (((float)elapsed) / ((float)r->timeout)) * 100;
    for (uint16_t i = 0; i < count; i++) {
        putcircle(r->x, r->y, radius, dapple);
        radius += r->wavelength;
    }

    return true;
}

void ripple_init(void)
{
    srand(timer_read());
}

void process_record_ripples(keyrecord_t *record)
{
    /* TODO: find oled size #defines so numbers arent hardcoded */
    if (record->event.pressed) {
        addripple(
            rand() % 128, rand() % 64
        );
    }
}

void oled_write_ripples(void)
{
    static uint16_t key_timer;
    static bool clear = true;

    int count;

    if (timer_elapsed(key_timer) > RIPPLE_FRAMETIME) {
        key_timer = timer_read();

        if (!clear) {
            oled_clear();
        }

        count = 0;
        for (size_t i = 0; i < RIPPLE_MAX; i++) {
            if (ripples[i].start) {
                count += ripple(&ripples[i]);
            }
        }

        if (!count) {
            oled_clear();
            clear = true;
        } else {
            clear = false;
        }
    }
}
#else
enum empty { NIL };
#endif
