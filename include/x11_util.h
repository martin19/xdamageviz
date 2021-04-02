#ifndef X11DAMAGEVIZ_X11_UTIL_H
#define X11DAMAGEVIZ_X11_UTIL_H

void init_x11(Options *options, Display **display, cairo_t **ctx, Window *window, int *damage);
void free_x11(Display *display, Window window);

#endif //X11DAMAGEVIZ_X11_UTIL_H
