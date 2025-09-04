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
 * Account system for managing multiple characters per user with shared resources.      *
 ****************************************************************************************/

#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

/* Ensure core project types/macros are available when this header is included
 * directly so the header is self-contained for tools (IDE/intellisense) and
 * translation units that may not include `mud.h` beforehand. */
#include "mud.hpp"

// =============================================================================
// Account System Constants
// =============================================================================

#define ACCOUNT_DIR (char*)"../account/"

#define ACCOUNT_SOUND ACCOUNT_MSP
#define ACCOUNT_MSP   BV00
#define ACCOUNT_MXP   BV01

// =============================================================================
// Forward Declarations
// =============================================================================

typedef struct account_data ACCOUNT_DATA;

// =============================================================================
// Global Variables
// =============================================================================

extern ACCOUNT_DATA *first_account;
extern ACCOUNT_DATA *last_account;

// =============================================================================
// Account Data Structure
// =============================================================================

// =============================================================================
// Account Data Structure
// =============================================================================

struct account_data
{
        ACCOUNT_DATA *prev;
        ACCOUNT_DATA *next;
        struct alias_data *first_alias;
        struct alias_data *last_alias;
        char     *name;
        char     *password;
        char     *character[MAX_CHARACTERS];
        int       rppoints;
        int       rpcurrent;
        int       qpoints;
        int       inuse;    /* To prevent deleting one that is active */
        int       flags;
        struct note_data *comments;
        char     *email;
};

// =============================================================================
// Function Prototypes
// =============================================================================

ACCOUNT_DATA *load_account args((const char *name));
ACCOUNT_DATA *create_account args((void));
void save_account args((ACCOUNT_DATA * account));
bool add_to_account args((ACCOUNT_DATA * account, CHAR_DATA * ch));
bool del_from_account args((ACCOUNT_DATA * account, CHAR_DATA * ch));
void show_account_characters args((DESCRIPTOR_DATA * d));
void free_account args((ACCOUNT_DATA * account));
void fread_account args((ACCOUNT_DATA * account, FILE * fp));

#endif
