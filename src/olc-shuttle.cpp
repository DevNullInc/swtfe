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
 *                              OLC Shuttle Module                                       *
 ****************************************************************************************/
#include <unistd.h>
#include "mud.hpp"
#include "olc-shuttle.hpp"
#include "mxp.hpp"
#include "olc-shuttle.hpp"

ShuttleData *first_shuttle = NULL;
ShuttleData *last_shuttle = NULL;

StopData *create_stop(void)
{
        StopData *stop = NULL;

        CREATE(stop, StopData, 1);
        stop->next = NULL;
        stop->prev = NULL;
        stop->stop_name = NULL;
        stop->room = -1;
        return stop;
}

ShuttleData *create_shuttle(void)
{
        ShuttleData *shuttle = NULL;

        CREATE(shuttle, ShuttleData, 1);
        shuttle->name = NULL;
        shuttle->filename = NULL;
        shuttle->next_in_room = NULL;
        shuttle->prev_in_room = NULL;
        shuttle->in_room = NULL;
        shuttle->current = NULL;
        shuttle->current_number = -1;
        shuttle->state = ShuttleStateLanded;
        shuttle->first_stop = shuttle->last_stop = NULL;
        shuttle->type = ShuttleTurbocar;
        shuttle->delay = shuttle->current_delay = 2;
        shuttle->start_room = shuttle->end_room = shuttle->entrance =
                RoomVnumLimbo;
        /*
         * Not used right now anyways 
         */
        shuttle->takeoff_desc = NULL;
        shuttle->land_desc = NULL;
        shuttle->approach_desc = NULL;
        return shuttle;
}

ShuttleData *make_shuttle(char *filename, char *name)
{
        ShuttleData *shuttle = create_shuttle();

        shuttle->name = STRALLOC(name);
        shuttle->filename = str_dup(filename);
        if (save_shuttle(shuttle))
        {
                LINK(shuttle, first_shuttle, last_shuttle, next, prev);
                write_shuttle_list();
        }
        else
        {
                STRFREE(shuttle->name);
                DISPOSE(shuttle->filename);
                DISPOSE(shuttle);
                shuttle = NULL;
        }
        /*
         * Just so we don't duplicate code when the time comes 
         */
        return shuttle;
}

CMDF do_showshuttle(CharData * ch, char *argument)
{
        StopData *stop = NULL;
        int       count = 0;
        ShuttleData *shuttle = get_shuttle(argument);

        if (!shuttle)
        {
                if (first_shuttle == NULL)
                {
                        set_char_color(AtRed, ch);
                        send_to_char
                                ("There are no shuttles currently formed.\n\r",
                                 ch);
                        return;
                }
                set_char_color(AtRed, ch);
                send_to_char("No such shuttle.\n\rValid shuttles:\n\r", ch);
                set_char_color(AtShuttle, ch);
                for (shuttle = first_shuttle; shuttle;
                     shuttle = shuttle->next)
                        ch_printf(ch, "Shuttle Name: %s - %s\n\r",
                                  shuttle->name,
                                  shuttle->type ==
                                  ShuttleTurbocar ? "Turbocar" : shuttle->
                                  type ==
                                  ShuttleSpace ? "Space" : shuttle->type ==
                                  ShuttleHyperspace ? "Hyperspace" :
                                  "Other");
                return;
        }
        set_char_color(AtYellow, ch);
        ch_printf(ch, "Shuttle Name: %s - %s\n\r", shuttle->name,
                  shuttle->type == ShuttleTurbocar ? "Turbocar" :
                  shuttle->type == ShuttleSpace ? "Space" :
                  shuttle->type ==
                  ShuttleHyperspace ? "Hyperspace" : "Other");
        ch_printf(ch, "Filename: %s\t\tDelay: %d\n\r", shuttle->filename,
                  shuttle->delay);

        ch_printf(ch, "Start Room: %d\tEnd Room: %d\t\tEntrance: %d\n\r",
                  shuttle->start_room, shuttle->end_room, shuttle->entrance);

        send_to_char("Stops:\n\r", ch);
        for (stop = shuttle->first_stop; stop; stop = stop->next)
        {
                count += 1;
                ch_printf(ch, "\tStop # %d\n\r", count);
                ch_printf(ch, "\t\tStop Name: %s (%d)\n\r", stop->stop_name,
                          stop->room);
        }
        return;
}

CMDF do_makeshuttle(CharData * ch, char *argument)
{
        ShuttleData *shuttle;
        char      arg[MaxInputLength];

        argument = one_argument(argument, arg);

        if (!argument || !arg || arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char
                        ("Usage: makeshuttle <filename> <shuttle name>\n\r",
                         ch);
                return;
        }
        shuttle = make_shuttle(arg, argument);
        if (shuttle)
                send_to_char("Shuttle Created", ch);
        else
                send_to_char("Shuttle Failed to create", ch);
}

CMDF do_setshuttle(CharData * ch, char *argument)
{
        ShuttleData *shuttle;
        char      arg1[MIL];
        char      arg2[MIL];
        int       value;

        if (IsNpc(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
        {
                send_to_char
                        ("Usage: setshuttle <shuttle name> <field> <value>\n\r",
                         ch);
                send_to_char
                        ("Fields:\n\r\tstart_room, last_room, entrance, delay\n\r",
                         ch);
                send_to_char
                        ("\tname, filename, type, stop, remove shuttle\n\r",
                         ch);
                return;
        }

        shuttle = get_shuttle(arg1);
        if (!shuttle)
        {
                set_char_color(AtRed, ch);
                send_to_char("No such shuttle.\n\rValid shuttles:\n\r", ch);
                set_char_color(AtYellow, ch);
                for (shuttle = first_shuttle; shuttle;
                     shuttle = shuttle->next)
                        ch_printf(ch, "Shuttle Name: %s - %s\n\r",
                                  shuttle->name,
                                  shuttle->type ==
                                  ShuttleTurbocar ? "Turbocar" : shuttle->
                                  type ==
                                  ShuttleSpace ? "Space" : shuttle->type ==
                                  ShuttleHyperspace ? "Hyperspace" :
                                  "Other");
                return;
        }

        value = is_number(argument) ? atoi(argument) : -1;

        if (!str_cmp(arg2, "start_room"))
        {
                if (value > shuttle->end_room)
                {
                        send_to_char
                                ("Uh.. First room should be less than last room.\n\r",
                                 ch);
                        return;
                }
                shuttle->start_room = value;
        }
        else if (!str_cmp(arg2, "last_room"))
        {
                if (value < shuttle->start_room)
                {
                        send_to_char
                                ("Uh.. First room should be less than last room.\n\r",
                                 ch);
                        return;
                }
                shuttle->end_room = value;
        }
        else if (!str_cmp(arg2, "entrance"))
        {
                if (value > shuttle->end_room || value < shuttle->start_room)
                {
                        send_to_char("Not within Valid range.\n\r", ch);
                        return;
                }
                shuttle->entrance = value;
        }

        else if (!str_cmp(arg2, "delay"))
        {
                shuttle->delay = value;
                shuttle->current_delay = shuttle->delay;
        }
        else if (!str_cmp(arg2, "name"))
        {
                if (shuttle->name)
                        STRFREE(shuttle->name);
                shuttle->name = STRALLOC(argument);
        }
        else if (!str_cmp(arg2, "filename"))
        {
                if (shuttle->filename && shuttle->filename[0] != '\0')
                {
                        char      filename[MSL];

                        snprintf(filename, MSL, "%s/%s", ShuttleDir,
                                 shuttle->filename);
                        unlink(filename);
                        DISPOSE(shuttle->filename);
                }
                shuttle->filename = str_dup(argument);
                write_shuttle_list();
        }
        else if (!str_cmp(arg2, "type"))
        {
                if (!str_cmp(argument, "turbocar"))
                        shuttle->type = ShuttleTurbocar;
                else if (!str_cmp(argument, "space"))
                        shuttle->type = ShuttleSpace;
                else if (!str_cmp(argument, "hyperspace"))
                        shuttle->type = ShuttleHyperspace;
                else
                {
                        send_to_char
                                ("Types are: turbocar, space, hyperspace.\n\r",
                                 ch);
                        return;
                }
        }
        else if (!str_cmp(arg2, "remove"))
        {
                destroy_shuttle(shuttle);
                send_to_char("Shuttle Removed.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg2, "stop"))
        {
                StopData *stop = NULL;

                argument = one_argument(argument, arg1);
                if (arg1[0] == '\0')
                {
                        send_to_char("Usage: \n\r", ch);
                        send_to_char("\tsetshuttle <shuttle> stop <add>\n\r",
                                     ch);
                        send_to_char
                                ("\tsetshuttle <shuttle> stop <stop #> name <value>\n\r",
                                 ch);
                        send_to_char
                                ("\tsetshuttle <shuttle> stop <stop #> room <value>\n\r",
                                 ch);
                        return;
                }

                if (!str_cmp(arg1, "add"))
                {
                        stop = create_stop();
                        stop->stop_name = STRALLOC("Stopless Name");
                        stop->room = 2;
                        LINK(stop, shuttle->first_stop, shuttle->last_stop,
                             next, prev);
                        if (shuttle->current == NULL)
                                shuttle->current = shuttle->first_stop;
                }
                else
                {
                        int       count = 0;

                        if (arg1[0] == '\0' || argument[0] == '\0')
                        {
                                send_to_char("Invalid Param.\n\r", ch);
                                return;
                        }
                        value = is_number(arg1) ? atoi(arg1) : -1;

                        for (stop = shuttle->first_stop; stop;
                             stop = stop->next)
                        {
                                count++;
                                if (value == count)
                                        break;
                        }

                        if (stop == NULL)
                        {
                                send_to_char("Invalid Stop\n\r", ch);
                                return;
                        }

                        argument = one_argument(argument, arg2);
                        if (!str_cmp(arg2, "name"))
                        {
                                if (stop->stop_name)
                                        STRFREE(stop->stop_name);
                                stop->stop_name = str_dup(argument);
                        }
                        else if (!str_cmp(arg2, "room"))
                        {
                                value = is_number(argument) ? atoi(argument) :
                                        -1;
                                stop->room = value;
                        }
                        else if (!str_cmp(arg2, "remove"))
                        {
                                UNLINK(stop, shuttle->first_stop,
                                       shuttle->last_stop, next, prev);
                                if (stop->stop_name)
                                        STRFREE(stop->stop_name);
                                DISPOSE(stop);
                                send_to_char("Stop removed.\n\r", ch);
                        }
                        else
                        {
                                send_to_char("Invalid Option.\n\r", ch);
                                return;
                        }
                }
        }
        else
        {
                send_to_char("Unknown field", ch);
                return;
        }
        save_shuttle(shuttle);
        send_to_char("Ok.\n\r", ch);
        return;
}

ShuttleData *get_shuttle(char *name)
{
        ShuttleData *shuttle;

        for (shuttle = first_shuttle; shuttle; shuttle = shuttle->next)
                if (!str_cmp(name, shuttle->name))
                        return shuttle;

        for (shuttle = first_shuttle; shuttle; shuttle = shuttle->next)
                if (nifty_is_name_prefix(name, shuttle->name))
                        return shuttle;

        return NULL;
}

void write_shuttle_list()
{
        ShuttleData *shuttle;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, 256, "%s%s", ShuttleDir, ShuttleList);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open shuttle.lst for writing!\n\r", 0);
                return;
        }

        for (shuttle = first_shuttle; shuttle; shuttle = shuttle->next)
                fprintf(fpout, "%s\n", shuttle->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

bool save_shuttle(ShuttleData * shuttle)
{
        FILE     *fp;
        char      filename[256];
        StopData *stop;

        if (!shuttle)
        {
                bug("save_shuttle: null shuttle pointer!", 0);
                return FALSE;
        }

        if (!shuttle->filename || shuttle->filename[0] == '\0')
        {
                bug("save_shuttle: %s has no filename", shuttle->name);
                return FALSE;
        }

        snprintf(filename, 256, "%s%s", ShuttleDir, shuttle->filename);

        /*
         * FCLOSE( fpReserve ); 
         */
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_shuttle: fopen", 0);
                perror("save_shuttle: fopen");
                /*
                 * perror( filename ); 
                 */
                return FALSE;
        }

        fprintf(fp, "#SHUTTLE\n");
        fprintf(fp, "Name         %s~\n", shuttle->name);
        fprintf(fp, "Filename     %s~\n", shuttle->filename);
        fprintf(fp, "Delay        %d\n", shuttle->delay);
        fprintf(fp, "CurrentDelay %d\n", shuttle->current_delay);
        if (shuttle->current)
                fprintf(fp, "Current      %d\n", shuttle->current_number);
        fprintf(fp, "Type         %d\n", shuttle->type);
        fprintf(fp, "State        %d\n", shuttle->state);
        fprintf(fp, "StartRoom    %d\n", shuttle->start_room);
        fprintf(fp, "EndRoom      %d\n", shuttle->end_room);
        fprintf(fp, "Entrance      %d\n", shuttle->entrance);
        if (shuttle->takeoff_desc)
                fprintf(fp, "Takeoff      %s~\n", shuttle->takeoff_desc);
        if (shuttle->land_desc)
                fprintf(fp, "Land         %s~\n", shuttle->land_desc);
        if (shuttle->approach_desc)
                fprintf(fp, "Approach     %s~\n", shuttle->approach_desc);


        fprintf(fp, "End\n\n");

        /*
         * Rest Goes Here 
         */
        for (stop = shuttle->first_stop; stop; stop = stop->next)
        {
                fprintf(fp, "#STOP\n");
                fprintf(fp, "StopName	   %s~\n", stop->stop_name);
                fprintf(fp, "Room    	   %d\n", stop->room);
                fprintf(fp, "End\n\n");
        }

        fprintf(fp, "#END\n");
        FCLOSE(fp);

        /*
         * fpReserve = fopen( NullFile, "r" ); 
         */
        return TRUE;
}

void update_shuttle()
{
        char      buf[MSL];
        ShuttleData *shuttle = NULL;

        for (shuttle = first_shuttle; shuttle; shuttle = shuttle->next)
        {
                /*
                 * No Stops? Make sure we ignore 
                 */
                if (shuttle->first_stop == NULL)
                        continue;
                if (shuttle->current == NULL)
                {
                        shuttle->current = shuttle->first_stop;
                        /*
                         * bug("Shuttle '%s' no current", shuttle->name); 
                         */
                        continue;
                }
#ifdef DEBUG
                bug("Shuttle '%s' delay: %d", shuttle->name,
                    shuttle->current_delay);
#endif
                if (--shuttle->current_delay <= 0)
                {
                        int       room = 0;

                        shuttle->current_delay = shuttle->delay;
                        /*
                         * Probably some intermediate Stages in the middle ? 
                         */
                        if (shuttle->state == ShuttleStateTakingoff)
                        {
                                /*
                                 * Move to next spot 
                                 */
                                if (shuttle->current->next == NULL)
                                {
                                        shuttle->current =
                                                shuttle->first_stop;
                                        shuttle->current_number = 1;
                                }
                                else
                                {
                                        shuttle->current =
                                                shuttle->current->next;
                                        shuttle->current_number++;
                                }
#ifdef DEBUG
                                bug("Shuttle '%s' Taking Off.",
                                    shuttle->name);
#endif
                                /*
                                 * An electronic voice says, 'Preparing for launch.'
                                 * It continues, 'Next stop, Gamorr'
                                 * 
                                 * The hatch slides shut.
                                 * The ship begins to launch.
                                 */
                                snprintf(buf, MSL,
                                         "An electronic voice says, 'Preparing for launch.'\n\r"
                                         "It continues, 'Next stop, %s'",
                                         shuttle->current->stop_name);
                                for (room = shuttle->start_room;
                                     room <= shuttle->end_room; ++room)
                                {
                                        RoomIndexData *iRoom =
                                                get_room_index(room);
                                        echo_to_room(AtCyan, iRoom, buf);
                                        if (shuttle->type != ShuttleTurbocar)
                                        {
                                                echo_to_room(AtYellow, iRoom,
                                                             "The hatch slides shut.");
                                                echo_to_room(AtYellow, iRoom,
                                                             "The ship begins to launch.");
                                        }
                                }

                                /*
                                 * FIXME - Sound to room 
                                 */
                                if (shuttle->type != ShuttleTurbocar)
                                        snprintf(buf, MSL,
                                                 "The hatch on %s closes and it begins to launch..",
                                                 shuttle->name);
                                else
                                        snprintf(buf, MSL,
                                                 "%s speeds out of the station.",
                                                 shuttle->name);
                                echo_to_room(AtYellow, shuttle->in_room,
                                             buf);

                                extract_shuttle(shuttle);

                                if (shuttle->type == ShuttleTurbocar
                                    || shuttle->type == ShuttleSpace)
                                        shuttle->state =
                                                ShuttleStateLanding;
                                else if (shuttle->type == ShuttleHyperspace)
                                        shuttle->state =
                                                ShuttleStateHyperspaceLaunch;
                                else
                                        bug("Shuttle '%s' is an unknown type",
                                            shuttle->name);
                        }
                        else if (shuttle->state ==
                                 ShuttleStateHyperspaceLaunch)
                        {
                                for (room = shuttle->start_room;
                                     room <= shuttle->end_room; ++room)
                                        echo_to_room(AtYellow,
                                                     get_room_index(room),
                                                     "The ship lurches slightly as it makes the jump to lightspeed.");
                                shuttle->state = ShuttleStateHyperspaceEnd;
                                shuttle->current_delay *= 2;
                        }
                        else if (shuttle->state ==
                                 ShuttleStateHyperspaceEnd)
                        {
                                for (room = shuttle->start_room;
                                     room <= shuttle->end_room; ++room)
                                        echo_to_room(AtYellow,
                                                     get_room_index(room),
                                                     "The ship lurches slightly as it comes out of hyperspace.");
                                shuttle->state = ShuttleStateLanding;
                        }
                        else if (shuttle->state == ShuttleStateLanding)
                        {
                                /*
                                 * An electronic voice says, 'Welcome to Adari'
                                 * * It continues, 'Please exit through the main ramp. Enjoy your stay.' 
                                 * *
                                 * * You feel a slight thud as the ship sets down on the ground.
                                 * * The hatch opens.
                                 */


                                /*
                                 * action_desc 
                                 */
                                snprintf(buf, MSL,
                                         "An electronic voice says, 'Welcome to %s'\n\r"
                                         "It continues, 'Please exit through the %s. Enjoy your stay.'",
                                         shuttle->current->stop_name,
                                         shuttle->type ==
                                         ShuttleTurbocar ? "doors" :
                                         "main ramp");

                                insert_shuttle(shuttle,
                                               get_room_index(shuttle->
                                                              current->room));

                                for (room = shuttle->start_room;
                                     room <= shuttle->end_room; ++room)
                                {
                                        RoomIndexData *iRoom =
                                                get_room_index(room);
                                        echo_to_room(AtCyan, iRoom, buf);
                                        if (shuttle->type != ShuttleTurbocar)
                                        {
                                                echo_to_room(AtYellow, iRoom,
                                                             "You feel a slight thud as the ship sets down on the ground.");
                                                echo_to_room(AtYellow, iRoom,
                                                             "The hatch opens.");
                                        }
                                }

                                if (shuttle->type != ShuttleTurbocar)
                                        snprintf(buf, MSL,
                                                 "%s lands on the platform.",
                                                 shuttle->name);
                                else
                                        snprintf(buf, MSL,
                                                 "%s arrives at the station.",
                                                 shuttle->name);
                                echo_to_room(AtYellow, shuttle->in_room,
                                             buf);
                                if (shuttle->type != ShuttleTurbocar)
                                {
                                        snprintf(buf, MSL,
                                                 "The hatch on %s opens.",
                                                 shuttle->name);
                                        echo_to_room(AtYellow,
                                                     shuttle->in_room, buf);
                                        /*
                                         * FIXME - Sound to room 
                                         */
                                }

                                shuttle->state = ShuttleStateLanded;
                        }
                        else if (shuttle->state == ShuttleStateLanded)
                        {
                                /*
                                 * Just for a delay between stops ? 
                                 */
                                shuttle->state = ShuttleStateTakingoff;
                        }
                        else
                        {
                                bug("Shuttle '%s' has invalid state of %d",
                                    shuttle->name, shuttle->state);
                                continue;
                        }
                }
        }
        return;
}

void show_shuttles_to_char(CharData * ch, ShuttleData * shuttle)
{
        while (shuttle)
        {
                set_char_color(AtShuttle, ch);
                if (IsMxp(ch))
                        ch_printf(ch, MXPTAG("shuttle '%s'"), shuttle->name);
                ch_printf(ch, "%-35s", shuttle->name);
                if (IsMxp(ch))
                        send_to_char(MXPTAG("/ship"), ch);
                /*
                 * eww code dupliction 
                 */
                if (shuttle->next_in_room)
                {
                        shuttle = shuttle->next_in_room;
                        if (IsMxp(ch))
                                ch_printf(ch, MXPTAG("shuttle '%s'"),
                                          shuttle->name);
                        ch_printf(ch, "%-35s", shuttle->name);
                        if (IsMxp(ch))
                                send_to_char(MXPTAG("/shuttle"), ch);
                }
                shuttle = shuttle->next_in_room;
                send_to_char("\n\r&w", ch);
        }

        return;
}

bool extract_shuttle(ShuttleData * shuttle)
{
        RoomIndexData *room;

        if ((room = shuttle->in_room) != NULL)
        {
                UNLINK(shuttle, room->first_shuttle, room->last_shuttle,
                       next_in_room, prev_in_room);
                shuttle->in_room = NULL;
        }
        return TRUE;
}

bool insert_shuttle(ShuttleData * shuttle, RoomIndexData * room)
{
        if (shuttle == NULL)
        {
                bug("insert_shuttle: NULL shuttle", 0);
                return FALSE;
        }

        if (room == NULL)
        {
                bug("Insert_shuttle: %s Room: %d",
                    shuttle->name ? shuttle->name : "ERROR",
                    shuttle->current->room);
                return FALSE;
        }
        /*
         * Error checking = GOOD 
         */
        if (shuttle->in_room)
                extract_shuttle(shuttle);
        shuttle->in_room = room;
        LINK(shuttle, room->first_shuttle, room->last_shuttle, next_in_room,
             prev_in_room);
        return TRUE;
}


/*
 * Load in all the ship files.
 */
void load_shuttles()
{
        FILE     *fpList;
        const char *filename;
        char      shuttlelist[256];

        first_shuttle = NULL;
        last_shuttle = NULL;

        snprintf(shuttlelist, 256, "%s%s", ShuttleDir, ShuttleList);
        FCLOSE(fpReserve);
        if ((fpList = fopen(shuttlelist, "r")) == NULL)
        {
                perror(shuttlelist);
                exit(1);
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);

                if (filename[0] == '$')
                        break;

                if (!load_shuttle_file((char *) filename))
                        bug("Cannot load shuttle file: %s", filename);
        }
        FCLOSE(fpList);
        boot_log("Done shuttles");
        fpReserve = fopen(NullFile, "r");
        return;
}

/*
 * Load a ship file
 */

bool load_shuttle_file(char *shuttlefile)
{
        char      filename[256];
        ShuttleData *shuttle;
        FILE     *fp;
        bool      found;

        shuttle = create_shuttle();

        found = FALSE;
        snprintf(filename, 256, "%s%s", ShuttleDir, shuttlefile);

        if ((fp = fopen(filename, "r")) != NULL)
        {
                found = TRUE;
                for (;;)
                {
                        char      letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("Load_shuttle_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "SHUTTLE"))
                        {
                                fread_shuttle(shuttle, fp);
                                if (shuttle->entrance == -1)
                                        shuttle->entrance =
                                                shuttle->start_room;
                                shuttle->in_room = NULL;
                                continue;
                        }
                        else if (!str_cmp(word, "STOP"))
                        {
                                StopData *stop = create_stop();

                                fread_stop(stop, fp);
                                LINK(stop, shuttle->first_stop,
                                     shuttle->last_stop, next, prev);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                        {
                                break;
                        }
                        else
                        {
                                bug("Load_shuttle_file: bad section: %s.",
                                    word);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        if (!(found))
        {
                DISPOSE(shuttle);
        }
        else
        {
                LINK(shuttle, first_shuttle, last_shuttle, next, prev);
                if (shuttle->current_number != -1)
                {
                        int       count = 0;
                        StopData *stop = NULL;

                        for (stop = shuttle->first_stop; stop;
                             stop = stop->next)
                        {
                                count++;
                                if (count == shuttle->current_number)
                                        shuttle->current = stop;
                        }
                }
                else
                {
                        shuttle->current_number = 0;
                        shuttle->current = shuttle->first_stop;
                }
                if (shuttle->current)
                        insert_shuttle(shuttle,
                                       get_room_index(shuttle->current->
                                                      room));
        }
        return found;
}

void fread_shuttle(ShuttleData * shuttle, FILE * fp)
{
        const char *word;
        bool      fMatch;

        shuttle->delay = 2;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);

                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;
                case 'A':
                        KEY("Approach", shuttle->approach_desc,
                            fread_string_nohash(fp));
                        break;

                case 'C':
                        KEY("Current", shuttle->current_number,
                            fread_number(fp));
                        KEY("CurrentDelay", shuttle->current_delay,
                            fread_number(fp));
                        break;

                case 'D':
                        KEY("Delay", shuttle->delay, fread_number(fp));
                        break;

                case 'E':
                        KEY("EndRoom", shuttle->end_room, fread_number(fp));
                        KEY("Entrance", shuttle->entrance, fread_number(fp));
                        if (!str_cmp(word, "End"))
                        {
                                shuttle->current_delay = shuttle->delay;
                                shuttle->next_in_room = NULL;
                                shuttle->prev_in_room = NULL;
                                shuttle->in_room = NULL;
                                shuttle->current = NULL;
                                shuttle->first_stop = NULL;
                                shuttle->last_stop = NULL;
                                return;
                        }
                        break;

                case 'F':
                        KEY("Filename", shuttle->filename,
                            fread_string_nohash(fp));
                        break;

                case 'L':
                        KEY("Land", shuttle->land_desc,
                            fread_string_nohash(fp));
                        break;

                case 'N':
                        KEY("Name", shuttle->name, fread_string(fp));
                        break;

                case 'S':
                        KEY("StartRoom", shuttle->start_room,
                            fread_number(fp));
                        KEY("State", shuttle->state, fread_number(fp));
                        break;

                case 'T':
                        KEY("Type", shuttle->type,
                            (ShuttleClass) fread_number(fp));
                        KEY("Takeoff", shuttle->takeoff_desc,
                            fread_string_nohash(fp));
                        break;
                }

                if (!fMatch)
                {
                        bug("Fread_shuttle: no match: %s", word);
                }
        }
}

void fread_stop(StopData * stop, FILE * fp)
{
        const char *word;
        bool      fMatch;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);

                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                return;
                        }
                        break;

                case 'R':
                        KEY("Room", stop->room, fread_number(fp));
                        break;

                case 'S':
                        KEY("StopName", stop->stop_name,
                            fread_string_nohash(fp));
                        break;
                }

                if (!fMatch)
                {
                        bug("Fread_shuttle: no match: %s", word);
                }
        }
}

void destroy_shuttle(ShuttleData * shuttle)
{
        StopData *stop, *stop_next;

        UNLINK(shuttle, first_shuttle, last_shuttle, next, prev);
        if (shuttle->filename)
        {
                char      buf[MSL];

                snprintf(buf, MSL, "%s/%s", ShuttleDir, shuttle->filename);
                unlink(buf);
                DISPOSE(shuttle->filename);
        }

        for (stop = shuttle->first_stop; stop; stop = stop_next)
        {
                stop_next = stop->next;
                if (stop->stop_name)
                        DISPOSE(stop->stop_name);
                DISPOSE(stop);
        }

        if (shuttle->name)
                STRFREE(shuttle->name);
        if (shuttle->takeoff_desc)
                DISPOSE(shuttle->takeoff_desc);
        if (shuttle->filename)
                DISPOSE(shuttle->filename);
        if (shuttle->approach_desc)
                DISPOSE(shuttle->approach_desc);
        if (shuttle->land_desc)
                DISPOSE(shuttle->land_desc);

        DISPOSE(shuttle);
        write_shuttle_list();

        return;
}

void free_shuttle(ShuttleData * shuttle)
{
        StopData *stop, *stop_next;

        UNLINK(shuttle, first_shuttle, last_shuttle, next, prev);
        if (shuttle->filename)
                DISPOSE(shuttle->filename);
        for (stop = shuttle->first_stop; stop; stop = stop_next)
        {
                stop_next = stop->next;
                if (stop->stop_name)
                        DISPOSE(stop->stop_name);
                DISPOSE(stop);
        }

        if (shuttle->name)
                STRFREE(shuttle->name);
        if (shuttle->takeoff_desc)
                DISPOSE(shuttle->takeoff_desc);
        if (shuttle->filename)
                DISPOSE(shuttle->filename);
        if (shuttle->approach_desc)
                DISPOSE(shuttle->approach_desc);
        if (shuttle->land_desc)
                DISPOSE(shuttle->land_desc);

        DISPOSE(shuttle);
        return;
}

ShuttleData *shuttle_in_room(RoomIndexData * room, char *name)
{
        ShuttleData *shuttle;

        if (!room)
                return NULL;

        for (shuttle = room->first_shuttle; shuttle;
             shuttle = shuttle->next_in_room)
                if (!str_cmp(name, shuttle->name))
                        return shuttle;

        for (shuttle = room->first_shuttle; shuttle;
             shuttle = shuttle->next_in_room)
                if (nifty_is_name_prefix(name, shuttle->name))
                        return shuttle;

        return NULL;
}

ShuttleData *shuttle_from_entrance(int vnum)
{
        ShuttleData *shuttle;

        for (shuttle = first_shuttle; shuttle; shuttle = shuttle->next)
                if (vnum == shuttle->entrance)
                        return shuttle;
        return NULL;
}
