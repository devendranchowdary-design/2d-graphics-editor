#include "shapes.h"
#include "canvas.h"
#include <stdlib.h>

/* ================================================================== */
/*  Helper: plot 8-fold symmetry for circle                           */
/* ================================================================== */
static void plot8(int cy, int cx, int dy, int dx, char ch)
{
    canvas_set(cy + dy, cx + dx, ch);
    canvas_set(cy + dy, cx - dx, ch);
    canvas_set(cy - dy, cx + dx, ch);
    canvas_set(cy - dy, cx - dx, ch);
    canvas_set(cy + dx, cx + dy, ch);
    canvas_set(cy + dx, cx - dy, ch);
    canvas_set(cy - dx, cx + dy, ch);
    canvas_set(cy - dx, cx - dy, ch);
}

/* ================================================================== */
/*  Circle  — Mid-point (Bresenham) circle algorithm                  */
/* ================================================================== */
void draw_circle(int cy, int cx, int radius, char ch)
{
    int x = 0, y = radius;
    int p = 1 - radius;        /* decision parameter                  */

    plot8(cy, cx, y, x, ch);

    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plot8(cy, cx, y, x, ch);
    }
}

/* ================================================================== */
/*  Rectangle  — Four straight sides                                  */
/* ================================================================== */
void draw_rectangle(int r1, int c1, int r2, int c2, char ch)
{
    /* Normalise */
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }

    /* Top and bottom edges */
    for (int c = c1; c <= c2; c++) {
        canvas_set(r1, c, ch);
        canvas_set(r2, c, ch);
    }
    /* Left and right edges */
    for (int r = r1; r <= r2; r++) {
        canvas_set(r, c1, ch);
        canvas_set(r, c2, ch);
    }
}

/* ================================================================== */
/*  Line  — Bresenham's line algorithm                                */
/* ================================================================== */
void draw_line(int r1, int c1, int r2, int c2, char ch)
{
    int dr = abs(r2 - r1);
    int dc = abs(c2 - c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        canvas_set(r1, c1, ch);
        if (r1 == r2 && c1 == c2) break;

        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

/* ================================================================== */
/*  Triangle  — Three sides via draw_line()                           */
/* ================================================================== */
void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3, char ch)
{
    draw_line(r1, c1, r2, c2, ch);
    draw_line(r2, c2, r3, c3, ch);
    draw_line(r3, c3, r1, c1, ch);
}
