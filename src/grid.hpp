/*****************************************************************************************
 *                      .___________. __    __   _______                                 *
 *                      |           ||  |  |  | |   ____|                                *
 *                      `---|  |----`|  |__|  | |  |__                                   *
 *                          |  |     |   __   | |   __|                                  *
 *                          |  |     |  |  |  | |  |____                                 *
 *                          |__|     |__|  |__| |_______|                                *
 *                                                                                       *
 *                _______  __  .__   __.      ___       __                               *
 *               |   ____||  | |  \ |  |     /   \     |  |                              *
 *               |  |__   |  | |   \|  |    /  ^  \    |  |                              *
 *               |   __|  |  | |  . `  |   /  /_\  \   |  |                              *
 *               |  |     |  | |  |\   |  /  _____  \  |  `----.                         *
 *               |__|     |__| |__| \__| /__/     \__\ |_______|                         *
 *                                                                                       *
 *      _______ .______    __       _______.  ______    _______   _______                *
 *     |   ____||   _  \  |  |     /       | /  __  \  |       \ |   ____|               *
 *     |  |__   |  |_)  | |  |    |   (----`|  |  |  | |  .--.  ||  |__                  *
 *     |   __|  |   ___/  |  |     \   \    |  |  |  | |  |  |  ||   __|                 *
 *     |  |____ |  |      |  | .----)   |   |  `--'  | |  '--'  ||  |____                *
 *     |_______|| _|      |__| |_______/     \______/  |_______/ |_______|               *
 *****************************************************************************************
 *                                                                                       *
 * Star Wars: The Final Episode additions and changes from the Star Wars Reality code    *
 * copyright (c) 2025 /dev/null Industries - StygianRenegade                             *
 *                                                                                       *
 * Star Wars Reality Code Additions and changes from the Smaug Code copyright (c) 1997   *
 * by Sean Cooper                                                                        *
 *                                                                                       *
 * Starwars and Starwars Names copyright(c) Lucas Film Ltd.                              *
 *****************************************************************************************
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                                 Grid Module                                           *
 *****************************************************************************************/
#pragma once
#include <vector>
#include <memory>

struct grid_data {
   std::vector<unsigned long> data;
   int length = 0;
   int width = 0;
   int height = 0;
   int base = 0; // 0 base or above
};
using GRID_DATA = grid_data;


// Creates a new GRID_DATA of length, width and height
std::shared_ptr<GRID_DATA> grid_create(int base, int cols, int rows, int height);
void grid_destroy(std::shared_ptr<GRID_DATA> grid);


// Updates the coords based on the base value
void grid_update_coords(std::shared_ptr<GRID_DATA> grid, int* col, int* row, int* height);


// True if its Valid coords, false if not
bool grid_valid_coors(std::shared_ptr<GRID_DATA> grid, int col, int row, int height);


// Set the data at pos x,y,z
void grid_set_pos(std::shared_ptr<GRID_DATA> grid, int col, int row, int height, void* item);


// Get the data at pos x,y,z
void* grid_get_pos(std::shared_ptr<GRID_DATA> grid, int col, int row, int height);


// Find object in the grid, return the position in raw format
// call grid_translate to get x,y,z pos
int grid_find(std::shared_ptr<GRID_DATA> grid, void* obj, int* col, int* row, int* height);


// Translate grid position to coordinates
void grid_translate(std::shared_ptr<GRID_DATA> grid, int origpos, int* x, int* y, int* z);


// C++ interface for GRID
struct grid_wrapper {
   std::shared_ptr<GRID_DATA> grid;
};
using GRID_WRAPPER = grid_wrapper;

// Functions to replace class methods
std::shared_ptr<GRID_WRAPPER> grid_new(int base, int length, int width, int height);
void grid_free(std::shared_ptr<GRID_WRAPPER> g);

void grid_set(std::shared_ptr<GRID_WRAPPER> g, int col, int row, int height, void* item);
void* grid_get(std::shared_ptr<GRID_WRAPPER> g, int col, int row, int height);
int grid_find_obj(std::shared_ptr<GRID_WRAPPER> g, void* obj, int* col, int* row, int* height);
bool grid_valid(std::shared_ptr<GRID_WRAPPER> g, int x, int y, int z);
void grid_translate_pos(std::shared_ptr<GRID_WRAPPER> g, int pos, int* x, int* y, int* z);

int grid_get_width(std::shared_ptr<GRID_WRAPPER> g);
int grid_get_length(std::shared_ptr<GRID_WRAPPER> g);
int grid_get_height(std::shared_ptr<GRID_WRAPPER> g);
int grid_get_base(std::shared_ptr<GRID_WRAPPER> g);

void grid_set_width(std::shared_ptr<GRID_WRAPPER> g, int width);
void grid_set_length(std::shared_ptr<GRID_WRAPPER> g, int length);
void grid_set_height(std::shared_ptr<GRID_WRAPPER> g, int height);
void grid_set_base(std::shared_ptr<GRID_WRAPPER> g, int base);
