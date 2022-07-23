#include "ripple.h"

/* maximum number of ripples */
#define RIPPLE_MAX_COUNT 3
/* initial radius of a ripple */
#define RIPPLE_RADIUS 2
/* step size of each ring */
#define RIPPLE_STEP 5
/* maximum number of steps before disappearing */
#define RIPPLE_STEP_MAX 5
/* base probability a ripple pixel gets dappled */
#define RIPPLE_DAPPLE 5
/* speed of ripples */
#define RIPPLE_TRAVEL 5

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

static inline void putquads(int xc, int yc, int x, int y, int dapple, bool on)
{
    bool paint;

    if (on) {
        paint = (rand() % 100) > dapple;
    } else {
        paint = false;
    }

    pixel(xc+x, yc+y, paint);
    pixel(xc-x, yc+y, paint);
    pixel(xc+x, yc-y, paint);
    pixel(xc-x, yc-y, paint);
    pixel(xc+y, yc+x, paint);
    pixel(xc-y, yc+x, paint);
    pixel(xc+y, yc-x, paint);
    pixel(xc-y, yc-x, paint);
}

static void putcircle(int xc, int yc, int r, int dapple, bool on)
{
    int x, y, d;

    /* bresenham’s circle drawing algorithm */
    x = 0;
    y = r;

    d = 3 - 2 * r;

    while (y >= x) {
        putquads(xc, yc, x, y, dapple, on);
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
        .radius = RIPPLE_RADIUS,
        .step = RIPPLE_STEP,
        .iteration = 1,
    };

    rippndx = (rippndx + 1) % RIPPLE_MAX_COUNT;
}

static void draw(ripple_t *r, uint16_t iteration, int dapple, bool on)
{
    uint16_t radius;
    uint16_t travel;

    radius = r->radius + r->step * iteration;
    /* the amount the radius has traveled */
    travel = RIPPLE_TRAVEL * (iteration - 1);

    for (size_t i = 0; i < iteration; i++) {
        putcircle(r->x, r->y, radius + travel, dapple*iteration, on);
        radius += r->step;
    }
}

static bool ripple(ripple_t *r, int dapple)
{
    /* step 1: undraw all ripples */
    if (r->iteration > 1) {
        draw(r, r->iteration - 1, 0, false);
    }

    if (!r->iteration) {
        return false;
    } else if (r->iteration > RIPPLE_STEP_MAX) {
        r->iteration = 0;
        return false;
    }

    /* step 2: redraw new ripples */
    if (r->iteration) {
        draw(r, r->iteration, dapple, true);
    }

    r->iteration++;

    return r->iteration <= RIPPLE_STEP_MAX;
}

void process_record_ripples(keyrecord_t *record)
{
    /* TODO: find oled size #defines so numbers arent hardcoded */
    if (record->event.pressed) {
        addripple(
            128 - record->event.key.col * (128 / MATRIX_COLS),
            record->event.key.row * (64 / MATRIX_ROWS)
        );
    }
}

void oled_write_ripples(void)
{
    static uint16_t key_timer;
    static bool clear = true;

    int count;

    if (timer_elapsed(key_timer) > 100) {
        key_timer = timer_read();

        count = 0;
        for (size_t i = 0; i < RIPPLE_MAX_COUNT; i++) {
            count += ripple(&ripples[i], RIPPLE_DAPPLE);
        }

        if (!count) {
            if (!clear) {
                clear = true;
                oled_clear();
            }
        } else {
            clear = false;
        }
    }
}
