#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct grid_data {
   unsigned long * data;
   int length;
   int width;
   int height;
   int base; /* 0 base or above */
};
typedef struct grid_data GRID_DATA;

/**
 * Creates a new GRID_DATA of length, width and height 
 */
GRID_DATA * grid_create(int base, int cols, int rows, int height);
void grid_destroy(GRID_DATA * grid);

/**
 * Updates the coords based on the base value
 */
void grid_update_coords(GRID_DATA * grid, int * col, int * row, int * height);

/**
 * True if its valid coords, false if not
 */
bool grid_valid_coors(GRID_DATA * grid, int col, int row, int height);

/**
 * Set the data at pos x,y,z
 */
void   grid_set_pos (GRID_DATA * grid, int col, int row, int height, void * item);

/**
 * Get the data at pos x,y,z
 */
void * grid_get_pos (GRID_DATA * grid, int col, int row, int height);

/** 
 * Find object in the grid, return the position in raw format
 * call grid_translate to get x,y,z pos
 */
//int    grid_find (GRID_DATA * grid, void * obj);
int    grid_find (GRID_DATA * grid, void * obj, int * col, int * row, int * height);

/**
 * Translate grid position to coordinates
 */
void   grid_translate (GRID_DATA * grid, int origpos, int * x, int * y, int * z);


/* C-style interface for GRID */
typedef struct {
   GRID_DATA * grid;
} GRID;

/* Functions to replace class methods */
GRID* grid_new(int base, int length, int width, int height);
void grid_free(GRID* g);

void grid_set(GRID* g, int col, int row, int height, void* item);
void* grid_get(GRID* g, int col, int row, int height);
int grid_find_obj(GRID* g, void* obj, int* col, int* row, int* height);
bool grid_valid(GRID* g, int x, int y, int z);
void grid_translate_pos(GRID* g, int pos, int* x, int* y, int* z);

int grid_get_width(GRID* g);
int grid_get_length(GRID* g);
int grid_get_height(GRID* g);
int grid_get_base(GRID* g);

void grid_set_width(GRID* g, int width);
void grid_set_length(GRID* g, int length);
void grid_set_height(GRID* g, int height);
void grid_set_base(GRID* g, int base);
