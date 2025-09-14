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
 *                           ^     +----- |  / ^     ^ |     | +-\                       *
 *                          / \    |      | /  |\   /| |     | |  \                      *
 *                         /   \   +---   |<   | \ / | |     | |  |                      *
 *                        /-----\  |      | \  |  v  | |     | |  /                      *
 *                       /       \ |      |  \ |     | +-----+ +-/                       *
 *****************************************************************************************
 *                                                                                       *
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson), Levi Beckerson (Whir),           *
 * Michael Ward (Tarl), Erik Wolfe (Dwip), Cameron Carroll (Cam), Cyberfox, Karangi,     *
 * Rathian, Raine, and Adjani. All Rights Reserved.                                      *
 *                                                                                       *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                      Color Module -- Allow user customizable Colors.                  *
 *                                         --Matthew                                     *
 *                             Enhanced ANSI parser by Samson                            *
 ****************************************************************************************/

/*
* The following instructions assume you know at least a little bit about
* coding.  I firmly believe that if you can't code (at least a little bit),
* you don't belong running a mud.  So, with that in mind, I don't hold your
* hand through these instructions.
*
* You may use this code provided that:
*
*     1)  You understand that the authors _DO NOT_ support this code
*         Any help you need must be obtained from other sources.  The
*         authors will ignore any and all requests for help.
*     2)  You will mention the authors if someone asks about the code.
*         You will not take credit for the code, but you can take credit
*         for any enhancements you make.
*     3)  This message remains intact.
*
* If you would like to find out how to send the authors large sums of money,
* you may e-mail the following address:
*
* Matthew Bafford & Christopher Wigginton
* wiggy@mudservices.com
*/

/*
 * To add new color types:
 *
 * 1.  Edit color.h, and:
 *     1.  Add a new AT_ define.
 *     2.  Increment MaxColors by however many AT_'s you added.
 * 2.  Edit color.c and:
 *     1.  Add the name(s) for your new color(s) to the end of the pc_displays array.
 *     2.  Add the default color(s) to the end of the default_set array.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "mud.hpp"
#include "mxp.hpp"
#include "cpp_compat.hpp"

extern const char *const pc_displays[MaxColors];

const char *const pc_displays[MaxColors] = {
        "black", "dred", "dgreen", "orange",
        "dblue", "purple", "cyan", "grey",
        "dgrey", "red", "green", "yellow",
        "blue", "pink", "lblue", "white",
        "blink", "plain", "action", "say",
        "chat", "yells", "tell", "hit",
        "hitme", "immortal", "hurting", "falling",
        "danger", "magic", "consider", "report",
        "poison", "social", "dying", "dead",
        "skills", "lines", "damage", "fleeing",
        "rmname", "rmdesc", "objects", "people",
        "list", "bye", "gold", "gtells",
        "note", "hungry", "thirsty", "fire",
        "sober", "wearoff", "exits", "score",
        "reset", "log", "die_msg", "wartalk",
        "arena", "muse", "think", "aflags",
        "who", "racetalk", "ignore", "whisper",
        "divider", "morph", "shout", "rflags",
        "stype", "aname", "auction", "score2",
        "score3", "score4", "who2", "who3",
        "who4", "intermud", "helpfiles", "who5",
        "score5", "who6", "who7", "prac",
        "prac2", "prac3", "prac4", "mxpprompt",
        "guildtalk", "ship", "clan", "ooc",
        "avatar", "shuttle"
};

/* All defaults are set to Alsherok default scheme, if you don't 
like it, change it around to suite your own needs - Samson */
const sh_int default_set[MaxColors] = {
        AtBlack, AtBlood, AtDgreen, AtOrange,   /*  3 */
        AtDblue, AtPurple, AtCyan, AtGrey,  /*  7 */
        AtDgrey, AtRed, AtGreen, AtYellow,  /* 11 */
        AtBlue, AtPink, AtLblue, AtWhite,   /* 15 */

        AtRed + AtBlink, AtGrey, AtGrey, AtLblue,  /* 19 */
        AtGreen, AtLblue, AtWhite, AtGrey,  /* 23 */
        AtGrey, AtYellow, AtGrey, AtGrey,   /* 27 */
        AtGrey, AtBlue, AtGrey, AtGrey, /* 31 */
        AtDgreen, AtCyan, AtGrey, AtGrey,   /* 35 */
        AtBlue, AtBlue, AtGrey, AtGrey, /* 39 */
        AtWhite, AtDgrey, AtGreen, AtPink,  /* 43 */
        AtGrey, AtGrey, AtYellow, AtGrey,   /* 47 */
        AtGrey, AtOrange, AtBlue, AtRed,    /* 51 */
        AtGrey, AtGrey, AtGreen, AtDgreen,  /* 55 */
        AtDgreen, AtPurple, AtGrey, AtRed,  /* 59 */
        AtGrey, AtDgreen, AtRed, AtBlue,    /* 63 */
        AtRed, AtCyan, AtYellow, AtPink,    /* 67 */
        AtDgreen, AtPink, AtWhite, AtBlue,  /* 71 */
        AtBlue, AtBlue, AtGreen, AtGrey,    /* 75 */
        AtGreen, AtGreen, AtYellow, AtDgrey,    /* 79 */
        AtGreen, AtPink, AtDgreen, AtCyan,  /* 83 */
        AtRed, AtWhite, AtBlue, AtDgreen,   /* 87 */
        AtCyan, AtBlood, AtRed, AtDgreen,   /* 91 */
        AtPink, AtCyan, AtPink, AtYellow,   /* 95 */
        AtBlue, AtDgreen
};

const char *const valid_color[] = {
        "black",
        "dred",
        "dgreen",
        "orange",
        "dblue",
        "purple",
        "cyan",
        "grey",
        "dgrey",
        "red",
        "green",
        "yellow",
        "blue",
        "pink",
        "lblue",
        "white",
        "\0"
};

/* Color align functions by Justice@Aaern */
int const_color_str_len(const char *argument)
{
        int       str, count = 0;
        bool      IsColor = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else
                {
                        if (IsColor == FALSE)
                                count++;
                        else
                                IsColor = FALSE;
                }
        }

        return count;
}

int const_color_strnlen(const char *argument, int maxlength)
{
        int       str, count = 0;
        bool      IsColor = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else
                {
                        if (IsColor == FALSE)
                                count++;
                        else
                                IsColor = FALSE;
                }

                if (count >= maxlength)
                        break;
        }
        if (count < maxlength)
                return ((str - count) + maxlength);

        str++;
        return str;
}

int color_str_len(char *argument)
{
        int       str, count = 0;
        bool      IsColor = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else
                {
                        if (IsColor == FALSE)
                                count++;
                        else
                                IsColor = FALSE;
                }
        }

        return count;
}

int color_strnlen(char *argument, int maxlength)
{
        int       str, count = 0;
        bool      IsColor = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IsColor == TRUE)
                        {
                                count++;
                                IsColor = FALSE;
                        }
                        else
                                IsColor = TRUE;
                }
                else
                {
                        if (IsColor == FALSE)
                                count++;
                        else
                                IsColor = FALSE;
                }

                if (count >= maxlength)
                        break;
        }
        if (count < maxlength)
                return ((str - count) + maxlength);

        str++;
        return str;
}

const char *const_color_align(const char *argument, int size, int align)
{
        int       space = (size - const_color_str_len(argument));
        static char buf[MaxStringLength];

        if (align == AlignRight || const_color_str_len(argument) >= size)
                snprintf(buf, MSL, "%*.*s",
                         const_color_strnlen(argument, size),
                         const_color_strnlen(argument, size), argument);
        else if (align == AlignCenter)
                snprintf(buf, MSL, "%*s%s%*s", (space / 2), "", argument,
                         ((space / 2) * 2) ==
                         space ? (space / 2) : ((space / 2) + 1), "");
        else
                snprintf(buf, MSL, "%s%*s", argument, space, "");

        return buf;
}

char     *color_align(char *argument, int size, int align)
{
        int       space = (size - color_str_len(argument));
        static char buf[MaxStringLength];

        if (align == AlignRight || color_str_len(argument) >= size)
                snprintf(buf, MSL, "%*.*s", color_strnlen(argument, size),
                         color_strnlen(argument, size), argument);
        else if (align == AlignCenter)
                snprintf(buf, MSL, "%*s%s%*s", (space / 2), "", argument,
                         ((space / 2) * 2) ==
                         space ? (space / 2) : ((space / 2) + 1), "");
        else if (align == AlignLeft)
                snprintf(buf, MSL, "%s%*s", argument, space, "");

        return buf;
}

void show_colors(CharData * ch)
{
        sh_int    count;

        send_to_pager_color
                ("&BSyntax: color [color type] [color] | default\n\r", ch);
        send_to_pager_color
                ("&BSyntax: color _reset_ (Resets all colors to default set)\n\r",
                 ch);
        send_to_pager_color
                ("&BSyntax: color _all_ [color] (Sets all color types to [color])\n\r\n\r",
                 ch);

        send_to_pager_color
                ("&W********************************[ COLORS ]*********************************\n\r",
                 ch);

        for (count = 0; count < 16; ++count)
        {
                if ((count % 8) == 0 && count != 0)
                {
                        send_to_pager("\n\r", ch);
                }
                pager_printf(ch, "%s%-10s", color_str(count, ch),
                             pc_displays[count]);
        }

        send_to_pager
                ("\n\r\n\r&W******************************[ COLOR TYPES ]******************************\n\r",
                 ch);

        for (count = 16; count < MaxColors; ++count)
        {
                if ((count % 8) == 0 && count != 16)
                {
                        send_to_pager("\n\r", ch);
                }
                pager_printf(ch, "%s%-10s%s", color_str(count, ch),
                             pc_displays[count], AnsiReset);
        }
        send_to_pager("\n\r\n\r", ch);
        send_to_pager("&YAvailable colors are:\n\r", ch);

        for (count = 0; valid_color[count][0] != '\0'; ++count)
        {
                if ((count % 8) == 0 && count != 0)
                        send_to_pager("\n\r", ch);

                pager_printf(ch, "%s%-10s", color_str(AtPlain, ch),
                             valid_color[count]);
        }
        send_to_pager("\n\r", ch);
        return;
}

CMDF do_color(CharData * ch, char *argument)
{
        bool      dMatch, cMatch;
        sh_int    count = 0, y = 0;
        char      arg[MIL];
        char      arg2[MIL];
        char      arg3[MIL];
        char      buf[MSL];

        dMatch = FALSE;
        cMatch = FALSE;

        if (IsNpc(ch))
        {
                send_to_pager("Only PC's can change colors.\n\r", ch);
                return;
        }

        if (!argument || argument[0] == '\0')
        {
                show_colors(ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "ansitest"))
        {
                snprintf(log_buf, MSL, "%sBlack\n\r", AnsiBlack);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Red\n\r", AnsiDred);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Green\n\r", AnsiDgreen);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sOrange/Brown\n\r", AnsiOrange);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Blue\n\r", AnsiDblue);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPurple\n\r", AnsiPurple);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sCyan\n\r", AnsiCyan);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGrey\n\r", AnsiGrey);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Grey\n\r", AnsiDgrey);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sRed\n\r", AnsiRed);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGreen\n\r", AnsiGreen);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sYellow\n\r", AnsiYellow);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sBlue\n\r", AnsiBlue);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPink\n\r", AnsiPink);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sLight Blue\n\r", AnsiLblue);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sWhite\n\r", AnsiWhite);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sBlack\n\r", BlinkBlack);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Red\n\r", BlinkDred);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Green\n\r", BlinkDgreen);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sOrange/Brown\n\r", BlinkOrange);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Blue\n\r", BlinkDblue);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPurple\n\r", BlinkPurple);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sCyan\n\r", BlinkCyan);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGrey\n\r", BlinkGrey);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Grey\n\r", BlinkDgrey);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sRed\n\r", BlinkRed);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGreen\n\r", BlinkGreen);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sYellow\n\r", BlinkYellow);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sBlue\n\r", BlinkBlue);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPink\n\r", BlinkPink);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sLight Blue\n\r", BlinkLblue);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sWhite\n\r", BlinkWhite);
                write_to_buffer(ch->desc, log_buf, 0);
                write_to_buffer(ch->desc, AnsiReset, 0);
                snprintf(log_buf, MSL, "%s%sBlack\n\r", AnsiWhite,
                         BackBlack);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sDark Red\n\r", AnsiBlack,
                         BackDred);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sDark Green\n\r", AnsiBlack,
                         BackDgreen);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sOrange/Brown\n\r", AnsiBlack,
                         BackOrange);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sDark Blue\n\r", AnsiBlack,
                         BackDblue);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sPurple\n\r", AnsiBlack,
                         BackPurple);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sCyan\n\r", AnsiBlack, BackCyan);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sGrey\n\r", AnsiBlack, BackGrey);
                write_to_buffer(ch->desc, log_buf, 0);
                write_to_buffer(ch->desc, AnsiReset, 0);

                return;
        }

        if (!str_prefix(arg, "_reset_"))
        {
                reset_colors(ch);
                send_to_pager("All color types reset to default colors.\n\r",
                              ch);
                return;
        }

        argument = one_argument(argument, arg2);

        if (arg[0] == '\0')
        {
                send_to_char("Change which color type?\r\n", ch);
                return;
        }

        argument = one_argument(argument, arg3);

        if (!str_prefix(arg, "_all_"))
        {
                dMatch = TRUE;
                count = -1;

                /*
                 * search for a Valid color setting
                 */
                for (y = 0; y < 16; y++)
                {
                        if (!str_cmp(arg2, valid_color[y]))
                        {
                                cMatch = TRUE;
                                break;
                        }
                }
        }
        else if (arg2[0] == '\0')
        {
                cMatch = FALSE;
        }
        else
        {
                /*
                 * search for the display type and str_cmp
                 */
                for (count = 0; count < MaxColors; count++)
                {
                        if (!str_prefix(arg, pc_displays[count]))
                        {
                                dMatch = TRUE;
                                break;
                        }
                }

                if (!dMatch)
                {
                        ch_printf(ch, "%s is an invalid color type.\n\r",
                                  arg);
                        send_to_char
                                ("Type color with no arguments to see available options.\n\r",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "default"))
                {
                        ch->colors[count] = default_set[count];
                        snprintf(buf, MSL,
                                 "Display %s set back to default.\n\r",
                                 pc_displays[count]);
                        send_to_pager(buf, ch);
                        return;
                }

                /*
                 * search for a Valid color setting
                 */
                for (y = 0; y < 16; y++)
                {
                        if (!str_cmp(arg2, valid_color[y]))
                        {
                                cMatch = TRUE;
                                break;
                        }
                }
        }

        if (!cMatch)
        {
                if (arg[0])
                {
                        ch_printf(ch, "Invalid color for type %s.\n", arg);
                }
                else
                {
                        send_to_pager("Invalid color.\n\r", ch);
                }

                send_to_pager("Choices are:\n\r", ch);

                for (count = 0; count < 16; count++)
                {
                        if (count % 5 == 0 && count != 0)
                                send_to_pager("\r\n", ch);

                        pager_printf(ch, "%-10s", valid_color[count]);
                }

                pager_printf(ch, "%-10s\r\n", "default");
                return;
        }
        else
        {
                snprintf(buf, MSL, "Color type %s set to color %s.\n\r",
                         count == -1 ? "_all_" : pc_displays[count],
                         valid_color[y]);
        }

        if (!str_cmp(arg3, "blink"))
        {
                y += AtBlink;
        }

        if (count == -1)
        {
                int       ccount;

                for (ccount = 0; ccount < MaxColors; ++ccount)
                {
                        ch->colors[ccount] = y;
                }

                set_pager_color(y, ch);

                snprintf(buf, MSL, "All color types set to color %s%s.%s\n\r",
                         valid_color[y > AtBlink ? y - AtBlink : y],
                         y > AtBlink ? " [BLINKING]" : "", AnsiReset);

                send_to_pager(buf, ch);
        }
        else
        {
                ch->colors[count] = y;

                set_pager_color(count, ch);

                if (!str_cmp(arg3, "blink"))
                        snprintf(buf, MSL,
                                 "Display %s set to color %s [BLINKING]%s\n\r",
                                 pc_displays[count],
                                 valid_color[y - AtBlink], AnsiReset);
                else
                        snprintf(buf, MSL, "Display %s set to color %s.\n\r",
                                 pc_displays[count], valid_color[y]);

                send_to_pager(buf, ch);
        }
        set_pager_color(AtPlain, ch);

        return;
}

void reset_colors(CharData * ch)
{
        memcpy(&ch->colors, &default_set, sizeof(default_set));
}

const char *color_str(sh_int AType, CharData * ch)
{
        if (!ch)
        {
                bug("%s", "color_str: NULL ch!");
                return ("");
        }

        if (IsNpc(ch) || !IsSet(ch->act, PlrAnsi))
                return ("");

        switch (ch->colors[AType])
        {
        case 0:
                return (AnsiBlack);
                break;
        case 1:
                return (AnsiDred);
                break;
        case 2:
                return (AnsiDgreen);
                break;
        case 3:
                return (AnsiOrange);
                break;
        case 4:
                return (AnsiDblue);
                break;
        case 5:
                return (AnsiPurple);
                break;
        case 6:
                return (AnsiCyan);
                break;
        case 7:
                return (AnsiGrey);
                break;
        case 8:
                return (AnsiDgrey);
                break;
        case 9:
                return (AnsiRed);
                break;
        case 10:
                return (AnsiGreen);
                break;
        case 11:
                return (AnsiYellow);
                break;
        case 12:
                return (AnsiBlue);
                break;
        case 13:
                return (AnsiPink);
                break;
        case 14:
                return (AnsiLblue);
                break;
        case 15:
                return (AnsiWhite);
                break;

                /*
                 * 16 thru 31 are for blinking colors 
                 */
        case 16:
                return (BlinkBlack);
                break;
        case 17:
                return (BlinkDred);
                break;
        case 18:
                return (BlinkDgreen);
                break;
        case 19:
                return (BlinkOrange);
                break;
        case 20:
                return (BlinkDblue);
                break;
        case 21:
                return (BlinkPurple);
                break;
        case 22:
                return (BlinkCyan);
                break;
        case 23:
                return (BlinkGrey);
                break;
        case 24:
                return (BlinkDgrey);
                break;
        case 25:
                return (BlinkRed);
                break;
        case 26:
                return (BlinkGreen);
                break;
        case 27:
                return (BlinkYellow);
                break;
        case 28:
                return (BlinkBlue);
                break;
        case 29:
                return (BlinkPink);
                break;
        case 30:
                return (BlinkLblue);
                break;
        case 31:
                return (BlinkWhite);
                break;

        default:
                return (AnsiReset);
                break;
        }
}

int colorcode(const char *col, char *code, CharData * ch)
{
        const char *ctype = col;
        int       ln;
        bool      ansi;


        if (!ch || ch->gold == 0)
                ansi = TRUE;
        else
                ansi = (!IsNpc(ch) && IsSet(ch->act, PlrAnsi));

        col++;

        if (!*col)
                ln = -1;
#ifdef OVERLANDCODE
        else if (*ctype != '&' && *ctype != '{' && *ctype != '}')
        {
                bug("colorcode: command '%c' not '&', '{' or '}'", *ctype);
                ln = -1;
        }
#else
        else if (*ctype != '&' && *ctype != '^' && *ctype != '}')
        {
                bug("colorcode: command '%c' not '&', '^' or '}'", *ctype);
                ln = -1;
        }
#endif
        else if (*col == *ctype)
        {
                code[0] = *col;
                code[1] = '\0';
                ln = 1;
        }
        else if (!ansi)
                ln = 0;
        else
        {
                /*
                 * Foreground text - non-blinking 
                 */
                if (*ctype == '&')
                {
                        switch (*col)
                        {
                        default:
                                code[0] = *ctype;
                                code[1] = *col;
                                code[2] = '\0';
                                return 2;
                        case 'i':  /* Italic text */
                        case 'I':
                                mudstrlcpy(code, AnsiItalic, 20);
                                break;
                        case 'v':  /* Reverse colors */
                        case 'V':
                                mudstrlcpy(code, AnsiReverse, 20);
                                break;
                        case 'u':  /* Underline */
                        case 'U':
                                mudstrlcpy(code, AnsiUnderline, 20);
                                break;
                        case 's':  /* Strikeover */
                        case 'S':
                                mudstrlcpy(code, AnsiStrikeout, 20);
                                break;
                        case 'd':  /* Player's Client default color */
                                mudstrlcpy(code, AnsiReset, 20);
                                break;
                        case 'D':  /* Reset to custom color for whatever is being displayed */
                                mudstrlcpy(code, AnsiReset, 20);   /* Yes, this reset here is quite necessary to cancel out other things */
                                mudstrlcat(code,
                                           color_str(ch->desc->PageColor, ch),
                                           20);
                                break;
                        case 'x':  /* Black */
                                mudstrlcpy(code, AnsiBlack, 20);
                                break;
                        case 'O':  /* Orange/Brown */
                                mudstrlcpy(code, AnsiOrange, 20);
                                break;
                        case 'c':  /* Cyan */
                                mudstrlcpy(code, AnsiCyan, 20);
                                break;
                        case 'z':  /* Dark Grey */
                                mudstrlcpy(code, AnsiDgrey, 20);
                                break;
                        case 'g':  /* Dark Green */
                                mudstrlcpy(code, AnsiDgreen, 20);
                                break;
                        case 'G':  /* Light Green */
                                mudstrlcpy(code, AnsiGreen, 20);
                                break;
                        case 'P':  /* Pink/Light Purple */
                                mudstrlcpy(code, AnsiPink, 20);
                                break;
                        case 'r':  /* Dark Red */
                                mudstrlcpy(code, AnsiDred, 20);
                                break;
                        case 'b':  /* Dark Blue */
                                mudstrlcpy(code, AnsiDblue, 20);
                                break;
                        case 'w':  /* Grey */
                                mudstrlcpy(code, AnsiGrey, 20);
                                break;
                        case 'Y':  /* Yellow */
                                mudstrlcpy(code, AnsiYellow, 20);
                                break;
                        case 'C':  /* Light Blue */
                                mudstrlcpy(code, AnsiLblue, 20);
                                break;
                        case 'p':  /* Purple */
                                mudstrlcpy(code, AnsiPurple, 20);
                                break;
                        case 'R':  /* Red */
                                mudstrlcpy(code, AnsiRed, 20);
                                break;
                        case 'B':  /* Blue */
                                mudstrlcpy(code, AnsiBlue, 20);
                                break;
                        case 'W':  /* White */
                                mudstrlcpy(code, AnsiWhite, 20);
                                break;
                        }
                }
                /*
                 * Foreground text - blinking 
                 */
                if (*ctype == '}')
                {
                        switch (*col)
                        {
                        default:
                                code[0] = *ctype;
                                code[1] = *col;
                                code[2] = '\0';
                                return 2;
                        case 'x':  /* Black */
                                mudstrlcpy(code, BlinkBlack, 20);
                                break;
                        case 'O':  /* Orange/Brown */
                                mudstrlcpy(code, BlinkOrange, 20);
                                break;
                        case 'c':  /* Cyan */
                                mudstrlcpy(code, BlinkCyan, 20);
                                break;
                        case 'z':  /* Dark Grey */
                                mudstrlcpy(code, BlinkDgrey, 20);
                                break;
                        case 'g':  /* Dark Green */
                                mudstrlcpy(code, BlinkDgreen, 20);
                                break;
                        case 'G':  /* Light Green */
                                mudstrlcpy(code, BlinkGreen, 20);
                                break;
                        case 'P':  /* Pink/Light Purple */
                                mudstrlcpy(code, BlinkPink, 20);
                                break;
                        case 'r':  /* Dark Red */
                                mudstrlcpy(code, BlinkDred, 20);
                                break;
                        case 'b':  /* Dark Blue */
                                mudstrlcpy(code, BlinkDblue, 20);
                                break;
                        case 'w':  /* Grey */
                                mudstrlcpy(code, BlinkGrey, 20);
                                break;
                        case 'Y':  /* Yellow */
                                mudstrlcpy(code, BlinkYellow, 20);
                                break;
                        case 'C':  /* Light Blue */
                                mudstrlcpy(code, BlinkLblue, 20);
                                break;
                        case 'p':  /* Purple */
                                mudstrlcpy(code, BlinkPurple, 20);
                                break;
                        case 'R':  /* Red */
                                mudstrlcpy(code, BlinkRed, 20);
                                break;
                        case 'B':  /* Blue */
                                mudstrlcpy(code, BlinkBlue, 20);
                                break;
                        case 'W':  /* White */
                                mudstrlcpy(code, BlinkWhite, 20);
                                break;
                        }
                }
                /*
                 * Background color 
                 */
#ifdef OVERLANDCODE
                if (*ctype == '{')
#else
                if (*ctype == '^')
#endif
                {
                        switch (*col)
                        {
                        default:
                                code[0] = *ctype;
                                code[1] = *col;
                                code[2] = '\0';
                                return 2;
                        case 'x':  /* Black */
                                mudstrlcpy(code, BackBlack, 20);
                                break;
                        case 'r':  /* Dark Red */
                                mudstrlcpy(code, BackDred, 20);
                                break;
                        case 'g':  /* Dark Green */
                                mudstrlcpy(code, BackDgreen, 20);
                                break;
                        case 'O':  /* Orange/Brown */
                                mudstrlcpy(code, BackOrange, 20);
                                break;
                        case 'b':  /* Dark Blue */
                                mudstrlcpy(code, BackDblue, 20);
                                break;
                        case 'p':  /* Purple */
                                mudstrlcpy(code, BackPurple, 20);
                                break;
                        case 'c':  /* Cyan */
                                mudstrlcpy(code, BackCyan, 20);
                                break;
                        case 'w':  /* Grey */
                                mudstrlcpy(code, BackGrey, 20);
                                break;
                        }
                }
                ln = static_cast<int>(strlen(code));
        }
        if (ln <= 0)
                *code = '\0';
        return ln;
}

/* Moved from comm.c */
void set_char_color(int AType, CharData * ch)
{
        if (!ch || !ch->desc)
                return;

        write_to_buffer(ch->desc, color_str(static_cast<sh_int>(AType), ch), 0);
        ch->desc->PageColor = static_cast<char>(ch->colors[AType]);
}

void set_pager_color(sh_int AType, CharData * ch)
{
        if (!ch || !ch->desc)
                return;

        write_to_pager(ch->desc, color_str(AType, ch), 0);
        ch->desc->PageColor = static_cast<char>(ch->colors[AType]);
}

void write_to_pager(DescriptorData * d, const char *txt, int length)
{
        int       pageroffset;  /* Pager fix by thoric */
        int       origlength = 0;

        if (length <= 0)
                length = static_cast<int>(strlen(txt));

        /*
         * Find length in case caller didn't. 
         */
        if (length == 0)
                return;

        origlength = length;
        /*
         * How much space do we need to expand stuff 
         */
        length += count_mxp_tags(d, txt, length);

        if (!d->PageBuf)
        {
                d->PageSize = MaxStringLength;
                CREATE(d->PageBuf, char, static_cast<size_t>(d->PageSize));
        }
        if (!d->PagePoint)
        {
                d->PagePoint = d->PageBuf;
                d->PageTop = 0;
                d->PageCmd = '\0';
        }
        if (d->PageTop == 0 && !d->fcommand)
        {
                d->PageBuf[0] = '\n';
                d->PageBuf[1] = '\r';
                d->PageTop = 2;
        }
        pageroffset = static_cast<int>(d->PagePoint - d->PageBuf);    /* pager fix (goofup fixed 08/21/97) */
        while (d->PageTop + length >= d->PageSize)
        {
                if (d->PageSize > MSL * 16)
                {
                        bug("%s", "Pager overflow.  Ignoring.\n\r");
                        d->PageTop = 0;
                        d->PagePoint = NULL;
                        DISPOSE(d->PageBuf);
                        d->PageSize = MSL;
                        return;
                }
                d->PageSize *= 2;
                _Pragma("GCC diagnostic push")
                _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"")
                RECREATE(d->PageBuf, char, static_cast<size_t>(d->PageSize));
                _Pragma("GCC diagnostic pop")
        }
        d->PagePoint = d->PageBuf + pageroffset;    /* pager fix (goofup fixed 08/21/97) */
/*   mudstrlcpy( d->PageBuf + d->PageTop, txt, length ); */
        convert_mxp_tags(d, d->PageBuf + d->PageTop, txt, origlength);
        d->PageTop += length;
        d->PageBuf[d->PageTop] = '\0';
        return;
}

/* Writes to a descriptor, usually best used when there's no character to send to ( like logins ) */
void send_to_desc_color(const char *txt, DescriptorData * d)
{
        char     *colstr;
        const char *prevstr = txt;
        char      colbuf[20];
        int       ln;

        if (!d)
        {
                bug("%s", "send_to_desc_color: NULL *d");
                return;
        }

        if (!txt || !d->descriptor)
                return;

        while ((colstr = strpbrk(StringLiteral(prevstr), "&^}")) != NULL)
        {
                if (colstr > prevstr)
                        write_to_buffer(d, prevstr, static_cast<int>(colstr - prevstr));

                ln = colorcode(colstr, colbuf, d->character);
                if (ln < 0)
                {
                        prevstr = colstr + 1;
                        break;
                }

                else if (ln > 0)
                        write_to_buffer(d, colbuf, ln);
                prevstr = colstr + 2;
        }
        if (*prevstr)
                write_to_buffer(d, prevstr, 0);

        return;
}

/*
 * Write to one char. Convert color into ANSI sequences.
 */
void send_to_char_color(const char *txt, CharData * ch)
{
        char     *colstr;
        const char *prevstr = txt;
        char      colbuf[20];
        int       ln;

        if (!ch)
        {
                bug("%s", "send_to_char_color: NULL ch!");
                return;
        }

        if (txt && ch->desc)
        {
#ifdef OVERLANDCODE
                while ((colstr = strpbrk(StringLiteral(prevstr), "&{}")) != NULL)
#else
                while ((colstr = strpbrk(StringLiteral(prevstr), "&^}")) != NULL)
#endif
                {
                        if (colstr > prevstr)
                                write_to_buffer(ch->desc, prevstr,
                                                static_cast<int>(colstr - prevstr));
                        if (!ch->desc)
                                return;
                        ln = colorcode(colstr, colbuf, ch);
                        if (ln < 0)
                        {
                                prevstr = colstr + 1;
                                break;
                        }
                        else if (ln > 0)
                                write_to_buffer(ch->desc, colbuf, ln);
                        prevstr = colstr + 2;
                }
                if (*prevstr)
                        write_to_buffer(ch->desc, prevstr, 0);
        }
        return;
}

void send_to_pager_color(const char *txt, CharData * ch)
{
        char     *colstr;
        const char *prevstr = txt;
        char      colbuf[20];
        int       ln;

        if (IsNpc(ch)) /* NPCs can't do pager */
                send_to_char_color(txt, ch);

        if (!ch)
        {
                bug("%s", "send_to_pager_color: NULL ch!");
                return;
        }

        if (txt && ch->desc)
        {
                DescriptorData *d = ch->desc;

                ch = d->original ? d->original : d->character;
                if (IsNpc(ch) || !IsSet(ch->pcdata->flags, PcflagPageron))
                {
                        send_to_char_color(txt, d->character);
                        return;
                }
#ifdef OVERLANDCODE
                while ((colstr = strpbrk(StringLiteral(prevstr), "&{}")) != NULL)
#else
                while ((colstr = strpbrk(StringLiteral(prevstr), "&^}")) != NULL)
#endif
                {
                        if (colstr > prevstr)
                                write_to_pager(ch->desc, prevstr,
                                               static_cast<int>(colstr - prevstr));
                        ln = colorcode(colstr, colbuf, ch);
                        if (ln < 0)
                        {
                                prevstr = colstr + 1;
                                break;
                        }
                        else if (ln > 0)
                                write_to_pager(ch->desc, colbuf, ln);
                        prevstr = colstr + 2;
                }
                if (*prevstr)
                        write_to_pager(ch->desc, prevstr, 0);
        }
        return;
}

void send_to_char(const char *txt, CharData * ch)
{
        send_to_char_color(txt, ch);
        return;
}

void send_to_pager(const char *txt, CharData * ch)
{
        if (IsNpc(ch))
                send_to_char_color(txt, ch);
        else
                send_to_pager_color(txt, ch);
        return;
}

void ch_printf(CharData * ch, char *fmt, ...)
{
        char      buf[MSL * 4];
        va_list   args;

        va_start(args, fmt);
        vsnprintf(buf, MSL * 4, fmt, args);
        va_end(args);

        send_to_char_color(buf, ch);
}

void pager_printf(CharData * ch, char *fmt, ...)
{
        char      buf[MSL * 2];
        va_list   args;

        va_start(args, fmt);
        vsnprintf(buf, MSL, fmt, args);
        va_end(args);

        send_to_pager_color(buf, ch);
}
