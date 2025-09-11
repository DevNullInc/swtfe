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
 * Immortal communication tools for staff interactions and administrative messaging.     *
 ****************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.hpp"
#include "homes.hpp"
#include "changes.hpp"
#include "astral.hpp"
#include "space2.hpp"

void list_resets args((CharData * ch, AreaData * pArea,
                       RoomIndexData * pRoom, int start, int end));
void save_sysdata args((SystemData sys));
void save_banlist args((void));


CMDF do_fakequit(CharData * ch, char *argument)
{
        char      buf[MaxInputLength];

        argument = NULL;
        set_char_color(AtWhite, ch);
        send_to_char
                ("You have quit, how funny\n\rYou left the game, well not completly\n\r",
                 ch);
        act(AtBye, "$n has left the game.", ch, NULL, NULL, ToRoom);
        set_char_color(AtGrey, ch);
        snprintf(log_buf, MSL, "%s has quit.", ch->name);
        snprintf(buf, MSL, "%s has left %s", ch->name, sysdata.mud_name);
        info_chan(buf);


        if (!IsSet(ch->act, PlrWizinvis))
        {
                SetBit(ch->act, PlrWizinvis);
        }


}
CMDF do_fakeenter(CharData * ch, char *argument)
{
        char      buf[MaxInputLength];

        argument = NULL;
        set_char_color(AtWhite, ch);
        act(AtWhite, "$n has entered the game.", ch, NULL, NULL, ToRoom);
        set_char_color(AtGrey, ch);
        snprintf(buf, MSL, "%s has entered %s", ch->name, sysdata.mud_name);
        info_chan(buf);


        if (IsSet(ch->act, PlrWizinvis))
        {
                RemoveBit(ch->act, PlrWizinvis);
        }


}

CMDF do_lagout(CharData * ch, char *argument)
{

        CharData *victim;
        char      arg1[MaxStringLength];
        int       x;

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: Lagout <victim> <amount>\n\r", ch);
                return;
        }

        if (!str_cmp("badhash", argument))
        {
                char     *hashstr = STRALLOC("Hash test");
                char     *nohashstr = str_dup("Non-hashed test");

                DISPOSE(hashstr);
                STRFREE(nohashstr);
                return;
        }

        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }

        if ((x = atoi(argument)) <= 0)
        {
                send_to_char("So your giving them 0 lag?\n\r", ch);
                return;
        }

        if (x > 1000)
        {
                send_to_char("That # is too high.\n\r", ch);
                return;
        }

        send_to_char("Your getting some lag right about now...\n\r", victim);
        WaitState(victim, x);
        send_to_char("Adding lag...\n\r", ch);
        return;
}


CMDF do_rseek(CharData * ch, char *argument)
{
        ResetData *pReset;
        AreaData *pArea;
        int       x, counter = 1;

        if (!is_number(argument))
        {
                ch_printf(ch,
                          "%s is not a number, and only vnums can be searched for in resets.\n\r",
                          argument);
                return;
        }
        x = atoi(argument);
        for (pArea = first_area; pArea; pArea = pArea->next)
        {
                for (pReset = pArea->first_reset; pReset;
                     pReset = pReset->next)
                {
                        if (pReset->arg1 == x || pReset->arg2 == x
                            || pReset->arg3 == x)
                        {
                                pager_printf(ch,
                                             "[%-6d] %-20s Reset Number: %d\n\r",
                                             x, pArea->filename, counter);
                                list_resets(ch, pArea, NULL, counter,
                                            counter);
                                send_to_char("\n\r", ch);
                        }
                        counter++;
                }
                counter = 1;
        }
        return;
}

CMDF do_mudsave(CharData * ch, char *argument)
{

        CharData *wch;
        ShipData *ship;
        SpaceData *starsystem;
        ClanData *clan;
        PlanetData *planet;
        char      arg[MaxInputLength];
        AreaData *tarea;
        BodyData *body = NULL;

        if (IsNpc(ch))
                return;

        one_argument(argument, arg);

        send_to_char("Saving Socials..................Done.\n\r", ch);
        save_socials();
        send_to_char("Saving Skill Table..............Done.\n\r", ch);
        save_skill_table();
        send_to_char("Saving Ban List.................Done.\n\r", ch);
        save_banlist();
        send_to_char("Saving System Data..............Done.\n\r", ch);
        save_sysdata(sysdata);
        send_to_char("Saving Command Table............Done.\n\r", ch);
        save_commands();
        send_to_char("Saving Changes..................Done.\n\r", ch);
        save_changes();
        send_to_char("Saving Fingers..................Done.\n\r", ch);
        send_to_char("Saving Homes....................Done.\n\r", ch);
        for (HomeData * home = first_home; home; home = home->next)
                home->save();

        for (wch = first_char; wch; wch = wch->next)
        {
                do_save(wch, "");
                save_finger(wch);
                save_home(wch);
        }

        send_to_char("Saving Clans....................Done.\n\r", ch);
        for (clan = first_clan; clan; clan = clan->next)
        {
                save_clan(clan);
        }

        send_to_char("Saving Planets..................Done.\n\r", ch);
        for (planet = first_planet; planet; planet = planet->next)
        {
                save_planet(planet, FALSE);
        }

        send_to_char("Saving Ships....................Done.\n\r", ch);
        for (ship = first_ship; ship; ship = ship->next)
        {
                save_ship(ship);
        }
        send_to_char("Saving Star System..............Done.\n\r", ch);
        for (starsystem = first_starsystem; starsystem;
             starsystem = starsystem->next)
        {
                save_starsystem(starsystem);
        }
        send_to_char("Saving Body.....................Done.\n\r", ch);
        ForEachList(BodyList, bodies, body) body->save();
        send_to_char("Done.\n\r", ch);
#ifdef OlcHomes
        send_to_char("Saving Homes.....................", ch);
        {
                HomeData *home = NULL;

                for (home = first_home; home; home = home->next)
                        home->save();
        }
        send_to_char("Done.\n\r", ch);
#endif
        if (!str_cmp(arg, "areas"))
        {
                send_to_char("Saving Area Files...............Done.\n\r", ch);
                for (tarea = first_area; tarea; tarea = tarea->next)
                        fold_area(tarea, tarea->filename, FALSE, FALSE);
                for (tarea = first_bsort; tarea; tarea = tarea->next_sort)
                        if (IsSet(tarea->status, AreaLoaded))
                                fold_area(tarea, tarea->filename, FALSE,
                                          TRUE);

        }
        send_to_char("&W&wMud save completed.", ch);
        return;

}

CMDF do_working(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];

        argument = NULL;
        if (IsNpc(ch))
                return;


        if (IsSet(ch->pcdata->flags, PcflagWorking))
        {
                RemoveBit(ch->pcdata->flags, PcflagWorking);
                send_to_char("You relax after working so hard!&R&W\n\r", ch);
                if (!IsSet(ch->act, PlrWizinvis))
                {
                        snprintf(buf, MSL, "%s is no longer working.",
                                 ch->name);
                        info_chan(buf);
                }
        }
        else
        {
                SetBit(ch->pcdata->flags, PcflagWorking);
                send_to_char
                        ("You get ready to start working really hard!\n\r",
                         ch);
                if (!IsSet(ch->act, PlrWizinvis))
                {
                        snprintf(buf, MSL, "%s is now working.", ch->name);
                        info_chan(buf);
                }
        }

        return;
}
