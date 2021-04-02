#include <stdlib.h>
#include <pthread.h>
#include <cairo/cairo-xlib.h>
#include <unistd.h>
#include <options.h>
#include <stdio.h>
#include "debug.h"
#include "box_renderer.h"

typedef struct COLOR_RGBA {
  double r;
  double g;
  double b;
  double a;
} COLOR_RGBA;

static const double line_width = 2;
static const int decay_us = 500000; //500ms
static int delta_us = 33333; //33.3ms
static struct COLOR_RGBA color_fg;
static struct COLOR_RGBA color_bg;

//boxes list
static struct Box *first = NULL;
static struct Box *last = NULL;

//thread context
static pthread_t thread_id;
struct ThreadContext {
    Display *display;
    cairo_t *ctx;
} threadContext;


void add_box(int x, int y, int w, int h) {
  Box *new_box = calloc(1, sizeof(struct Box));
  new_box->remaining_us = decay_us;
  new_box->x = x;
  new_box->y = y;
  new_box->w = w;
  new_box->h = h;

  if(first == NULL) {
    first = new_box;
    last = new_box;
    new_box->next = NULL;
    new_box->prev = NULL;
  } else {
    last->next = new_box;
    new_box->prev = last;
    new_box->next = NULL;
    last = new_box;
  }
}

void remove_box(Box *box) {
  if(box->prev == NULL && box->next == NULL) {
    first = NULL;
    last = NULL;
  } else if(box->prev == NULL) {
    box->next->prev = NULL;
    first = box->next;
  } else if(box->next == NULL) {
    box->prev->next =NULL;
    last = box->prev;
  } else {
    box->prev->next = box->next;
    box->next->prev = box->prev;
  }
  free(box);
}

void free_boxes() {
  while(last != NULL) {
    remove_box(last);
  }
}

void advance_time(int us) {
  if(first == NULL) return;
  Box *cur = first;
  do {
    cur->remaining_us -= us;
    if(cur->remaining_us <= 0) {
      Box *next = cur->next;
      remove_box(cur);
      cur = next;
    } else {
      cur = cur->next;
    }
  } while(cur != NULL);
}

void render_boxes(cairo_t *ctx, Display *display) {

  if(first == NULL) {
    cairo_set_operator (ctx, CAIRO_OPERATOR_CLEAR);
    cairo_paint(ctx);
    cairo_set_source_rgba(ctx, color_bg.r, color_bg.g, color_bg.b, color_bg.a);
    cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
    cairo_paint(ctx);
    XFlush(display);
    return;
  }

  BoxPtr cur = first;
  cairo_set_operator (ctx, CAIRO_OPERATOR_CLEAR);
  cairo_paint(ctx);
  cairo_set_source_rgba(ctx, color_bg.r, color_bg.g, color_bg.b, color_bg.a);
  cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
  cairo_paint(ctx);

  cairo_set_operator (ctx, CAIRO_OPERATOR_OVER);
  cairo_set_line_width(ctx, line_width);

  do {
    double alpha_base = (double)cur->remaining_us / (double)decay_us;
    double alpha_stroke =  alpha_base * color_fg.a;
    double alpha_fill = alpha_base * (color_fg.a * 0.8); //fill is 80% of foreground color
    cairo_set_source_rgba(ctx, color_fg.r, color_fg.g, color_fg.b, alpha_fill);
    cairo_rectangle(ctx, cur->x,cur->y,cur->w,cur->h);
    cairo_fill_preserve(ctx);
    cairo_set_source_rgba(ctx, color_fg.r, color_fg.g, color_fg.b, alpha_stroke);
    cairo_stroke(ctx);
    cur = cur->next;
  } while(cur != NULL);


  XFlush(display);
  //XSync(display, False);
}

_Noreturn void *render_loop() {
  while(1) {
    advance_time(delta_us);
    render_boxes(threadContext.ctx, threadContext.display);
    usleep(delta_us);
  }
}

void parseColor(char* s, COLOR_RGBA *colorArgb) {
  int r, g, b, a;
  if(sscanf(s, "#%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
    colorArgb->r = (double)r/255.0;
    colorArgb->g = (double)g/255.0;
    colorArgb->b = (double)b/255.0;
    colorArgb->a = (double)a/255.0;
  }
}

void start_box_renderer(Options *options, cairo_t *ctx, Display *display) {
  delta_us = options->fps ? (1000000 / options->fps) : 33333*2; //15 fps
  if(options->color_fg) parseColor(options->color_fg, &color_fg);
  if(options->color_bg) parseColor(options->color_bg, &color_bg);

  threadContext.ctx = ctx;
  threadContext.display = display;
  pthread_create(&thread_id, NULL, &render_loop, NULL);
}

void stop_box_renderer() {
  pthread_cancel(thread_id);
  free_boxes();
}