#include "canvas.h"
#include "objects.h"
#include "shapes.h"
#include <string.h>

/* The global 2-D canvas */
char canvas[ROWS][COLS];

/* ------------------------------------------------------------------ */
void canvas_init(void)
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

/* ------------------------------------------------------------------ */
void canvas_set(int row, int col, char ch)
{
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = ch;
}

/* ------------------------------------------------------------------ */
char canvas_get(int row, int col)
{
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        return canvas[row][col];
    return ' ';
}

/* ------------------------------------------------------------------ */
void canvas_display(WINDOW *win, int win_row, int win_col)
{
    /* Draw border */
    for (int r = 0; r < ROWS; r++) {
        mvwprintw(win, win_row + r, win_col, "|");
        for (int c = 0; c < COLS; c++)
            mvwaddch(win, win_row + r, win_col + 1 + c, canvas[r][c]);
        mvwprintw(win, win_row + r, win_col + 1 + COLS, "|");
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
