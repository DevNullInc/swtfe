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
 *                                SWR Astral body module                                 *
 ****************************************************************************************/
#include "mud.hpp"
#include "astral.hpp"
#include "space2.hpp"
#include "installations.hpp"
#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <cstring>

BodyList bodies;
// TODO: Refactor dock management to use smart pointers and containers
std::list<DockData*> docks;

BodyData *get_body(const char *name, BodyTypes type)
{
        BodyData *body = nullptr;
        BodyList::iterator it;

        if (!name)
                return nullptr;

        for (auto body : bodies) {
                if (!str_prefix(name, body->name())) {
                        if ((type != BodyTypes::ALL && static_cast<BodyTypes>(body->type()) == type) || type == BodyTypes::ALL)
                                return body;
                }
        }

        return nullptr;
}

// Renamed to avoid overload ambiguity
BodyData *get_body_all(const char *name)
{
        return get_body(name, BodyTypes::ALL);
}

RoomIndexData *BodyData::get_rand_room(int bit, bool include)
{
        int count, areacount, roomcount, roomcountfind;
        AreaData *area = nullptr;
        RoomIndexData *room = nullptr;

                count = areacount = roomcount = roomcountfind = 0;
                if (!this->_areas.empty()) {
                        count = static_cast<int>(this->_areas.size());
                        areacount = number_range(1, count);
                        auto it = this->_areas.begin();
                        std::advance(it, areacount - 1);
                        area = *it;
                        if (!area || !area->filename)
                                return nullptr;
                        for (roomcountfind = area->low_r_vnum; roomcountfind < area->hi_r_vnum; roomcountfind++) {
                                roomcount = number_range(area->low_r_vnum, area->hi_r_vnum);
                                room = get_room_index(roomcount);
                                if (room) {
                                        if ((include && xIS_SET(room->RoomFlags, bit)) || (!include && !xIS_SET(room->RoomFlags, bit)))
                                                return room;
                                }
                        }
                }
                return nullptr;
}

int get_rflag(char *flag);
CMDF do_testbody(CharData * ch, char *argument)
{
        char arg1[MaxInputLength];
        char arg2[MaxInputLength];
        int value;
        BodyData *body = nullptr;
        RoomIndexData *room = nullptr;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        value = atoi(argument);
        body = get_body_all(arg1);
        // Note: arg2/flag processing removed as it was unused
        // Note: arg2/flag processing removed as it was unused

        if (!body)
        {
                send_to_char("Cannot find body", ch);
                return;
        }

        room = body->get_rand_room(ROOM_INDOORS, value);
        if (room)
                ch_printf(ch, const_cast<char*>("Room name: %s\n\rRoom VNUM: %d\n\r"),
                          room->name, room->vnum);
        if (!room)
                send_to_char("The pointer returned NULL", ch);
}

BodyData::BodyData() : _filename{}, _gravity{0}, _name{}, _type{0}, 
                         _xpos{0}, _ypos{0}, _zpos{0}, _orbitcount{0},
                         _xmove{0}, _ymove{0}, _zmove{0},
                         _centerx{0}, _centery{0}, _centerz{0},
                         _planet{nullptr}, _starsystem{nullptr}
{
        // Modern initialization with member initializer list
}

BodyData::~BodyData()
{
        PlanetData *planet;
        AreaData *tarea;
        DockData *dock, *next_dock;

        this->starsystem(nullptr);

        for (planet = first_planet; planet != nullptr; planet = planet->next)
                if (planet->body == this)
                        planet->body = nullptr;

        for (tarea = first_area; tarea; tarea = tarea->next)
                if (tarea->body == this)
                        tarea->body = nullptr;

        this->_areas.clear();

        for (dock = first_dock; dock; dock = next_dock)
	    {
   			    next_dock = dock->next;
                if (dock->body == this)
                        free_dock(dock);
		}

        // std::string members automatically handle their own memory cleanup
}

char     *BodyData::get_direction(ShipData * ship)
{
        static char buf[11];

        buf[0] = ' ';
        if (static_cast<float>(this->xpos()) > ship->vx)
                buf[1] = 'E';
        else if (static_cast<float>(this->xpos()) < ship->vx)
                buf[1] = 'W';
        else
                buf[1] = ' ';
        buf[2] = ' ';
        buf[3] = ' ';
        if (static_cast<float>(this->ypos()) > ship->vy)
                buf[4] = 'N';
        else if (static_cast<float>(this->ypos()) < ship->vy)
                buf[4] = 'S';
        else
                buf[4] = ' ';
        buf[5] = ' ';
        buf[6] = ' ';
        if (static_cast<float>(this->zpos()) > ship->vz)
                buf[7] = 'U';
        else if (static_cast<float>(this->zpos()) < ship->vz)
                buf[7] = 'D';
        else
                buf[7] = ' ';
        buf[8] = ' ';
        buf[9] = '\0';
        return buf;
}

int BodyData::distance(ShipData * ship)
{
        return static_cast<int>(sqrt(pow(static_cast<int>(ship->vx - static_cast<float>(this->xpos())), 2) +
                          pow(static_cast<int>(ship->vy - static_cast<float>(this->ypos())),
                              2) + pow(static_cast<int>(ship->vz - static_cast<float>(this->zpos())), 2)));
}

int BodyData::distance(BodyData * pbody)
{
        return static_cast<int>(sqrt(pow(static_cast<int>(pbody->xpos() - this->xpos()), 2) +
                          pow(static_cast<int>(pbody->ypos() - this->ypos()),
                              2) + pow(static_cast<int>(pbody->zpos() - this->zpos()),
                                       2)));
}

int BodyData::hyperdistance(ShipData * ship)
{
        return static_cast<int>(sqrt(pow(static_cast<int>(ship->jx - static_cast<float>(this->xpos())), 2) +
                          pow(static_cast<int>(ship->jy - static_cast<float>(this->ypos())),
                              2) + pow(static_cast<int>(ship->jz - static_cast<float>(this->zpos())), 2)));
}


void BodyData::remove_area(AreaData * area)
{
                auto it = std::find(this->_areas.begin(), this->_areas.end(), area);
                if (it != this->_areas.end()) {
                        this->_areas.erase(it);
                        area->body = nullptr;
                }
}

/* These should be in respected classes */
void BodyData::add_area(AreaData * area)
{
                if (area->body) {
                        area->body->remove_area(area);
                }
                area->body = this;
                this->_areas.push_back(area);
}

void BodyData::add_dock(DockData * dock)
{
        this->_docks.push_back(dock);
}

void BodyData::remove_dock(DockData * dock)
{
                auto it = std::find(this->_docks.begin(), this->_docks.end(), dock);
                if (it != this->_docks.end()) {
                        this->_docks.erase(it);
                }
}

void BodyData::starsystem(SpaceData * starsystem)
{
                if (this->_starsystem != nullptr) {
                        auto it = std::find(this->_starsystem->bodies.begin(), this->_starsystem->bodies.end(), this);
                        if (it != this->_starsystem->bodies.end()) {
                                this->_starsystem->bodies.erase(it);
                        }
                        this->_starsystem = nullptr;
                }
                if (starsystem != nullptr) {
                        starsystem->bodies.push_back(this);
                        this->_starsystem = starsystem;
                }
}

BodyData *BodyData::load(FILE * fp)
{
        const char *word;
        bool fMatch;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        bug("Matching *");
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        if (!str_cmp(word, "Area"))
                        {
                                char aName[MaxStringLength];
                                AreaData *pArea = first_area;

                                snprintf(aName, MSL, "%s",
                                         fread_string_noalloc(fp));
                                for (pArea = first_area; pArea;
                                     pArea = pArea->next)
                                {
                                        if (pArea->filename
                                            && !str_cmp(pArea->filename,
                                                        aName))
                                                this->add_area(pArea);
                                }
                                fMatch = TRUE;
                        }
                        break;
                case 'C':
                        KEY("Centerx", this->_centerx, fread_number(fp));
                        KEY("Centery", this->_centery, fread_number(fp));
                        KEY("Centerz", this->_centerz, fread_number(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (this->_name.empty())
                                        this->_name = "";
                                while (this->_xmove > -10
                                       && this->_xmove < 10)
                                        this->_xmove = number_range(-50, 50);
                                while (this->_ymove > -10
                                       && this->_ymove < 10)
                                        this->_ymove = number_range(-50, 50);
                                while (this->_zmove > -10
                                       && this->_zmove < 10)
                                        this->_zmove = number_range(-50, 50);
                                return this;
                        }
                        break;

                case 'F':
                        // Modern string loading with proper memory management
                        {
                                char* temp_str = fread_string(fp);
                                this->_filename = temp_str ? temp_str : "";
                                if (temp_str) STRFREE(temp_str);
                        }
                        break;

                case 'G':
                        KEY("Gravity", this->_gravity, fread_number(fp));
                        break;

                case 'N':
                        // Modern string loading with proper memory management
                        {
                                char* temp_str = fread_string(fp);
                                this->_name = temp_str ? temp_str : "";
                                if (temp_str) STRFREE(temp_str);
                        }
                        break;

                case 'O':
                        KEY("Orbitcount", this->_orbitcount,
                            fread_number(fp));
                        break;

                case 'P':
                        KEY("Planet", this->_planet,
                            get_planet(fread_string_noalloc(fp)));
                        break;

                case 'S':
                        if (!str_cmp(word, "Starsystem"))
                        {
                                SpaceData *starsystem =
                                        starsystem_from_name
                                        (fread_string_noalloc(fp));
                                if (starsystem)
                                        this->starsystem(starsystem);
                                fMatch = TRUE;
                                break;
                        }
                        break;  // Add missing break for non-matching 'S' case

                case 'T':
                        KEY("Type", this->_type, fread_number(fp));
                        break;

                case 'X':
                        KEY("Xpos", this->_xpos, fread_number(fp));
                        KEY("Xmove", this->_xmove, fread_number(fp));
                        break;

                case 'Y':
                        KEY("Ypos", this->_ypos, fread_number(fp));
                        KEY("Ymove", this->_ymove, fread_number(fp));
                        break;

                case 'Z':
                        KEY("Zpos", this->_zpos, fread_number(fp));
                        KEY("Zmove", this->_zmove, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        bug("Fread_body: no match: %s", word);
                }
        }
        return this;
}

bool load_body_file(char *bodyfile)
{
        char filename[256];
        BodyData *body;
        FILE     *fp;
        bool found;

        //CREATE(body, BodyData, 1);
         /* If it never gets to BODY below, this goes dead, no pointer to it.*/

        found = FALSE;
        snprintf(filename, 256, "%s%s", BODY_DIR, bodyfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

                found = TRUE;
                for (;;)
                {
                        char letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("Load_body_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "BODY"))
                        {
   							    body = new BodyData; /* lets try this here instead */
                                body->load(fp);
                                bodies.push_front(body);
                        }
                        else if (!str_cmp(word, "DOCK"))
                        {
                                DockData *dock;

                                // Modern C++ replacement for CREATE macro
                                dock = static_cast<DockData*>(CALLOC(1, sizeof(DockData)));
                                if (!dock) {
                                        perror("malloc failure");
                                        fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__);
                                        abort();
                                }
                                fread_dock(dock, fp);
                                dock->body = body;
                                LINK(dock, first_dock, last_dock, next, prev);
                                dock->body->add_dock(dock);
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_body_file: bad section: %s.", word);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (!found)
                DELETE(body);

        return found;
}

void BodyData::save()
{
        FILE     *fp;
        char filename[256];
        char buf[MaxStringLength];

        // Note: 'this' can never be null in a member function in C++
        // Removed the unnecessary null check

        if (this->_filename.empty())
        {
                snprintf(buf, MSL, "save_body: %s has no filename",
                         this->_name.c_str());
                bug(buf, 0);
                return;
        }
        snprintf(filename, 256, "%s%s", BODY_DIR, this->_filename.c_str());

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_body: fopen", 0);
                perror(filename);
        }
        else
        {
                AreaData *pArea = NULL;
                DockData *dock = NULL;

                fprintf(fp, "#BODY\n");
                fprintf(fp, "Name         %s~\n", this->_name.c_str());
                fprintf(fp, "Filename     %s~\n", this->_filename.c_str());
                fprintf(fp, "Type         %d\n", this->_type);
                fprintf(fp, "Xpos         %d\n", this->_xpos);
                fprintf(fp, "Ypos         %d\n", this->_ypos);
                fprintf(fp, "Zpos         %d\n", this->_zpos);
                fprintf(fp, "Xmove        %d\n", this->_xmove);
                fprintf(fp, "Ymove        %d\n", this->_ymove);
                fprintf(fp, "Zmove        %d\n", this->_zmove);
                fprintf(fp, "Centerx      %d\n", this->_centerx);
                fprintf(fp, "Centery      %d\n", this->_centery);
                fprintf(fp, "Centerz      %d\n", this->_centerz);
                fprintf(fp, "Gravity      %d\n", this->_gravity);
                fprintf(fp, "Orbitcount   %d\n", this->_orbitcount);
                if (this->_starsystem && this->_starsystem->name)
                        fprintf(fp, "Starsystem   %s~\n",
                                this->_starsystem->name);
                if (this->_planet && this->_planet->name)
                        fprintf(fp, "Planet   %s~\n", this->_planet->name);
                FOR_EACH_LIST(AreaList, this->_areas, pArea)
                        if (pArea->filename)
                        fprintf(fp, "Area         %s~\n", pArea->filename);

                fprintf(fp, "End\n\n");

                FOR_EACH_LIST(DOCK_LIST, this->_docks, dock)
                {
                        if (dock->temporary)
                                continue;
                        fwrite_dock(fp, dock);
                }

                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void write_body_list()
{
        BodyData *tbody = NULL;
        FILE     *fpout;
        char filename[256];

        snprintf(filename, 256, "%s%s", BODY_DIR, FILE_BodyList);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open body.lst for writing!\n\r", 0);
                return;
        }
        FOR_EACH_LIST(BodyList, bodies, tbody)
                fprintf(fpout, "%s\n", tbody->filename());
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

void load_bodies()
{
        FILE     *fpList;
        const char *filename;
        char bodylist[256];
        PlanetData *planet;

        snprintf(bodylist, 256, "%s%s", BODY_DIR, FILE_BodyList);
        FCLOSE(fpReserve);
        if ((fpList = fopen(bodylist, "r")) == NULL)
        {
                perror(bodylist);
                exit(1);
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_body_file(const_cast<char*>(filename)))
                        bug("Cannot load body file: %s", filename);
        }
        FCLOSE(fpList);
        boot_log(" Done bodies ");
        for (planet = first_planet; planet; planet = planet->next)
                planet->body = get_body_all(planet->bodyname);
        return;
        return;
}

const char *BodyData::type_name() const
{
        switch (this->type()) {
                case BodyTypes::STAR: return "Star";
                case BodyTypes::PLANET: return "Planet";
                case BodyTypes::MOON: return "Moon";
                case BodyTypes::COMET: return "Comet";
                case BodyTypes::ASTEROID: return "Asteroid";
                case BodyTypes::BLACKHOLE: return "Blackhole";
                case BodyTypes::NEBULA: return "Nebula";
                case BodyTypes::ALL: return "All";
                default: return "Unknown";
        }
}