/*
 * menu.c  -- ncurses UI for the 2-D Graphics Editor
 */

#include "menu.h"
#include "canvas.h"
#include "objects.h"
#include "shapes.h"

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static WINDOW *win_title   = NULL;
static WINDOW *win_canvas  = NULL;
static WINDOW *win_status  = NULL;
static WINDOW *win_cmd     = NULL;

#define TITLE_ROWS   1
#define STATUS_ROWS  1
#define CMD_ROWS     10
#define INPUT_COL    32   /* column where typed digits appear */

static void ui_refresh(void);
static void status_msg(const char *msg);
static int  prompt_int_at(int row, const char *label, int *out);
static int  pick_from_list(const char *title, const char **items,
                           int n, int *chosen);
static void do_add(void);
static void do_delete(void);
static void do_modify(void);
static void do_list(void);
static void draw_main_menu(int selected);

void menu_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN,    COLOR_BLACK);
        init_pair(2, COLOR_WHITE,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_GREEN,   COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    }

    int max_r, max_c;
    getmaxyx(stdscr, max_r, max_c);

    int need_rows = TITLE_ROWS + ROWS + 2 + STATUS_ROWS + CMD_ROWS;
    int need_cols = COLS + 2;
    if (max_r < need_rows || max_c < need_cols) {
        endwin();
        printf("ERROR: Terminal too small!\n");
        printf("  Your terminal : %d rows x %d cols\n", max_r, max_c);
        printf("  Minimum needed: %d rows x %d cols\n", need_rows, need_cols);
        exit(1);
    }

    win_title  = newwin(TITLE_ROWS, COLS + 2, 0, 0);
    wbkgd(win_title, COLOR_PAIR(1) | A_BOLD);

    win_canvas = newwin(ROWS + 2, COLS + 2, TITLE_ROWS, 0);
    wbkgd(win_canvas, COLOR_PAIR(2));

    int canvas_bottom = TITLE_ROWS + ROWS + 2;
    win_status = newwin(STATUS_ROWS, COLS + 2, canvas_bottom, 0);
    wbkgd(win_status, COLOR_PAIR(3) | A_BOLD);

    win_cmd = newwin(CMD_ROWS, COLS + 2, canvas_bottom + STATUS_ROWS, 0);
    wbkgd(win_cmd, COLOR_PAIR(2));
    keypad(win_cmd, TRUE);

    canvas_init();
    ui_refresh();
}

void menu_cleanup(void)
{
    if (win_cmd)    { delwin(win_cmd);    win_cmd    = NULL; }
    if (win_status) { delwin(win_status); win_status = NULL; }
    if (win_canvas) { delwin(win_canvas); win_canvas = NULL; }
    if (win_title)  { delwin(win_title);  win_title  = NULL; }
    endwin();
}

static void ui_refresh(void)
{
    werase(win_title);
    mvwprintw(win_title, 0, 1,
        "  2D ASCII Graphics Editor   "
        "[* and _ chars]   Objects: %d", obj_count);
    wrefresh(win_title);

    werase(win_canvas);
    box(win_canvas, 0, 0);
    canvas_display(win_canvas, 1, 1);
    wrefresh(win_canvas);

    wrefresh(win_status);
    wrefresh(win_cmd);
}

static void status_msg(const char *msg)
{
    werase(win_status);
    mvwprintw(win_status, 0, 1, "%s", msg);
    wrefresh(win_status);
}

/*
 * prompt_int_at
 * Draws the label at a fixed ROW; manually reads digits using mvwaddch.
 * NO echo() -- we place each character at (row, INPUT_COL+pos) ourselves.
 * Returns 1 on success, 0 on ESC or empty input.
 */
static int prompt_int_at(int row, const char *label, int *out)
{
    char buf[8];
    int  pos = 0;
    memset(buf, 0, sizeof(buf));

    /* Draw label in a fixed 26-char field */
    mvwprintw(win_cmd, row, 2, "  %-26s: ", label);

    /* Place cursor at input start position */
    curs_set(1);
    wmove(win_cmd, row, INPUT_COL);
    wrefresh(win_cmd);

    int ch;
    for (;;) {
        ch = wgetch(win_cmd);

        if (ch == '\n' || ch == KEY_ENTER) break;

        if (ch == 27) {                          /* ESC: cancel */
            curs_set(0);
            return 0;
        }

        if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && pos > 0) {
            pos--;
            buf[pos] = '\0';
            mvwaddch(win_cmd, row, INPUT_COL + pos, ' '); /* erase char */
            wmove(win_cmd,   row, INPUT_COL + pos);       /* move back  */
            wrefresh(win_cmd);
            continue;
        }

        if (pos < 5 && ((ch >= '0' && ch <= '9') || (pos == 0 && ch == '-'))) {
            buf[pos] = (char)ch;
            mvwaddch(win_cmd, row, INPUT_COL + pos, (chtype)ch); /* draw digit */
            pos++;
            wmove(win_cmd, row, INPUT_COL + pos);  /* advance cursor */
            wrefresh(win_cmd);
        }
    }

    curs_set(0);
    if (pos == 0) return 0;

    *out = atoi(buf);

    /* Show confirmed value in green */
    wattron(win_cmd, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win_cmd, row, INPUT_COL, "%-6d", *out);
    wattroff(win_cmd, COLOR_PAIR(4) | A_BOLD);
    wrefresh(win_cmd);

    return 1;
}

/*
 * pick_from_list
 * Shows items[] with arrow-key + number-key navigation.
 * Returns 1 and sets *chosen (0-based) on Enter. 0 on ESC.
 */
static int pick_from_list(const char *title, const char **items,
                          int n, int *chosen)
{
    int sel = (*chosen >= 0 && *chosen < n) ? *chosen : 0;

    for (;;) {
        werase(win_cmd);
        box(win_cmd, 0, 0);
        mvwprintw(win_cmd, 0, 2, " %s ", title);

        for (int i = 0; i < n && i < CMD_ROWS - 3; i++) {
            if (i == sel) {
                wattron(win_cmd, COLOR_PAIR(4) | A_BOLD | A_REVERSE);
                mvwprintw(win_cmd, 1 + i, 3, "  %-44s", items[i]);
                wattroff(win_cmd, COLOR_PAIR(4) | A_BOLD | A_REVERSE);
            } else {
                mvwprintw(win_cmd, 1 + i, 3, "  %-44s", items[i]);
            }
        }
        mvwprintw(win_cmd, CMD_ROWS - 2, 2,
            "Arrow/number to select, Enter=OK, ESC=cancel");
        wrefresh(win_cmd);

        int key = wgetch(win_cmd);
        if (key == 27)                              return 0;
        if (key == KEY_UP)   sel = (sel - 1 + n) % n;
        if (key == KEY_DOWN) sel = (sel + 1) % n;
        if (key == '\n' || key == KEY_ENTER) { *chosen = sel; return 1; }
        if (key >= '1' && key <= '9') {
            int idx = key - '1';
            if (idx < n) { *chosen = idx; return 1; }
        }
    }
}

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

static const char *shape_labels[4] = {
    "1. Circle", "2. Rectangle", "3. Line", "4. Triangle"
};
static const char *fill_labels[2] = {
    "* (asterisk)", "_ (underscore)"
};

static void do_add(void)
{
    int choice = 0;
    if (!pick_from_list("ADD SHAPE - choose type", shape_labels, 4, &choice)) {
        status_msg("Add cancelled.");
        return;
    }
    ShapeType stype = (ShapeType)choice;

    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " ADD: %s - enter coordinates ", shape_name(stype));
    wrefresh(win_cmd);

    Shape s;
    memset(&s, 0, sizeof(s));
    s.type      = stype;
    s.fill_char = '*';

    int ok = 1;
    switch (stype) {
    case CIRCLE:
        ok = prompt_int_at(1, "Center col x (0-75)", &s.x1) &&
             prompt_int_at(2, "Center row y (0-9)",  &s.y1) &&
             prompt_int_at(3, "Radius",              &s.radius);
        break;
    case RECTANGLE:
        ok = prompt_int_at(1, "Top-left  x1 (0-75)", &s.x1) &&
             prompt_int_at(2, "Top-left  y1 (0-9)",  &s.y1) &&
             prompt_int_at(3, "Bot-right x2 (0-75)", &s.x2) &&
             prompt_int_at(4, "Bot-right y2 (0-9)",  &s.y2);
        break;
    case LINE:
        ok = prompt_int_at(1, "Start x1 (0-75)", &s.x1) &&
             prompt_int_at(2, "Start y1 (0-9)",  &s.y1) &&
             prompt_int_at(3, "End   x2 (0-75)", &s.x2) &&
             prompt_int_at(4, "End   y2 (0-9)",  &s.y2);
        break;
    case TRIANGLE:
        ok = prompt_int_at(1, "V1 x1 (0-75)", &s.x1) &&
             prompt_int_at(2, "V1 y1 (0-9)",  &s.y1) &&
             prompt_int_at(3, "V2 x2 (0-75)", &s.x2) &&
             prompt_int_at(4, "V2 y2 (0-9)",  &s.y2) &&
             prompt_int_at(5, "V3 x3 (0-75)", &s.x3) &&
             prompt_int_at(6, "V3 y3 (0-9)",  &s.y3);
        break;
    }
    if (!ok) { status_msg("Add cancelled."); return; }

    int fc = 0;
    if (!pick_from_list("Choose fill character", fill_labels, 2, &fc)) {
        status_msg("Add cancelled.");
        return;
    }
    s.fill_char = (fc == 0) ? '*' : '_';

    int id = obj_add(&s);
    if (id < 0) { status_msg("ERROR: object list full!"); return; }

    switch (stype) {
    case CIRCLE:
        draw_circle(s.y1, s.x1, s.radius, s.fill_char); break;
    case RECTANGLE:
        draw_rectangle(s.y1, s.x1, s.y2, s.x2, s.fill_char); break;
    case LINE:
        draw_line(s.y1, s.x1, s.y2, s.x2, s.fill_char); break;
    case TRIANGLE:
        draw_triangle(s.y1, s.x1, s.y2, s.x2, s.y3, s.x3, s.fill_char); break;
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "Added %s (ID %d) with '%c'",
             shape_name(stype), id, s.fill_char);
    status_msg(msg);
    ui_refresh();
}

static void do_delete(void)
{
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " DELETE SHAPE ");
    wrefresh(win_cmd);

    int id = 0;
    if (!prompt_int_at(1, "Object ID to delete", &id)) {
        status_msg("Delete cancelled.");
        return;
    }
    if (obj_delete(id) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Deleted object ID %d.", id);
        status_msg(msg);
    } else {
        status_msg("ERROR: ID not found or already deleted.");
    }
    ui_refresh();
}

static void do_modify(void)
{
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " MODIFY SHAPE ");
    wrefresh(win_cmd);

    int id = 0;
    if (!prompt_int_at(1, "Object ID to modify", &id)) {
        status_msg("Modify cancelled.");
        return;
    }
    Shape *orig = obj_find(id);
    if (!orig) { status_msg("ERROR: ID not found."); return; }

    Shape updated = *orig;

    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " MODIFY ID %d: %s - new values ",
              id, shape_name(orig->type));
    wrefresh(win_cmd);

    int ok = 1;
    switch (orig->type) {
    case CIRCLE:
        ok = prompt_int_at(1, "New center col x", &updated.x1) &&
             prompt_int_at(2, "New center row y", &updated.y1) &&
             prompt_int_at(3, "New radius",       &updated.radius);
        break;
    case RECTANGLE:
        ok = prompt_int_at(1, "New x1", &updated.x1) &&
             prompt_int_at(2, "New y1", &updated.y1) &&
             prompt_int_at(3, "New x2", &updated.x2) &&
             prompt_int_at(4, "New y2", &updated.y2);
        break;
    case LINE:
        ok = prompt_int_at(1, "New x1", &updated.x1) &&
             prompt_int_at(2, "New y1", &updated.y1) &&
             prompt_int_at(3, "New x2", &updated.x2) &&
             prompt_int_at(4, "New y2", &updated.y2);
        break;
    case TRIANGLE:
        ok = prompt_int_at(1, "New x1", &updated.x1) &&
             prompt_int_at(2, "New y1", &updated.y1) &&
             prompt_int_at(3, "New x2", &updated.x2) &&
             prompt_int_at(4, "New y2", &updated.y2) &&
             prompt_int_at(5, "New x3", &updated.x3) &&
             prompt_int_at(6, "New y3", &updated.y3);
        break;
    }
    if (!ok) { status_msg("Modify cancelled."); return; }

    int fc = (orig->fill_char == '_') ? 1 : 0;
    if (!pick_from_list("Choose fill character", fill_labels, 2, &fc)) {
        status_msg("Modify cancelled.");
        return;
    }
    updated.fill_char = (fc == 0) ? '*' : '_';

    if (obj_modify(id, &updated) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Modified object ID %d.", id);
        status_msg(msg);
    } else {
        status_msg("ERROR: modify failed.");
    }
    ui_refresh();
}

static void do_list(void)
{
    werase(win_cmd);
    box(win_cmd, 0, 0);
    mvwprintw(win_cmd, 0, 2, " OBJECT LIST ");

    int row = 1, shown = 0;
    for (int i = 0; i < obj_count && row < CMD_ROWS - 2; i++) {
        Shape *s = &objects[i];
        if (!s->alive) continue;
        shown++;
        switch (s->type) {
        case CIRCLE:
            mvwprintw(win_cmd, row++, 2,
                "ID%2d CIRCLE  cx=%2d cy=%2d r=%2d  '%c'",
                s->id, s->x1, s->y1, s->radius, s->fill_char);
            break;
        case RECTANGLE:
            mvwprintw(win_cmd, row++, 2,
                "ID%2d RECT    (%2d,%2d)->(%2d,%2d)  '%c'",
                s->id, s->x1, s->y1, s->x2, s->y2, s->fill_char);
            break;
        case LINE:
            mvwprintw(win_cmd, row++, 2,
                "ID%2d LINE    (%2d,%2d)->(%2d,%2d)  '%c'",
                s->id, s->x1, s->y1, s->x2, s->y2, s->fill_char);
            break;
        case TRIANGLE:
            mvwprintw(win_cmd, row++, 2,
                "ID%2d TRI     (%2d,%2d) (%2d,%2d) (%2d,%2d)  '%c'",
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

void menu_run(void)
{
    int selected = 0;

    status_msg("Use arrow keys or number keys. Enter = select.");
    draw_main_menu(selected);

    for (;;) {
        int key = wgetch(win_cmd);

        switch (key) {
        case KEY_UP:
            selected = (selected - 1 + MENU_ITEMS) % MENU_ITEMS;
            break;
        case KEY_DOWN:
            selected = (selected + 1) % MENU_ITEMS;
            break;
        case '1': selected = 0; goto execute;
        case '2': selected = 1; goto execute;
        case '3': selected = 2; goto execute;
        case '4': selected = 3; goto execute;
        case '5': selected = 4; goto execute;
        case '6': case 'q': case 'Q': return;
        case '\n': case KEY_ENTER:
execute:
            switch (selected) {
            case 0: do_add();    break;
            case 1: do_delete(); break;
            case 2: do_modify(); break;
            case 3: do_list();   break;
            case 4:
                canvas_init();
                for (int i = 0; i < obj_count; i++)
                    objects[i].alive = 0;
                obj_count = 0;
                status_msg("Canvas cleared.");
                ui_refresh();
                break;
            case 5: return;
            }
            break;
        }

        draw_main_menu(selected);
    }
}
