#include <stdlib.h>  /* For malloc and free */
#include "grid.h"

/* Implementation of C functions to replace the C++ class methods */

GRID_WRAPPER* grid_new(int base, int length, int width, int height) {
    GRID_WRAPPER* g = (GRID_WRAPPER*)malloc(sizeof(GRID_WRAPPER));
    if (g) {
        g->grid = grid_create(base, length, width, height);
    }
    return g;
}

void grid_free(GRID_WRAPPER* g) {
    if (g) {
        if (g->grid) {
            grid_destroy(g->grid);
        }
        free(g);
    }
}

void grid_set(GRID_WRAPPER* g, int col, int row, int height, void* item) {
    if (g && g->grid) {
        grid_set_pos(g->grid, col, row, height, item);
    }
}

void* grid_get(GRID_WRAPPER* g, int col, int row, int height) {
    if (g && g->grid) {
        return grid_get_pos(g->grid, col, row, height);
    }
    return NULL;
}

int grid_find_obj(GRID_WRAPPER* g, void* obj, int* col, int* row, int* height) {
    if (g && g->grid) {
        return grid_find(g->grid, obj, col, row, height);
    }
    return 0;
}

bool grid_valid(GRID_WRAPPER* g, int x, int y, int z) {
    if (g && g->grid) {
        return grid_valid_coors(g->grid, x, y, z);
    }
    return 0;
}

void grid_translate_pos(GRID_WRAPPER* g, int pos, int* x, int* y, int* z) {
    if (g && g->grid) {
        grid_translate(g->grid, pos, x, y, z);
    }
}

int grid_get_width(GRID_WRAPPER* g) {
    if (g && g->grid) {
        return g->grid->width;
    }
    return 0;
}

int grid_get_length(GRID_WRAPPER* g) {
    if (g && g->grid) {
        return g->grid->length;
    }
    return 0;
}

int grid_get_height(GRID_WRAPPER* g) {
    if (g && g->grid) {
        return g->grid->height;
    }
    return 0;
}

int grid_get_base(GRID_WRAPPER* g) {
    if (g && g->grid) {
        return g->grid->base;
    }
    return 0;
}

void grid_set_width(GRID_WRAPPER* g, int width) {
    if (g && g->grid) {
        g->grid->width = width;
    }
}

void grid_set_length(GRID_WRAPPER* g, int length) {
    if (g && g->grid) {
        g->grid->length = length;
    }
}

void grid_set_height(GRID_WRAPPER* g, int height) {
    if (g && g->grid) {
        g->grid->height = height;
    }
}

void grid_set_base(GRID_WRAPPER* g, int base) {
    if (g && g->grid) {
        g->grid->base = base;
    }
}
