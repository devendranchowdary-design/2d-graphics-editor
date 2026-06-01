#include "objects.h"
#include "canvas.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Global object store                                                */
/* ------------------------------------------------------------------ */
Shape objects[MAX_OBJECTS];
int   obj_count = 0;
int   next_id   = 1;

/* ================================================================== */
/*  obj_add                                                            */
/* ================================================================== */
int obj_add(Shape *s)
{
    if (obj_count >= MAX_OBJECTS)
        return -1;

    s->id    = next_id++;
    s->alive = 1;
    objects[obj_count++] = *s;
    return s->id;
}

/* ================================================================== */
/*  obj_delete                                                         */
/* ================================================================== */
int obj_delete(int id)
{
    for (int i = 0; i < obj_count; i++) {
        if (objects[i].id == id && objects[i].alive) {
            objects[i].alive = 0;
            canvas_redraw();   /* Refresh without this shape */
            return 0;
        }
    }
    return -1;   /* Not found */
}

/* ================================================================== */
/*  obj_modify                                                         */
/* ================================================================== */
int obj_modify(int id, Shape *updated)
{
    for (int i = 0; i < obj_count; i++) {
        if (objects[i].id == id && objects[i].alive) {
            updated->id    = id;
            updated->alive = 1;
            objects[i] = *updated;
            canvas_redraw();
            return 0;
        }
    }
    return -1;
}

/* ================================================================== */
/*  obj_find                                                           */
/* ================================================================== */
Shape *obj_find(int id)
{
    for (int i = 0; i < obj_count; i++)
        if (objects[i].id == id && objects[i].alive)
            return &objects[i];
    return NULL;
}

/* ================================================================== */
/*  obj_list_all  — writes to the provided FILE* (or NULL = stdout)   */
/* ================================================================== */
void obj_list_all(void)
{
    /* Called by menu.c; the results are displayed in a popup window  */
    /* (see menu_list_objects in menu.c)                              */
}

/* ================================================================== */
/*  shape_name                                                         */
/* ================================================================== */
const char *shape_name(ShapeType t)
{
    switch (t) {
    case CIRCLE:    return "Circle";
    case RECTANGLE: return "Rectangle";
    case LINE:      return "Line";
    case TRIANGLE:  return "Triangle";
    default:        return "Unknown";
    }
}
