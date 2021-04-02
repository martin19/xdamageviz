#ifndef X11DAMAGEVIZ_DEBUG_H
#define X11DAMAGEVIZ_DEBUG_H

#define DEBUG 0
#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, ##__VA_ARGS__); } while (0)

#endif //X11DAMAGEVIZ_DEBUG_H
