#ifndef OBJECTS_H
#define OBJECTS_H

/* ------------------------------------------------------------------ */
/*  Shape types                                                        */
/* ------------------------------------------------------------------ */
typedef enum {
    CIRCLE    = 0,
    RECTANGLE = 1,
    LINE      = 2,
    TRIANGLE  = 3
} ShapeType;

/* ------------------------------------------------------------------ */
/*  A single drawable object                                           */
/*  Coordinates: row (y) and col (x)                                  */
/* ------------------------------------------------------------------ */
typedef struct {
    int       id;           /* Unique ID assigned on insertion       */
    int       alive;        /* 0 = deleted, 1 = live                 */
    ShapeType type;
    char      fill_char;    /* '*' or '_'                            */

    /* General point storage                                          */
    int x1, y1;             /* col, row  — primary point / top-left  */
    int x2, y2;             /* col, row  — secondary / bottom-right  */
    int x3, y3;             /* col, row  — third vertex (triangle)   */
    int radius;             /* circle only                           */
} Shape;

#define MAX_OBJECTS 100

extern Shape objects[MAX_OBJECTS];
extern int   obj_count;      /* Total slots used (including deleted) */
extern int   next_id;        /* Monotonically increasing ID          */

/* ------------------------------------------------------------------ */
/*  Object management API                                              */
/* ------------------------------------------------------------------ */

/* Add a new shape; returns the assigned ID, or -1 if list is full   */
int  obj_add(Shape *s);

/* Delete by ID (marks alive=0, redraws canvas); returns 0 on success */
int  obj_delete(int id);

/* Replace params for an existing ID; returns 0 on success            */
int  obj_modify(int id, Shape *updated);

/* Return pointer to live object with given ID, or NULL               */
Shape *obj_find(int id);

/* Print a summary of all live objects to the status window           */
void obj_list_all(void);

/* Helper: shape type to printable string */
const char *shape_name(ShapeType t);

#endif /* OBJECTS_H */
