#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/shape.h>
#include <cairo/cairo-xlib.h>
#include <getopt.h>
#include <options.h>

#include "debug.h"
#include "box_renderer.h"
#include "x11_util.h"

sig_atomic_t done = 0;
char default_display[] = ":0.0";
char default_color_fg[] = "#FF00FF88"; //white
char default_color_bg[] = "#00000000"; //transparent black

void signal_handler(int n) {
  if(n == SIGINT) done = 1;
}

int getAbsoluteSurfaceGeometry(Display *display, Window window, cairo_t *ctx,
                               int32_t *_x, int32_t *_y, u_int32_t *_width, u_int32_t *_height) {

  //get coordinates of window (including decoration)
  int x_window, y_window;
  Window root_window = DefaultRootWindow(display);
  Window child;
  XWindowAttributes xwa;
  XTranslateCoordinates(display, window, root_window, 0, 0, &x_window, &y_window, &child );
  XGetWindowAttributes( display, window, &xwa );

  //get coordinates and size of drawable (the cairo surface) relative to window.
  Window root_return;
  int x_drawable, y_drawable;
  unsigned int width, height, border_width, depth;
  cairo_surface_t *surface = cairo_get_target(ctx);
  Drawable drawable = cairo_xlib_surface_get_drawable(surface);
  XGetGeometry(display, drawable, &root_return, &x_drawable, &y_drawable, &width, &height, &border_width, &depth);

  *_x = x_window;
  *_y = y_window;
  *_width = width;
  *_height = height;
}

int main(int argc, char **argv) {
  Display *display;
  Window window;
  cairo_t *ctx;
  int damage_event;

  Options *options = calloc(1, sizeof(struct Options));
  getOptions(argc, argv, options);
  if(!options->depth) options->depth = 32;
  if(!options->display) options->display = default_display;
  if(!options->color_bg) options->color_bg = default_color_bg;
  if(!options->color_fg) options->color_fg = default_color_fg;

  signal(SIGINT, signal_handler);

  init_x11(options, &display, &ctx, &window, &damage_event);
  start_box_renderer(options, ctx, display);

  XDamageNotifyEvent *dev;
  int event_counter = 0;
  int32_t x, y;
  u_int32_t width, height;

  while (!done){
    XEvent ev;
    XNextEvent(display, &ev);

    if(ev.type == ClientMessage) {
      if (ev.xclient.message_type == XInternAtom(display, "WM_PROTOCOLS", 1) &&
          (Atom) ev.xclient.data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", 1))
        done = 1;
    } else if(ev.type == damage_event + XDamageNotify) {
      //XDamageSubtract(display, damage, None, None);
      dev = (XDamageNotifyEvent*)&ev;

      //exclude own window from reporting
      //XGetWindowAttributes(display, window, &xwa);
      //printf("[%4d,%4d,%4d,%4d]\n",xwa.x,xwa.y,xwa.width, xwa.height);

      getAbsoluteSurfaceGeometry(display, window, ctx, &x, &y, &width, &height);

      if(!(dev->area.x == x && dev->area.y == y && dev->area.width == width && dev->area.height == height)) {
        event_counter++;
        debug_print("[%4d,%4d,%4d,%4d] %d\n", dev->area.x, dev->area.y, dev->area.width, dev->area.height, event_counter);
        add_box(dev->area.x, dev->area.y, dev->area.width, dev->area.height);
      }
    }
  }

  stop_box_renderer();
  free_x11(display, window);
}