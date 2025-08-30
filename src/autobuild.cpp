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
 *                                 AutoBuild Module                                      *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"
#include <unistd.h>

CMDF do_create_list(CHAR_DATA * ch, char *argument)
{
        FILE     *fp;
        char      filename[256];

        /* Use step-by-step buffer building to avoid truncation */
        size_t len = 0;
        filename[0] = '\0';
        len += snprintf(filename + len, 256 - len, "%s", LIST_DIR);
        len += snprintf(filename + len, 256 - len, "%s", argument);

        if (file_exist(filename))
        {
                send_to_char("File already exists.\n\r", ch);
                return;
        }

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_body: fopen", 0);
                perror(filename);
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        send_to_char("File created sucessfully.\n\r", ch);

}

CMDF do_addto_list(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        char      arg1[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);

        if ((arg1[0] == '\0') || (argument[0] == '\0'))
        {
                send_to_char
                        ("Syntax: addtolist <list filename> <text to add>\n\r",
                         ch);
                return;
        }

        /* Use step-by-step buffer building to avoid truncation */
        size_t len = 0;
        filename[0] = '\0';
        len += snprintf(filename + len, 256 - len, "%s", LIST_DIR);
        len += snprintf(filename + len, 256 - len, "%s", arg1);

        if (!file_exist(filename))
        {
                send_to_char("File does not exist.\n\r", ch);
                return;
        }

        append_to_file(filename, argument);
        send_to_char("Text added to list.\n\r", ch);

}

CMDF do_showlist(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        FILE     *fp;
        int       line = 0;
        char     *string;

        if (argument[0] == '\0')
        {
                send_to_char("Syntax: showlist <list filename>\n\r", ch);
                return;
        }

        /* Use step-by-step buffer building to avoid truncation */
        size_t len = 0;
        filename[0] = '\0';
        len += snprintf(filename + len, 256 - len, "%s", LIST_DIR);
        len += snprintf(filename + len, 256 - len, "%s", argument);

        if (!file_exist(filename))
        {
                send_to_char("File does not exist.\n\r", ch);
                return;
        }
        else
        {
                send_to_char("&w", ch);
                if ((fp = fopen(filename, "r")) != NULL)
                {
                        for (;;)
                        {
                                string = feof(fp) ? (char *) "end" :
                                        fread_line(fp);
                                if (!str_cmp(string, "end"))
                                        break;
                                ch_printf(ch, "%4d) %s", ++line, string);
                        }
                }
                FCLOSE(fp);
                fpReserve = fopen(NULL_FILE, "r");
                ch_printf(ch, "&B[&w%d&B] &zlines in %s\n\r", line, filename);
        }
}

CMDF do_remlist(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        char      fntemp[256];
        FILE     *fp = NULL;
        FILE     *fptemp = NULL;
        int       line = 0;
        int       linetemp = 0;
        char     *string;
        char      arg1[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);

        if ((arg1[0] == '\0') || (argument[0] == '\0'))
        {
                send_to_char
                        ("Syntax: remtolist <list filename> <line to remove>\n\r",
                         ch);
                return;
        }

        if (!is_number(argument))
        {
                send_to_char
                        ("Syntax: line must be referenced by a number\n\r",
                         ch);
                return;
        }

        /* Use step-by-step buffer building to avoid truncation */
        size_t len1 = 0;
        filename[0] = '\0';
        len1 += snprintf(filename + len1, 256 - len1, "%s", LIST_DIR);
        len1 += snprintf(filename + len1, 256 - len1, "%s", arg1);

        /* Use step-by-step buffer building to avoid truncation */
        size_t len2 = 0;
        fntemp[0] = '\0';
        len2 += snprintf(fntemp + len2, 256 - len2, "%s", LIST_DIR);
        len2 += snprintf(fntemp + len2, 256 - len2, "%s", arg1);
        len2 += snprintf(fntemp + len2, 256 - len2, ".tmp");

        if (!file_exist(filename))
        {
                send_to_char("File does not exist.\n\r", ch);
                return;
        }

        if ((fp = fopen(filename, "r")) != NULL
            && (fptemp = fopen(fntemp, "w")) != NULL)
        {
                for (;;)
                {
                        string = feof(fp) ? (char *) "end" : fread_line(fp);
                        if (!str_cmp(string, "end"))
                                break;
                        ++line;
                        fprintf(fptemp, "%s", string);
                }
        }

        if (fp)
                FCLOSE(fp);
        if (fptemp)
                FCLOSE(fptemp);

        if ((atoi(argument)) > line)
        {
                send_to_char("Line number invalid.\n\r", ch);
                return;
        }

        if ((fp = fopen(filename, "w")) != NULL
            && (fptemp = fopen(fntemp, "r")) != NULL)
        {
                for (;;)
                {
                        string = feof(fptemp) ? (char *) "end" :
                                fread_line(fptemp);
                        if (!str_cmp(string, "end"))
                                break;
                        ++linetemp;
                        if (linetemp == atoi(argument))
                                continue;
                        fprintf(fp, "%s", string);
                }
        }

        if (fp)
                FCLOSE(fp);
        if (fptemp)
                FCLOSE(fptemp);
        fpReserve = fopen(NULL_FILE, "r");
        unlink(fntemp);

}
