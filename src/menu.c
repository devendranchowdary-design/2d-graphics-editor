/*
 * menu.c  —  ncurses-based interactive UI for the 2-D Graphics Editor
 *
 * Layout:
 *   +----------- Title bar (1 row) -----------+
 *   |         Canvas  (ROWS+2 rows)            |
 *   +----------  Status bar (1 row) -----------+
 *   |         Command area  (rest)             |
 *   +------------------------------------------+
 */

#include "menu.h"
#include "canvas.h"
#include "objects.h"
#include "shapes.h"

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/*  Window pointers                                                    */
/* ------------------------------------------------------------------ */
static WINDOW *win_title   = NULL;
static WINDOW *win_canvas  = NULL;
static WINDOW *win_status  = NULL;
static WINDOW *win_cmd     = NULL;

#define TITLE_ROWS   1
#define STATUS_ROWS  1
#define CMD_ROWS     10  /* command / input area height */

/* ------------------------------------------------------------------ */
/*  Forward declarations                                               */
/* ------------------------------------------------------------------ */
static void ui_refresh(void);
static void status_msg(const char *msg);
static int  prompt_int(const char *label, int *out);
static int  prompt_char(const char *label, char *out);
static void do_add(void);
static void do_delete(void);
static void do_modify(void);
static void do_list(void);
static void draw_main_menu(int selected);

/* ================================================================== */
/*  menu_init                                                          */
/* ================================================================== */
void menu_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);            /* hide cursor by default */

    /* Colour */
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN,    COLOR_BLACK);   /* title        */
        init_pair(2, COLOR_WHITE,   COLOR_BLACK);   /* canvas       */
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);   /* status       */
        init_pair(4, COLOR_GREEN,   COLOR_BLACK);   /* selected item*/
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);   /* shape char   */
    }

    int max_r, max_c;
    getmaxyx(stdscr, max_r, max_c);

    /* Check minimum terminal size */
    int need_rows = TITLE_ROWS + ROWS + 2 + STATUS_ROWS + CMD_ROWS;
    int need_cols = COLS + 2;
    if (max_r < need_rows || max_c < need_cols) {
        endwin();
        printf("ERROR: Terminal too small!\n");
        printf("  Your terminal : %d rows x %d cols\n", max_r, max_c);
        printf("  Minimum needed: %d rows x %d cols\n", need_rows, need_cols);
        printf("Please resize your terminal window and try again.\n");
        exit(1);
    }


    /* Title bar */
    win_title = newwin(TITLE_ROWS, COLS + 2, 0, 0);
    wbkgd(win_title, COLOR_PAIR(1) | A_BOLD);

    /* Canvas window: ROWS tall, COLS+2 wide (borders) */
    win_canvas = newwin(ROWS + 2, COLS + 2, TITLE_ROWS, 0);
    wbkgd(win_canvas, COLOR_PAIR(2));

    /* Status bar */
    int canvas_bottom = TITLE_ROWS + ROWS + 2;
    win_status = newwin(STATUS_ROWS, COLS + 2, canvas_bottom, 0);
    wbkgd(win_status, COLOR_PAIR(3) | A_BOLD);

    /* Command / input area */
    win_cmd = newwin(CMD_ROWS, COLS + 2, canvas_bottom + STATUS_ROWS, 0);
    wbkgd(win_cmd, COLOR_PAIR(2));

    canvas_init();
    ui_refresh();
}

/* ================================================================== */
/*  menu_cleanup                                                       */
/* ================================================================== */
void menu_cleanup(void)
{
    if (win_cmd)    { delwin(win_cmd);    win_cmd    = NULL; }
    if (win_status) { delwin(win_status); win_status = NULL; }
    if (win_canvas) { delwin(win_canvas); win_canvas = NULL; }
    if (win_title)  { delwin(win_title);  win_title  = NULL; }
    endwin();
}

/* ================================================================== */
/*  ui_refresh  —  redraw everything                                  */
/* ================================================================== */
static void ui_refresh(void)
{
    /* Title */
    werase(win_title);
    mvwprintw(win_title, 0, 1,
        "  2D ASCII Graphics Editor   "
        "[* and _ chars]   Objects: %d", obj_count);
    wrefresh(win_title);

    /* Canvas box + content */
    werase(win_canvas);
    box(win_canvas, 0, 0);
    canvas_display(win_canvas, 1, 1);
    wrefresh(win_canvas);

    /* Status */
    wrefresh(win_status);

    /* Command area */
    wrefresh(win_cmd);
}

/* ================================================================== */
/*  status_msg                                                         */
/* ================================================================== */
static void status_msg(const char *msg)
{
    werase(win_status);
    mvwprintw(win_status, 0, 1, "%s", msg);
    wrefresh(win_status);
}

/* ================================================================== */
/*  prompt_int  — read an integer from win_cmd                        */
/*  Returns 1 on success, 0 on ESC / empty input                      */
/* ================================================================== */
static int prompt_int(const char *label, int *out)
{
    char buf[32] = {0};
    int  pos     = 0;

    echo();
    curs_set(1);
    wprintw(win_cmd, "  %s: ", label);
    wrefresh(win_cmd);

    int ch;
    while ((ch = wgetch(win_cmd)) != '\n') {
        if (ch == 27) {          /* ESC */
            noecho(); curs_set(0);
            return 0;
        }
        if ((ch == KEY_BACKSPACE || ch == 127) && pos > 0) {
            buf[--pos] = '\0';
            /* Move back, erase, re-print */
            int r, c;
            getyx(win_cmd, r, c);
            (void)r;
            mvwaddch(win_cmd, getcury(win_cmd), c - 1, ' ');
            wmove(win_cmd, getcury(win_cmd), c - 1);
            wrefresh(win_cmd);
        } else if ((pos < 30 && (ch >= '0' && ch <= '9')) ||
                   (pos == 0 && ch == '-')) {
            buf[pos++] = (char)ch;
        }
    }
    noecho();
    curs_set(0);

    if (pos == 0) return 0;
    *out = atoi(buf);
    return 1;
}

/* ================================================================== */
/*  prompt_char — read a single character (fill char)                 */
/* ================================================================== */
static int prompt_char(const char *label, char *out)
{
    echo();
    curs_set(1);
    wprintw(win_cmd, "  %s [* or _]: ", label);
    wrefresh(win_cmd);

    int ch = wgetch(win_cmd);
    noecho();
    curs_set(0);

    if (ch == 27) return 0;
    if (ch != '*' && ch != '_') ch = '*';
    *out = (char)ch;
    wprintw(win_cmd, "%c\n", *out);
    wrefresh(win_cmd);
    return 1;
}

/* ================================================================== */
/*  draw_main_menu                                                     */
/* ================================================================== */
#define MENU_ITEMS 6
static const char *menu_labels[MENU_ITEMS] = {
    "1. Add Shape",
    "2. Delete Shape",
    "3. Modify Shape",
    "4. List Objects",
    "5. Clear Canvas",
    "6. Quit"
};

static void draw_main_menu(int selected)
{
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " MENU ");

    for (int i = 0; i < MENU_ITEMS; i++) {
        if (i == selected) {
            wattron(win_cmd, COLOR_PAIR(4) | A_BOLD | A_REVERSE);
            mvwprintw(win_cmd, 1 + i, 4, "  %-30s", menu_labels[i]);
            wattroff(win_cmd, COLOR_PAIR(4) | A_BOLD | A_REVERSE);
        } else {
            mvwprintw(win_cmd, 1 + i, 4, "  %-30s", menu_labels[i]);
        }
    }
    mvwprintw(win_cmd, CMD_ROWS - 2, 2,
        "Arrow keys / 1-6 to select, Enter to confirm");
    wrefresh(win_cmd);
}

/* ================================================================== */
/*  do_add  —  add a new shape                                        */
/* ================================================================== */
static const char *shape_labels[4] = {
    "1. Circle", "2. Rectangle", "3. Line", "4. Triangle"
};

static void do_add(void)
{
    /* Pick shape type */
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " ADD SHAPE ");
    for (int i = 0; i < 4; i++)
        mvwprintw(win_cmd, 2 + i, 4, "%s", shape_labels[i]);
    mvwprintw(win_cmd, CMD_ROWS - 2, 2, "Press 1-4 to select shape type:");
    wrefresh(win_cmd);

    int key = wgetch(win_cmd);
    ShapeType stype;
    switch (key) {
    case '1': stype = CIRCLE;    break;
    case '2': stype = RECTANGLE; break;
    case '3': stype = LINE;      break;
    case '4': stype = TRIANGLE;  break;
    default:
        status_msg("Cancelled.");
        return;
    }

    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " ADD: %s ", shape_name(stype));
    wmove(win_cmd, 2, 2);
    wrefresh(win_cmd);

    Shape s;
    memset(&s, 0, sizeof(s));
    s.type      = stype;
    s.fill_char = '*';

    int ok = 1;

    switch (stype) {
    case CIRCLE:
        mvwprintw(win_cmd, 1, 2,
            "Enter center col (x), row (y), radius");
        wmove(win_cmd, 2, 2); wrefresh(win_cmd);
        ok = prompt_int("Center col (x)", &s.x1) &&
             prompt_int("Center row (y)", &s.y1) &&
             prompt_int("Radius",         &s.radius);
        break;

    case RECTANGLE:
        mvwprintw(win_cmd, 1, 2,
            "Enter top-left col,row then bottom-right col,row");
        wmove(win_cmd, 2, 2); wrefresh(win_cmd);
        ok = prompt_int("Top-left  col (x1)", &s.x1) &&
             prompt_int("Top-left  row (y1)", &s.y1) &&
             prompt_int("Bot-right col (x2)", &s.x2) &&
             prompt_int("Bot-right row (y2)", &s.y2);
        break;

    case LINE:
        mvwprintw(win_cmd, 1, 2,
            "Enter start col,row then end col,row");
        wmove(win_cmd, 2, 2); wrefresh(win_cmd);
        ok = prompt_int("Start col (x1)", &s.x1) &&
             prompt_int("Start row (y1)", &s.y1) &&
             prompt_int("End   col (x2)", &s.x2) &&
             prompt_int("End   row (y2)", &s.y2);
        break;

    case TRIANGLE:
        mvwprintw(win_cmd, 1, 2,
            "Enter three vertices (col, row) each");
        wmove(win_cmd, 2, 2); wrefresh(win_cmd);
        ok = prompt_int("V1 col (x1)", &s.x1) &&
             prompt_int("V1 row (y1)", &s.y1) &&
             prompt_int("V2 col (x2)", &s.x2) &&
             prompt_int("V2 row (y2)", &s.y2) &&
             prompt_int("V3 col (x3)", &s.x3) &&
             prompt_int("V3 row (y3)", &s.y3);
        break;
    }

    if (!ok) { status_msg("Add cancelled."); return; }

    /* Choose fill character */
    if (!prompt_char("Fill char", &s.fill_char)) {
        status_msg("Add cancelled.");
        return;
    }

    int id = obj_add(&s);
    if (id < 0) {
        status_msg("ERROR: object list full!");
        return;
    }

    /* Draw onto canvas */
    switch (stype) {
    case CIRCLE:
        draw_circle(s.y1, s.x1, s.radius, s.fill_char);
        break;
    case RECTANGLE:
        draw_rectangle(s.y1, s.x1, s.y2, s.x2, s.fill_char);
        break;
    case LINE:
        draw_line(s.y1, s.x1, s.y2, s.x2, s.fill_char);
        break;
    case TRIANGLE:
        draw_triangle(s.y1, s.x1, s.y2, s.x2, s.y3, s.x3, s.fill_char);
        break;
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "Added %s with ID %d", shape_name(stype), id);
    status_msg(msg);
    ui_refresh();
}

/* ================================================================== */
/*  do_delete                                                          */
/* ================================================================== */
static void do_delete(void)
{
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " DELETE SHAPE ");
    wmove(win_cmd, 2, 2);
    wrefresh(win_cmd);

    int id = 0;
    if (!prompt_int("Enter object ID to delete", &id)) {
        status_msg("Delete cancelled.");
        return;
    }

    if (obj_delete(id) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Deleted object ID %d", id);
        status_msg(msg);
    } else {
        status_msg("ERROR: ID not found or already deleted.");
    }
    ui_refresh();
}

/* ================================================================== */
/*  do_modify                                                          */
/* ================================================================== */
static void do_modify(void)
{
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " MODIFY SHAPE ");
    wmove(win_cmd, 2, 2);
    wrefresh(win_cmd);

    int id = 0;
    if (!prompt_int("Enter object ID to modify", &id)) {
        status_msg("Modify cancelled.");
        return;
    }

    Shape *orig = obj_find(id);
    if (!orig) {
        status_msg("ERROR: ID not found.");
        return;
    }

    /* Start with a copy of the original */
    Shape updated = *orig;

    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " MODIFY ID %d: %s ", id, shape_name(orig->type));
    mvwprintw(win_cmd, 1, 2, "Enter new values (leave blank for same type):");
    wmove(win_cmd, 2, 2);
    wrefresh(win_cmd);

    int ok = 1;

    switch (orig->type) {
    case CIRCLE:
        ok = prompt_int("New center col (x)", &updated.x1) &&
             prompt_int("New center row (y)", &updated.y1) &&
             prompt_int("New radius",         &updated.radius);
        break;
    case RECTANGLE:
        ok = prompt_int("New x1", &updated.x1) &&
             prompt_int("New y1", &updated.y1) &&
             prompt_int("New x2", &updated.x2) &&
             prompt_int("New y2", &updated.y2);
        break;
    case LINE:
        ok = prompt_int("New x1", &updated.x1) &&
             prompt_int("New y1", &updated.y1) &&
             prompt_int("New x2", &updated.x2) &&
             prompt_int("New y2", &updated.y2);
        break;
    case TRIANGLE:
        ok = prompt_int("New x1", &updated.x1) &&
             prompt_int("New y1", &updated.y1) &&
             prompt_int("New x2", &updated.x2) &&
             prompt_int("New y2", &updated.y2) &&
             prompt_int("New x3", &updated.x3) &&
             prompt_int("New y3", &updated.y3);
        break;
    }

    if (!ok) { status_msg("Modify cancelled."); return; }

    if (!prompt_char("New fill char", &updated.fill_char)) {
        status_msg("Modify cancelled.");
        return;
    }

    if (obj_modify(id, &updated) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Modified object ID %d", id);
        status_msg(msg);
    } else {
        status_msg("ERROR: modify failed.");
    }
    ui_refresh();
}

/* ================================================================== */
/*  do_list                                                            */
/* ================================================================== */
static void do_list(void)
{
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " OBJECT LIST ");

    int row = 1;
    int shown = 0;
    for (int i = 0; i < obj_count && row < CMD_ROWS - 2; i++) {
        Shape *s = &objects[i];
        if (!s->alive) continue;

        shown++;
        switch (s->type) {
        case CIRCLE:
            mvwprintw(win_cmd, row++, 2,
                "ID %2d  CIRCLE     cx=%3d cy=%3d r=%2d  char='%c'",
                s->id, s->x1, s->y1, s->radius, s->fill_char);
            break;
        case RECTANGLE:
            mvwprintw(win_cmd, row++, 2,
                "ID %2d  RECT       (%3d,%3d)->(%3d,%3d)  char='%c'",
                s->id, s->x1, s->y1, s->x2, s->y2, s->fill_char);
            break;
        case LINE:
            mvwprintw(win_cmd, row++, 2,
                "ID %2d  LINE       (%3d,%3d)->(%3d,%3d)  char='%c'",
                s->id, s->x1, s->y1, s->x2, s->y2, s->fill_char);
            break;
        case TRIANGLE:
            mvwprintw(win_cmd, row++, 2,
                "ID %2d  TRIANGLE   (%d,%d) (%d,%d) (%d,%d)  char='%c'",
                s->id, s->x1, s->y1, s->x2, s->y2,
                s->x3, s->y3, s->fill_char);
            break;
        }
    }

    if (shown == 0)
        mvwprintw(win_cmd, 2, 4, "(no objects on canvas)");

    mvwprintw(win_cmd, CMD_ROWS - 2, 2, "Press any key to continue...");
    wrefresh(win_cmd);
    wgetch(win_cmd);
}

/* ================================================================== */
/*  menu_run                                                           */
/* ================================================================== */
void menu_run(void)
{
    int selected = 0;

    status_msg("Use arrow keys or number keys. Enter = select.");
    draw_main_menu(selected);

    while (1) {
        int key = wgetch(win_cmd);

        switch (key) {
        /* Arrow navigation */
        case KEY_UP:
            selected = (selected - 1 + MENU_ITEMS) % MENU_ITEMS;
            break;
        case KEY_DOWN:
            selected = (selected + 1) % MENU_ITEMS;
            break;

        /* Number shortcuts */
        case '1': selected = 0; goto execute;
        case '2': selected = 1; goto execute;
        case '3': selected = 2; goto execute;
        case '4': selected = 3; goto execute;
        case '5': selected = 4; goto execute;
        case '6': case 'q': case 'Q':
            return;   /* Quit */

        case '\n': case KEY_ENTER:
execute:
            switch (selected) {
            case 0: do_add();    break;
            case 1: do_delete(); break;
            case 2: do_modify(); break;
            case 3: do_list();   break;
            case 4:
                canvas_init();
                /* Mark all objects as deleted */
                for (int i = 0; i < obj_count; i++)
                    objects[i].alive = 0;
                status_msg("Canvas cleared.");
                ui_refresh();
                break;
            case 5: return;    /* Quit */
            }
            break;
        }

        draw_main_menu(selected);
    }
}
