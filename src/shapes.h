#ifndef SHAPES_H
#define SHAPES_H

/*
 * Low-level rasterisation functions.
 * All functions write directly to the canvas array via canvas_set().
 * Coordinates: row = y, col = x  (row 0 = top of canvas).
 */

/* Bresenham's / mid-point circle */
void draw_circle(int cy, int cx, int radius, char ch);

/* Axis-aligned rectangle (border only) */
void draw_rectangle(int r1, int c1, int r2, int c2, char ch);

/* Bresenham's line */
void draw_line(int r1, int c1, int r2, int c2, char ch);

/* Triangle from three vertices (draws three sides) */
void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3, char ch);

#endif /* SHAPES_H */
