#include "canvas.h"
#include "objects.h"
#include "shapes.h"
#include <string.h>
#include <stdlib.h>

/* The global 2-D canvas dimensions and pointer */
int ROWS = 10;
int COLS = 76;
char **canvas = NULL;

/* ------------------------------------------------------------------ */
void canvas_alloc(int rows, int cols)
{
    canvas_free();
    ROWS = rows;
    COLS = cols;
    canvas = malloc(ROWS * sizeof(char *));
    for (int r = 0; r < ROWS; r++) {
        canvas[r] = malloc(COLS * sizeof(char));
    }
}

void canvas_free(void)
{
    if (canvas != NULL) {
        for (int r = 0; r < ROWS; r++) {
            free(canvas[r]);
        }
        free(canvas);
        canvas = NULL;
    }
}

/* ------------------------------------------------------------------ */
void canvas_init(void)
{
    if (canvas == NULL) return;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

/* ------------------------------------------------------------------ */
/* Maps Cartesian coordinates (row=y, col=x) to array indices:
   r_idx = ROWS/2 - row
   c_idx = COLS/2 + col */
void canvas_set(int row, int col, char ch)
{
    if (canvas == NULL) return;
    int r_idx = (ROWS / 2) - row;
    int c_idx = (COLS / 2) + col;
    if (r_idx >= 0 && r_idx < ROWS && c_idx >= 0 && c_idx < COLS)
        canvas[r_idx][c_idx] = ch;
}

/* ------------------------------------------------------------------ */
char canvas_get(int row, int col)
{
    if (canvas == NULL) return ' ';
    int r_idx = (ROWS / 2) - row;
    int c_idx = (COLS / 2) + col;
    if (r_idx >= 0 && r_idx < ROWS && c_idx >= 0 && c_idx < COLS)
        return canvas[r_idx][c_idx];
    return ' ';
}

/* ------------------------------------------------------------------ */
void canvas_display(WINDOW *win, int win_row, int win_col)
{
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++)
            mvwaddch(win, win_row + r, win_col + c, canvas[r][c]);
    }
}

/* ------------------------------------------------------------------ */
/* Erase canvas, then re-render every live object */
void canvas_redraw(void)
{
    canvas_init();
    for (int i = 0; i < obj_count; i++) {
        if (objects[i].alive) {
            Shape *s = &objects[i];
            switch (s->type) {
            case CIRCLE:
                draw_circle(s->y1, s->x1, s->radius, s->fill_char);
                break;
            case RECTANGLE:
                draw_rectangle(s->y1, s->x1, s->y2, s->x2, s->fill_char);
                break;
            case LINE:
                draw_line(s->y1, s->x1, s->y2, s->x2, s->fill_char);
                break;
            case TRIANGLE:
                draw_triangle(s->y1, s->x1,
                              s->y2, s->x2,
                              s->y3, s->x3,
                              s->fill_char);
                break;
            }
        }
    }
}
