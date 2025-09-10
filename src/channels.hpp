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
 *                                SWR OLC Channel module                                 *
 ****************************************************************************************/


#pragma once
#include <string>
#include <string_view>
#include <memory>

struct channel_data;
using CHANNEL_DATA = channel_data;
extern CHANNEL_DATA *first_channel;
extern CHANNEL_DATA *last_channel;

#define CHANNEL_FILE SYSTEM_DIR "channel.dat"
struct channel_data
{
        CHANNEL_DATA *next;
        CHANNEL_DATA *prev;
        LOG_DATA *log;
        std::string name;
        std::string actmessage;   // The title to send, "OOC", "[INFO]", etc, accepts colors
        std::string emotemessage;
        std::string socialmessage;
        int logtype;   // logging behavior
        int type;      // IC, OOC, IMM?
        int color;     // Color of TEXT to send, best to reset title at the end with &D
        int range;     // Room/Area/Planet/System/Global/Clan
        int level;     // Minimum level to see this channel
        int logpos;    // Current position in the log (runtime only)
        int cost;      // Does it cost to use this channel?
        bool history;  // Whether or not we are saving a log on this channel
        bool enabled;  // Whether we want people to use this channel at the moment
};

enum class ChannelRange {
        ROOM, AREA, PLANET, SYSTEM, OOC_GLOBAL, CLAN
};

enum class ChannelType {
        IC, IC_COM, OOC
};

CHANNEL_DATA *get_channel(const char *name);
CHANNEL_DATA *get_channel(std::string_view name);
CHANNEL_DATA *get_channel(std::shared_ptr<std::string> name);
bool check_channel(CHAR_DATA *ch, const char *command, const char *argument);
bool check_channel(CHAR_DATA *ch, std::string_view command, std::string_view argument);
bool check_channel(std::shared_ptr<CHAR_DATA> ch, std::string_view command, std::string_view argument);
void add_channel_log(CHAR_DATA *from, const char *message, CHANNEL_DATA *channel);
void add_channel_log(CHAR_DATA *from, std::string_view message, CHANNEL_DATA *channel);
void add_channel_log(std::shared_ptr<CHAR_DATA> from, std::string_view message, std::shared_ptr<CHANNEL_DATA> channel);
int hasname(const char *list, const char *name);
int hasname(std::string_view list, std::string_view name);
void addname(char **list, const char *name);
void addname(std::shared_ptr<std::string> list, std::string_view name);
void removename(char **list, const char *name);
void removename(std::shared_ptr<std::string> list, std::string_view name);
const char *getarg(const char *argument, char *arg, int length);
std::string getarg(std::string_view argument, int length);
void load_channels();
extern char *const pc_displays[MAX_COLORS];
