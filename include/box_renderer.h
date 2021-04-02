#ifndef X11DAMAGEVIZ_BOX_RENDERER_H
#define X11DAMAGEVIZ_BOX_RENDERER_H

#include <cairo/cairo-xlib.h>

typedef struct Box {
    int remaining_us;
    int x;
    int y;
    int w;
    int h;
    struct Box *next;
    struct Box *prev;
} Box;

typedef struct Box *BoxPtr;

void start_box_renderer(Options *options, cairo_t *c, Display *d);
void add_box(int x, int y, int w, int h);
void stop_box_renderer();

#endif //X11DAMAGEVIZ_BOX_RENDERER_H
