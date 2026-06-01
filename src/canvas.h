#ifndef CANVAS_H
#define CANVAS_H

#include <ncurses.h>

#define ROWS    40
#define COLS    120
#define BORDER_CHAR '|'

/* The 2-D character canvas */
extern char canvas[ROWS][COLS];

/* Canvas offset inside the ncurses window */
#define CANVAS_WIN_ROW  2
#define CANVAS_WIN_COL  0

/* Initialise canvas to spaces */
void canvas_init(void);

/* Set a single pixel, bounds-checked */
void canvas_set(int row, int col, char ch);

/* Get a single pixel */
char canvas_get(int row, int col);

/* Print the canvas to stdscr (ncurses) starting at (win_row, win_col) */
void canvas_display(WINDOW *win, int win_row, int win_col);

/* Erase the canvas and redraw all live objects */
void canvas_redraw(void);

#endif /* CANVAS_H */
