#ifndef X11DAMAGEVIZ_OPTIONS_C_H
#define X11DAMAGEVIZ_OPTIONS_C_H

typedef struct Options {
    char *display;
    int depth;
    int remove_decoration_flag;
    int width;
    int height;
    int fps;
    char *color_fg;
    char *color_bg;
} Options;

void getOptions(int argc, char *argv[], Options *options);

#endif //X11DAMAGEVIZ_OPTIONS_C_H
