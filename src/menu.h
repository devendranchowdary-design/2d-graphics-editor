#ifndef MENU_H
#define MENU_H

#include <ncurses.h>

/*
 * Initialise ncurses and create the sub-windows.
 * Must be called once before any other menu_ function.
 */
void menu_init(void);

/*
 * Tear down ncurses.
 */
void menu_cleanup(void);

/*
 * Run the main interactive loop.
 * Returns when the user chooses Quit.
 */
void menu_run(void);

#endif /* MENU_H */
