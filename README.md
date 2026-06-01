# 2D ASCII Graphics Editor

A terminal-based 2D graphics editor written in **C** using `*` and `_` characters
to render shapes on a character canvas. Built with **ncurses** for the interactive UI.

## Features

- **Draw shapes**: Circle, Rectangle, Line, Triangle
- **Manage objects**: Add, Delete, Modify by unique ID
- **2D canvas**: 40 rows × 120 columns character array
- **ncurses UI**: Arrow key / number key menu, coloured display
- **Algorithms**: Mid-point circle, Bresenham line

## Build

```bash
# macOS — install ncurses if needed (usually pre-installed)
make
```

```bash
# Linux
sudo apt-get install libncurses5-dev   # Debian/Ubuntu
make
```

## Run

```bash
make run
# or
./graphics_editor
```

## Controls

| Key | Action |
|-----|--------|
| `↑ / ↓` | Navigate menu |
| `1`–`6` | Select menu item directly |
| `Enter` | Confirm selection |
| `ESC` | Cancel current input |
| `q` / `Q` / `6` | Quit |

## Menu Options

1. **Add Shape** — choose Circle / Rectangle / Line / Triangle, enter parameters
2. **Delete Shape** — enter object ID
3. **Modify Shape** — enter object ID then new parameters
4. **List Objects** — show all live objects with their IDs and params
5. **Clear Canvas** — erase everything
6. **Quit**

## Shape Parameters

| Shape | Parameters |
|-------|------------|
| Circle | Center (col, row), Radius |
| Rectangle | Top-left (col, row), Bottom-right (col, row) |
| Line | Start (col, row), End (col, row) |
| Triangle | Vertex 1, Vertex 2, Vertex 3 — each as (col, row) |

All shapes ask for a fill character: `*` or `_`.

## File Structure

```
2d-graphics-editor/
├── Makefile
├── README.md
└── src/
    ├── main.c      — entry point
    ├── canvas.c/h  — 2D char array, display
    ├── shapes.c/h  — circle, rectangle, line, triangle
    ├── objects.c/h — add/delete/modify/find
    └── menu.c/h    — ncurses menus and dialogs
```
