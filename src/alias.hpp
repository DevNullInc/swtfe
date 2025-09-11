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
 * This module is for the creation of aliases to keep commonly string of commands (or    *
 * emotes or whatever) setup to a single new command.                                    *
 *****************************************************************************************
 *         Command alias system header for player shortcuts and custom commands.         *
 *****************************************************************************************/

#ifndef AliasH
#define AliasH

// ============================================================================
// Data Structures
// ============================================================================

typedef struct alias_data AliasData;

struct alias_data
{
        AliasData *next;
        AliasData *prev;
        char *name;        // The alias name/command
        char *cmd;         // The command sequence to execute
};

// ============================================================================
// Function Declarations
// ============================================================================

// Alias lookup and management

// Modern C++23 prototypes and overloads
AliasData *get_alias(CharData *ch, const char *argument);
AliasData *get_alias(CharData *ch, std::string_view argument);
AliasData *get_alias(std::shared_ptr<CharData> ch, std::string_view argument);
void free_alias(AccountData *acct, AliasData *alias);
void free_alias(std::shared_ptr<AccountData> acct, std::shared_ptr<AliasData> alias);
void free_aliases(AccountData *acct);
void free_aliases(std::shared_ptr<AccountData> acct);

// Alias execution and processing
bool check_alias(CharData *ch, const char *command, const char *argument);
bool check_alias(CharData *ch, std::string_view command, std::string_view argument);
bool check_alias(std::shared_ptr<CharData> ch, std::string_view command, std::string_view argument);
bool check_aliases(DescriptorData *d);
bool check_aliases(std::shared_ptr<DescriptorData> d);

// File I/O operations
void fread_alias(AccountData *acct, FILE *fp);
void fread_alias(std::shared_ptr<AccountData> acct, FILE *fp);
void fwrite_alias(AccountData *acct, FILE *fp);
void fwrite_alias(std::shared_ptr<AccountData> acct, FILE *fp);
