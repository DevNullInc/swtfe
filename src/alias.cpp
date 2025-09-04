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
 *         Command alias system for creating shortcuts and custom player commands.       *
 *****************************************************************************************/

// ============================================================================
// System Includes
// ============================================================================
#include <ctype.h>
#include <string.h>

// ============================================================================
// Local Includes
// ============================================================================
#include "mud.hpp"
#include "editor.hpp"
#include "account.hpp"
#include "alias.hpp"

// ============================================================================
// Constants and Configuration
// ============================================================================
namespace {
    // Alias system limits
    constexpr int MAX_ALIAS_ARGS = 9;           // Maximum $1-$9 substitutions
    constexpr int MAX_ALIAS_RECURSION = 10;    // Prevent infinite loops
    
    // String constants
    constexpr const char* ALIAS_SYNTAX = 
        "Syntax:\n\r"
        "\talias list\n\r"
        "\talias <alias name> create\n\r"
        "\talias <alias name> edit\n\r"
        "\talias <alias name> delete\n\r"
        "\talias <alias name> show\n\r";
        
    constexpr const char* NO_ALIASES_MSG = "\t&GYou have no aliases.\n\r";
    constexpr const char* ALIAS_LIST_HEADER = "Your Aliases:\n\r";
    constexpr const char* ALIAS_EXISTS_MSG = "That alias already exists";
    constexpr const char* ALIAS_NOT_EXISTS_MSG = "That alias doesn't exist";
    constexpr const char* ALIAS_CREATED_MSG = "Created.\n\r";
    constexpr const char* ALIAS_DELETED_MSG = "Deleted.\n\r";
    constexpr const char* ALIAS_EMPTY_MSG = "Alias command empty.\n\r";
    constexpr const char* INVALID_COMMAND_MSG = "What....\n\r";
}

// ============================================================================
// Forward Declarations
// ============================================================================
void stop_idling args((CHAR_DATA * ch));

// ============================================================================
// Section: String Parsing Utilities
// ============================================================================

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes and handles line-based parsing for aliases.
 */
char *one_line(char *argument, char *arg_first)
{
        char cEnd;

        while (isspace(*argument) || *argument == '\n' || *argument == '\r')
                argument++;

        cEnd = '\n';

        while (*argument != '\0')
        {
                if (*argument == cEnd)
                {
                        argument++;
                        break;
                }
                *arg_first = *argument;
                arg_first++;
                argument++;
        }
        *arg_first = '\0';
        
        while (isspace(*argument) || *argument == '\n' || *argument == '\r')
                argument++;

        return argument;
}

// ============================================================================
// Section: Alias Lookup and Management
// ============================================================================

ALIAS_DATA *get_alias(CHAR_DATA *ch, char *argument)
{
        ALIAS_DATA *ali;

        if (!ch || !argument || IS_NPC(ch))
                return nullptr;

        // First try exact match
        for (ali = ch->pcdata->account->first_alias; ali; ali = ali->next)
        {
                if (!str_cmp(argument, ali->name))
                        return ali;
        }

        // Then try prefix match
        for (ali = ch->pcdata->account->first_alias; ali; ali = ali->next)
        {
                if (!str_prefix(argument, ali->name))
                        return ali;
        }

        return nullptr;
}

void free_alias(ACCOUNT_DATA *account, ALIAS_DATA *alias)
{
        STRFREE(alias->name);
        STRFREE(alias->cmd);
        UNLINK(alias, account->first_alias, account->last_alias, next, prev);
        DISPOSE(alias);
}

void free_aliases(ACCOUNT_DATA *account)
{
        ALIAS_DATA *alias, *al_next;

        if (!account)
                return;

        for (alias = account->first_alias; alias; alias = al_next)
        {
                al_next = alias->next;
                free_alias(account, alias);
        }
}

// ============================================================================
// Section: Alias Command Interface
// ============================================================================

CMDF do_alias(CHAR_DATA *ch, char *argument)
{
        ALIAS_DATA *alias;
        char cmd[MSL];
        char arg[MSL];

        if (!ch || !argument || IS_NPC(ch))
                return;

        CHECK_SUBRESTRICTED(ch);

        switch (ch->substate)
        {
        default:
                break;

        case SUB_ALIASMSG:
                alias = (ALIAS_DATA *)ch->dest_buf;
                if (!alias)
                {
                        bug("%s has no alias coming out of edit");
                        return;
                }
                if (alias->cmd)
                        STRFREE(alias->cmd);
                alias->cmd = copy_buffer(ch);
                stop_editing(ch);
                ch->substate = static_cast<sh_int>(ch->tempnum);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char(ALIAS_SYNTAX, ch);
                return;
        }

        if (!str_cmp(argument, "list"))
        {
                int count = 0;

                send_to_char(ALIAS_LIST_HEADER, ch);
                if (!ch->pcdata->account->first_alias)
                {
                        send_to_char(NO_ALIASES_MSG, ch);
                        return;
                }

                for (alias = ch->pcdata->account->first_alias; alias; alias = alias->next)
                {
                        count++;
                        ch_printf(ch, "\t&G%s\n\r", alias->name);
                }
                ch_printf(ch, "\n\r&G%d&w alias%s listed.\n\r", count,
                          count > 1 ? "es" : "");
                return;
        }

        argument = one_argument(argument, cmd);
        argument = one_argument(argument, arg);

        if (cmd[0] == '\0' || arg[0] == '\0')
        {
                do_alias(ch, const_cast<char*>(""));
                return;
        }

        if (!str_cmp(arg, "create"))
        {
                for (alias = ch->pcdata->account->first_alias; alias; alias = alias->next)
                {
                        if (!str_cmp(argument, alias->name))
                                break;
                }
                if (alias != nullptr)
                {
                        send_to_char(ALIAS_EXISTS_MSG, ch);
                        return;
                }
                CREATE(alias, ALIAS_DATA, 1);
                alias->name = STRALLOC(cmd);
                alias->cmd = STRALLOC(const_cast<char*>(""));
                LINK(alias, ch->pcdata->account->first_alias,
                     ch->pcdata->account->last_alias, next, prev);
                send_to_char(ALIAS_CREATED_MSG, ch);

                // Now move to editing
                if (ch->substate == SUB_REPEATCMD)
                        ch->tempnum = SUB_REPEATCMD;
                else
                        ch->tempnum = SUB_NONE;
                ch->substate = SUB_ALIASMSG;
                ch->dest_buf = alias;
                start_editing(ch, alias->cmd);
                return;
        }

        if ((alias = get_alias(ch, cmd)) == nullptr)
        {
                send_to_char(ALIAS_NOT_EXISTS_MSG, ch);
                return;
        }

        if (!str_cmp(arg, "edit"))
        {
                if (ch->substate == SUB_REPEATCMD)
                        ch->tempnum = SUB_REPEATCMD;
                else
                        ch->tempnum = SUB_NONE;
                ch->substate = SUB_ALIASMSG;
                ch->dest_buf = alias;
                start_editing(ch, alias->cmd);
                return;
        }
        else if (!str_cmp(arg, "delete"))
        {
                free_alias(ch->pcdata->account, alias);
                send_to_char(ALIAS_DELETED_MSG, ch);
                return;
        }
        else if (!str_cmp(arg, "show"))
        {
                ch_printf(ch, "\n\rAlias Name: %s\n\r", alias->name);
                ch_printf(ch, "&RCommands: \n\r%s\n\r", alias->cmd);
                return;
        }
        else
        {
                send_to_char(INVALID_COMMAND_MSG, ch);
                do_alias(ch, const_cast<char*>(""));
                return;
        }
}

// ============================================================================
// Section: Alias Execution and Processing
// ============================================================================

bool check_alias(CHAR_DATA *ch, char *command, char *argument)
{
        ALIAS_DATA *alias;

        if (!ch || !command || IS_NPC(ch) || !ch->desc)
                return FALSE;
        if (!IS_PLAYING(ch->desc))
                return FALSE;
        if (!ch->pcdata->account->first_alias)
                return FALSE;

        if ((alias = get_alias(ch, command)) == nullptr)
                return FALSE;
                
        // Prevent recursion
        if (ch->substate == SUB_ALIAS)
                return FALSE;
                
        if (alias->cmd[0] == '\0')
        {
                send_to_char(ALIAS_EMPTY_MSG, ch);
                return TRUE;
        }
        
        mudstrlcpy(ch->desc->incomm, alias->cmd, MIL);
        
        if (strchr(ch->desc->incomm, '$'))
        {
                char arg[MIL];
                char temp[MIL];
                char *src = nullptr;
                char *p;
                int count = 0;

                argument = one_argument(argument, arg);
                while (arg[0] != '\0' && ++count < MAX_ALIAS_ARGS)
                {
                        src = ch->desc->incomm;
                        while (*src && *src != '\0')
                        {
                                if (*src == '$' && *(src + 1) == count + '0')
                                {
                                        p = src + strlen(arg) + 1;
                                        *src = '\0';
                                        mudstrlcpy(temp, src + 2, MIL);
                                        mudstrlcat(src, arg, MIL);
                                        mudstrlcat(src, temp, MIL);
                                        src = p;
                                        continue;
                                }
                                src++;
                        }
                        argument = one_argument(argument, arg);
                }
        }

        ch->substate = SUB_ALIAS;
        check_aliases(ch->desc);
        return TRUE;
}

bool check_aliases(DESCRIPTOR_DATA *d)
{
        char arg[MSL];
        char *rem = nullptr;
        size_t len = 0;
        CHAR_DATA *ch;

        if (!d || !d->character)
                return FALSE;
        if (!IS_PLAYING(d))
                return FALSE;
                
        ch = d->original ? d->original : d->character;
        if (IS_NPC(ch))
                return FALSE;
        if (!ch->pcdata->account->first_alias)
                return FALSE;
        if (d->character->substate != SUB_ALIAS)
                return FALSE;

        // Split on new line (memcopy it back)
        rem = one_line(d->incomm, arg);
        interpret(d->character, arg);
        
        // Check substate
        stop_idling(d->character);
        if (rem[0] == '\0')
        {
                d->incomm[0] = '\0';
                d->character->substate = SUB_NONE;
                return TRUE;
        }
        
        len = strlen(rem);
        memcpy(d->incomm, rem, (len + 1) * sizeof(char));
        return TRUE;
}

// ============================================================================
// Section: File I/O Operations
// ============================================================================

void fwrite_alias(ACCOUNT_DATA *account, FILE *fp)
{
        ALIAS_DATA *alias;

        if (!account)
                return;

        for (alias = account->first_alias; alias; alias = alias->next)
        {
                fprintf(fp, "#ALIAS\n");
                fprintf(fp, "Name %s~\n", alias->name);
                fprintf(fp, "Cmd %s~\n", alias->cmd);
                fprintf(fp, "End\n\n");
        }
}

void fread_alias(ACCOUNT_DATA *account, FILE *fp)
{
        const char *word;
        bool fMatch;
        ALIAS_DATA *alias;

        if (!account)
                return;

        CREATE(alias, ALIAS_DATA, 1);

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
                case 'C':
                        KEY("Cmd", alias->cmd, fread_string(fp));
                        break;
                case 'N':
                        KEY("Name", alias->name, fread_string(fp));
                        break;
                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!alias->name)
                                        DISPOSE(alias);
                                else
                                {
                                        if (!alias->cmd)
                                                alias->cmd = STRALLOC(const_cast<char*>(""));
                                        LINK(alias, account->first_alias,
                                             account->last_alias, next, prev);
                                }
                                return;
                        }
                        break;
                }

                if (!fMatch)
                {
                        bug("Fread_char: no match: %s", word);
                }
        }
}
