#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "options.h"

static int remove_decoration_flag;
static int show_help_flag;

void printUsage() {
  printf("usage: xdamageviz [options]\n" \
      "options:\n" \
      "-d, --display <display>   the display to use\n"\
      "-w, --width <width>   width of the visualization (default is display width)\n"\
      "-h, --height <height>   height of the visualization (default is display height)\n"\
      "-f, --fps <frames per second>   number of frames per second (default is 30)\n"\
      "-n, --no-decoration    removes decoration from window\n"\
      "-a, --color-fg <foreground color>   sets foreground color of visualization (default is #FFFFFFFF)\n"\
      "-b, --color-bg <background color>   sets background color of visualization (default is #000000FF)\n"\
      "--help,    prints help\n");
}

void abort() {
  printf("Invalid arguments.\n\n");
  printUsage();
  exit(-1);
}

void getOptions(int argc, char *argv[], Options *options) {
  int c;

  while (1) {
    static struct option long_options[] = {
        {"help",          no_argument,       &show_help_flag,         1},
        {"no-decoration", no_argument,       &remove_decoration_flag, 1},
        {"display",       required_argument, 0,                       'd'},
        {"depth",         required_argument, 0,                       'b'},
        {"width",         required_argument, 0,                       'w'},
        {"height",        required_argument, 0,                       'h'},
        {"fps",        required_argument, 0,                       'f'},
        {"color-fg",        required_argument, 0,                       'a'},
        {"color-bg",        required_argument, 0,                       'b'},
        {0, 0,                               0,                       0}
    };

    int option_index = 0;
    c = getopt_long(argc, argv, "d:x:w:h:a:b:", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;

        printf("option %s", long_options[option_index].name);
        if (optarg)
          printf(" with arg %s", optarg);
        printf("\n");
        break;

      case 'd':
        options->display = optarg;
        break;
      case 'x':
        options->depth = atoi(optarg);
        break;
      case 'w':
        options->width = atoi(optarg);
        break;
      case 'h':
        options->height = atoi(optarg);
        break;
      case 'f':
        options->fps = atoi(optarg);
        break;
      case 'a':
        options->color_fg = optarg;
        break;
      case 'b':
        options->color_bg = optarg;
        break;
      default:
        abort();
    }
  }

  options->remove_decoration_flag = remove_decoration_flag;

  if (show_help_flag) {
    printUsage();
    exit(0);
  }
}