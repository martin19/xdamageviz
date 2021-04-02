#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/shape.h>
#include <cairo/cairo-xlib.h>
#include <stdlib.h>
#include <options.h>
#include "debug.h"

int get_visual_for_depth(Display* display, int screen, int depth, Visual **visual) {
  /* code from gtk project, gdk_screen_get_rgba_visual */
  XVisualInfo visual_template;
  XVisualInfo *visual_list;
  int nxvisuals = 0, i;

  visual_template.screen = screen;
  visual_list =
      XGetVisualInfo(display, VisualScreenMask, &visual_template, &nxvisuals);
  for (i = 0; i < nxvisuals; i++) {
    if (visual_list[i].depth == depth && (visual_list[i].red_mask == 0xff0000 &&
                                       visual_list[i].green_mask == 0x00ff00 &&
                                       visual_list[i].blue_mask == 0x0000ff)) {
      *visual = visual_list[i].visual;
      debug_print("Found ARGB Visual\n");
      XFree(visual_list);
      return 1;
    }
  }
  // no argb visual available
  fprintf(stderr, "No ARGB Visual found");
  XFree(visual_list);
  return 0;
}

void init_x11(Options *options, Display **_display, cairo_t **_ctx, Window *_window, int *_damage_event) {
  Visual *vis;
  int target_width, target_height;
  int root_width, root_height;

  XInitThreads();

  //open display
  Display *display = XOpenDisplay(options->display);
  if(display == NULL) {
    fprintf(stderr, "Cannot open display %s.\n", options->display);
    exit(1);
  }

  //create a window
  int screen = DefaultScreen(display);
  Window root = DefaultRootWindow(display);
  XWindowAttributes attributes = {0};
  XGetWindowAttributes(display, root, &attributes);
  root_width = attributes.width;
  root_height = attributes.height;

  target_width = options->width == 0 ? root_width : options->width;
  target_height = options->height == 0 ? root_height : options->height;

  XVisualInfo vinfo;
  XMatchVisualInfo(display, DefaultScreen(display), options->depth, TrueColor, &vinfo);
  XSetWindowAttributes attr;
  attr.colormap = XCreateColormap(display, DefaultRootWindow(display), vinfo.visual, AllocNone);
  attr.border_pixel = 0;
  attr.background_pixel = 0;
  Window window = XCreateWindow(display, DefaultRootWindow(display), 0, 0, target_width, target_height, 0, vinfo.depth,
                                InputOutput, vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel, &attr);

  //remove decoration from window
  if(options->remove_decoration_flag) {
    Atom window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    long value = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(display, window, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *) &value, 1);
  }

  //make window visible
  XMapWindow(display, window);

  //display window above all other windows
  Atom window_layer = XInternAtom(display, "_WIN_LAYER", False);
  u_int32_t value = 100000000; //?? how to get window on top - forever
  XChangeProperty(display, window, window_layer, XA_CARDINAL, 32, PropModeAppend, (unsigned char *) &value, 1);

  //prevent window from capturing mouse input
  int major_version;
  int minor_version;
  if (XShapeQueryVersion(display, &major_version, &minor_version) == 0) {
    fprintf(stderr, "x11 shape extension is not supported.\n");
    exit(1);
  } else {
    XShapeCombineRectangles(display, window, ShapeInput, 0, 0, NULL, 0, ShapeSet, Unsorted);
  }

  // Sets an empty WM_PROTOCOLS property
  XSetWMProtocols(display, window, NULL, 0);

  // Make window closable
  Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols(display, window, &wm_delete_window, 1);

  //select window to receive events of type StructureNotify
//  XSelectInput(display, window, StructureNotifyMask);

  //XDamage extension initialization
  int damage_error, test, damage_event;
  test = XDamageQueryExtension(display, &damage_event, &damage_error);
  if(!test) {
    fprintf(stderr, "No xdamage extension available.\n");
  }
  XDamageCreate(display, root, XDamageReportRawRectangles);

  //create a cairo surface
  get_visual_for_depth(display, screen, options->depth, &vis);
  cairo_surface_t *sfc = cairo_xlib_surface_create(display, window, vis, target_width, target_height);
  cairo_xlib_surface_set_size(sfc, target_width, target_height);
  cairo_t *ctx = cairo_create(sfc);

  //set scaling
  cairo_matrix_t scale;
  cairo_matrix_init_scale(&scale, (double)target_width/(double)root_width, (double)target_height/(double)root_height);
  cairo_set_matrix(ctx, &scale);

  *_display = display;
  *_window = window;
  *_ctx = ctx;
  *_damage_event = damage_event;
}

void free_x11(Display *display, Window window) {
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}