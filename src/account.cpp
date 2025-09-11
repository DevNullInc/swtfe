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
 *                               Account management                                      *
 *****************************************************************************************/


#include "mud.hpp"
#include "Account.hpp"
#include "alias.hpp"
#include "boards.hpp"
#include "imccfg.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

namespace Account {

constexpr std::size_t ACCOUNT_FILENAME_SIZE = 255;

ACCOUNT_DATA *first_account = nullptr;
ACCOUNT_DATA *last_account = nullptr;

// Function prototypes


// Function prototypes
int check_playing(DescriptorData* d, const std::string& name, bool kick);
bool check_reconnect(DescriptorData * d, char *name, bool fConn);
void fwrite_comments(ACCOUNT_DATA * Account, FILE * fp);
void fread_comment(ACCOUNT_DATA * Account, FILE * fp);
void fread_account(ACCOUNT_DATA * Account, FILE * fp);
ACCOUNT_DATA *get_account(const char *name);

// =============================================================================
// Account Creation and Management
// =============================================================================

ACCOUNT_DATA *create_account() noexcept
{
    ACCOUNT_DATA *Account = nullptr;

    CREATE(Account, ACCOUNT_DATA, 1);
    Account->rppoints = 0;
    Account->rpcurrent = -1;
    Account->qpoints = 0;
    Account->name = nullptr;
    Account->password = nullptr;
    Account->inuse = 1;
    Account->comments = nullptr;

    return Account;
}

void free_account(ACCOUNT_DATA *Account)
{
    if (!Account) {
        return;
    }

    if (--Account->inuse > 0) {
        return;
    }

    if (Account->name) {
        STRFREE(Account->name);
    }

    if (Account->password) {
        STRFREE(Account->password);
    }

    for (std::size_t count = 0; count < MaxCharacters; ++count) {
        if (Account->character[count]) {
            STRFREE(Account->character[count]);
        }
    }

    NOTE_DATA *pnote = Account->comments;
    while (pnote) {
        NOTE_DATA *next_note = pnote->next;
        free_note(pnote);
        pnote = next_note;
    }

    free_aliases(Account);
    UNLINK(Account, first_account, last_account, next, prev);
    DISPOSE(Account);
}

void save_account(ACCOUNT_DATA *Account)
{
    if (!Account) {
        bug("Save_account: null Account!", 0);
        return;
    }

    char accountsave[MIL];
    snprintf(accountsave, MIL, "%s%c/%s.Account", ACCOUNT_DIR, tolower(Account->name[0]), capitalize(Account->name));

    FILE *fp = fopen(accountsave, "w");
    if (!fp) {
        bug("save_account: fopen", 0);
        perror(accountsave);
        return;
    }

    fprintf(fp, "#%s\n", "ACCOUNT");
    fprintf(fp, "Name      %s~\n", Account->name);
    fprintf(fp, "Password  %s~\n", Account->password);
    fprintf(fp, "Email     %s~\n", Account->email);
    fprintf(fp, "RPpoints  %ld\n", Account->rppoints);
    fprintf(fp, "RPcurrent %ld\n", Account->rpcurrent);
    fprintf(fp, "Qpoints   %ld\n", Account->qpoints);

    for (std::size_t count = 0; count < MaxCharacters; ++count) {
        if (Account->character[count]) {
            fprintf(fp, "Character %s~\n", Account->character[count]);
        }
    }

    fwrite_alias(Account, fp);
    if (Account->comments) {
        fwrite_comments(Account, fp);
    }

    fprintf(fp, "#END\n");
    FCLOSE(fp);
}

ACCOUNT_DATA *get_account(const char *name)
{
    for (ACCOUNT_DATA *Account = first_account; Account; Account = Account->next) {
        if (!Account->name) {
            bug("Account with invalid Name", 0);
            free_account(Account);
        } else if (!str_cmp(Account->name, name)) {
            return Account;
        }
    }
    return nullptr;
}

ACCOUNT_DATA *load_account(const char *name)
{
    ACCOUNT_DATA *Account = get_account(name);
    if (Account) {
        ++Account->inuse;
        return Account;
    }

    char accountsave[MIL];
    snprintf(accountsave, MIL, "%s%c/%s.Account", ACCOUNT_DIR, tolower(name[0]), capitalize(name));

    FILE *fp = fopen(accountsave, "r");
    if (!fp) {
        return nullptr;
    }

    for (;;) {
        char letter = fread_letter(fp);
        if (letter == '*') {
            fread_to_eol(fp);
            continue;
        }

        if (letter != '#') {
            bug("Load_account_file: # not found.", 0);
            break;
        }

        char *word = fread_word(fp);
        if (!str_cmp(word, "ACCOUNT")) {
            Account = create_account();
            fread_account(Account, fp);
        } else if (!str_cmp(word, "COMMENT")) {
            fread_comment(Account, fp);
        } else if (!str_cmp(word, "ALIAS")) {
            fread_alias(Account, fp);
        } else if (!str_cmp(word, "END")) {
            break;
        } else {
            bug("Load_account_file: bad section.", 0);
            break;
        }
    }

    FCLOSE(fp);

    if (Account) {
        LINK(Account, first_account, last_account, next, prev);
    }

    return Account;
}

// =============================================================================
// Account Commands
// =============================================================================

CMDF do_showalts(CharData * ch, char *argument)
{
        CharData *victim;
        ACCOUNT_DATA *Account = NULL;
        int       count;

        if (argument[0] == '\0')
        {
                send_to_char("&Bs&zhowalts <&wchar&B|&waccount&z>\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, argument)) != NULL)
        {
                if (!victim->pcdata || !victim->pcdata->Account)
                {
                        send_to_char
                                ("&BN&zo &Raccount&z associated with that character.",
                                 ch);
                        return;
                }
                Account = victim->pcdata->Account;
        }
        else
        {
                if ((Account = get_account(argument)) == NULL)
                {
                        send_to_char("That Account is not around.", ch);
                        return;
                }
        }

        if (Account == NULL)
        {
                send_to_char("That player is not around.", ch);
                return;
        }

        ch_printf(ch, "Account Name: %s\n\r", Account->name);
        ch_printf(ch, "RP Points:    %d\n\r", Account->rppoints);
        ch_printf(ch, "RP Current:   %d\n\r", Account->rpcurrent);
        ch_printf(ch, "Quest Points: %d\n\r", Account->qpoints);
        send_to_char("----------------------------------------------\n\r",ch);

        for (count = 0; count < MaxCharacters; count++)
        {
                if (Account->character[count] == NULL)
                        continue;
                ch_printf(ch, "&w%-2d&B] &z%-66s&z\n\r", count,
                          capitalize(Account->character[count]));
        }

        if (count == 0)
        {
                send_to_char
                        ("&BT&zhat Account has no &Rcharacters&z linked to it.",
                         ch);
                return;
        }
}

void show_account_characters(DescriptorData * d)
{
        int       count;
        char      buf[MSL];

        if (!d->Account)
        {
                bug("show_account_character: no Account!", 0);
                return;
        }
        send_to_desc_color
                ("&z|-----------------------------------------------------------------------|\n\r",
                 d);
        for (count = 0; count < MaxCharacters; count++)
        {
                if (d->Account->character[count] == NULL)
                        break;
                snprintf(buf, MSL, "&z| &B[&w%-2d&B] &z%-65s&z|\n\r", count,
                         capitalize(d->Account->character[count]));
                send_to_desc_color(buf, d);
        }
        send_to_desc_color
                ("&z|                                                                       |\n\r",
                 d);
        /*
         * Bah to buffering first - Gavin 
         */
        snprintf(buf, MSL, "&z| &B[&wXX&B] &z%-65s&z|\n\r", "New");
        send_to_desc_color(buf, d);
        snprintf(buf, MSL, "&z| &B[&wXX&B] &z%-65s&z|\n\r", "Link");
        send_to_desc_color(buf, d);
        snprintf(buf, MSL, "&z| &B[&wXX&B] &z%-65s&z|\n\r", "Password");
        send_to_desc_color(buf, d);
        send_to_desc_color
                ("&z|-----------------------------------------------------------------------|\n\r",
                 d);
}

void save_account(ACCOUNT_DATA * Account)
{
        char      accountsave[MIL];
        FILE     *fp;

        if (!Account)
        {
                bug("Save_account: null Account!", 0);
                return;
        }

        snprintf(accountsave, MIL, "%s%c/%s.Account", ACCOUNT_DIR,
                 tolower(Account->name[0]), capitalize(Account->name));

        if ((fp = fopen(accountsave, "w")) == NULL)
        {
                bug("save_account: fopen", 0);
                perror(accountsave);
        }
        else
        {
                int       count;

                fprintf(fp, "#%s\n", "ACCOUNT");
                fprintf(fp, "Name      %s~\n", Account->name);
                fprintf(fp, "Password  %s~\n", Account->password);
                fprintf(fp, "Email     %s~\n", Account->email);
                fprintf(fp, "RPpoints  %ld\n", Account->rppoints);   // %ld for long
                fprintf(fp, "RPcurrent %ld\n", Account->rpcurrent);  // %ld for long
                fprintf(fp, "Qpoints   %ld\n", Account->qpoints);    // %ld for long
                for (count = 0; count < MaxCharacters; count++)
                {
                        if (Account->character[count] == NULL)
                                continue;

                        fprintf(fp, "Character %s~\n",
                                Account->character[count]);
                }
                fprintf(fp, "End\n\n");

                fwrite_alias(Account, fp);
                if (Account->comments)  /* comments */
                        fwrite_comments(Account, fp);   /* comments */
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }
        return;
}

ACCOUNT_DATA *get_account(const char *name)
{
        ACCOUNT_DATA *Account = NULL;

        for (Account = first_account; Account; Account = Account->next)
        {
                if (!Account->name)
                {
                        bug("Account with invalid Name", 0);
                        free_account(Account);
                }
                else if (!str_cmp(Account->name, name))
                {
#ifdef DEBUG
                        bug("Returned %s=%d", Account->name, Account->inuse);
#endif
                        return Account;
                }
        }
        return NULL;
}

ACCOUNT_DATA *load_account(const char *name)
{
        ACCOUNT_DATA *Account;
        char      accountsave[MIL];
        FILE     *fp;

        Account = get_account(name);
        if (Account)
        {
                Account->inuse++;
                return Account;
        }

        /* The capitalize function returns a static buffer, no need to modify input */
        snprintf(accountsave, MIL, "%s%c/%s.Account", ACCOUNT_DIR,
                 tolower(name[0]), capitalize(name));

        if ((fp = fopen(accountsave, "r")) != NULL)
        {
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
                                bug("Load_clan_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "ACCOUNT"))
                        {
                                /*
                                 * Create a create_account that sets defaults 
                                 */
                                Account = create_account();
                                fread_account(Account, fp);
                                continue;
                        }
                        else if (!str_cmp(word, "COMMENT"))
                        {
                                Account->comments = NULL;   /* comments */
                                fread_comment(Account, fp); /* Comments */
                        }
                        else if (!str_cmp(word, "ALIAS"))   /* Aliases */
                        {
                                fread_alias(Account, fp);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_account_file: bad section: %s.",
                                    word);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        else
        {
                return NULL;
        }
        if (Account != NULL)
        {
#ifdef DEBUG
                bug("Returned %s=%d", Account->name, Account->inuse);
#endif
                LINK(Account, first_account, last_account, next, prev);
        }
        else
        {
                bug("Account is null", 0);
        }
        return Account;
}

// =============================================================================
// Account File I/O Functions
// =============================================================================

void fread_account(ACCOUNT_DATA * Account, FILE * fp)
{
        char      buf[MSL];
        const char *word;
        bool      fMatch;
        int       count = 0;

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
                        if (!str_cmp(word, "Character"))
                        {
                                char     *name = fread_string(fp);

                                snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR,
                                         tolower(name[0]), capitalize(name));
                                if (access(buf, F_OK) != 0)
                                {
                                        /*
                                         * If their player file is NOT there, do not add it to the Account 
                                         */
                                        STRFREE(name);
                                        fMatch = TRUE;
                                        break;
                                }
                                Account->character[count] = name;
                                count++;
                                fMatch = TRUE;
                                break;
                        }
                        break;
                case 'E':
                        KEY("Email", Account->email, fread_string(fp));
                        if (!str_cmp(word, "End"))
                        {
                                if (Account->rpcurrent == -1)
                                        Account->rpcurrent =
                                                Account->rppoints;
                                return;
                        }
                        break;
                case 'N':
                        KEY("Name", Account->name, fread_string(fp));
                        break;
                case 'P':
                        KEY("Password", Account->password, fread_string(fp));
                        break;
                case 'Q':
                        KEY("Qpoints", Account->qpoints, fread_number(fp));
                        break;
                case 'R':
                        KEY("RPpoints", Account->rppoints, fread_number(fp));
                        KEY("RPcurrent", Account->rpcurrent,
                            fread_number(fp));
                        break;
                }
                if (!fMatch)
                {
                        snprintf(buf, MSL, "load_account: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }
}

bool add_to_account(ACCOUNT_DATA * acct, CharData * chdata)
{
        int       count;

        if (!acct || !chdata)
        {
                bug("add_to_account: null ch or Account!", 0);
                return FALSE;
        }
        for (count = 0; count < MaxCharacters; count++)
        {
                if (acct->character[count] == NULL)
                        continue;
                /*
                 * Prevent Duplicates anywhere in the list, but already in the list, so it was successful 
                 */
                if (!str_cmp(acct->character[count], chdata->name))
                        return TRUE;
        }
        for (count = 0; count < MaxCharacters; count++)
        {
                if (acct->character[count] == NULL)
                        break;
        }
        if (count >= MaxCharacters)
                return FALSE;

        acct->character[count] = STRALLOC(chdata->name);
        if (chdata->pcdata && chdata->pcdata->rp)
        {
                acct->rppoints += chdata->pcdata->rp;
                chdata->pcdata->rp = 0;
        }
/*      save_char_obj(chdata); */

        return TRUE;
}

bool del_from_account(ACCOUNT_DATA * Account, CharData * ch)
{
        int       count;

        if (!Account || !ch)
        {
                bug("del_from_account: null ch or Account!", 0);
                return FALSE;
        }

        for (count = 0; count < MaxCharacters; count++)
        {
                if (Account->character[count] == NULL)
                        continue;
                if (!str_cmp(Account->character[count], ch->name))
                {
                        STRFREE(Account->character[count]);
                        for (; count < MaxCharacters - 1; count++)
                                Account->character[count] =
                                        Account->character[count + 1];

                        break;
                }

        }

        return TRUE;
}

/*
 * Transfers one player from one Account to another
 * Currently both accounts and players need to be online
 * But soon to be fixed
 *
 * Actually, currently does nothing :D
 */
CMDF do_transaccount(CharData * ch, char *argument)
{
        CharData *victim = get_char_world(ch, argument);

        if (victim == NULL)
        {
                send_to_char("No such player online", ch);
                return;
        }

        return;
}

CMDF do_showaccounts(CharData * ch, char *argument)
{
        ACCOUNT_DATA *Account = NULL;
        CharData *victim = get_char_world(ch, argument);

        if (victim && !IS_NPC(victim))
        {
                Account = victim->pcdata->Account;
                ch_printf(ch, "Account Name: %s\n\r", Account->name);
                ch_printf(ch, "RP Points:    %d\n\r", Account->rppoints);
                ch_printf(ch, "RP Current:   %d\n\r", Account->rpcurrent);
                ch_printf(ch, "Quest Points: %d\n\r", Account->qpoints);
                return;
        }
        else
        {
                send_to_pager("All Chracters online:\n\r", ch);
                for (Account = first_account; Account;
                     Account = Account->next)
                {
                        pager_printf(ch, "%s: Count %d\n\r", Account->name,
                                     Account->inuse);
                }
        }
        send_to_char("&Bs&zhowalts <&wchar&B|&waccount&z>\n\r", ch);
        return;
}

CMDF do_switchchar(CharData * ch, char *argument)
{
        DescriptorData *d = ch->desc;
        int       count = 0;
        bool      loaded;

        if (d == NULL)
                return;
        if (d->Account == NULL)
                return;

        if (IS_NPC(ch))
                return;

        set_char_color(AT_PLAIN, ch);
        if (argument[0] == '\0')
        {
                set_pager_color(AT_PLAIN, ch);
                send_to_pager
                        ("&RSyntax: &Gswitchchar &C<&ccharacter name&C>&w\n\r\n\r",
                         ch);
                send_to_pager("Your choices are:\n\r", ch);
                for (count = 0; count < MaxCharacters; count++)
                {
                        if (d->Account->character[count] == NULL)
                                continue;
                        if (!str_cmp(ch->name, d->Account->character[count]))
                                continue;
                        pager_printf(ch, "\t&G%s\n\r",
                                     d->Account->character[count]);
                }
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                set_char_color(AT_RED, ch);
                send_to_char("No way! You are fighting.\n\r", ch);
                return;
        }

        if (auction->item != NULL
            && ((ch == auction->buyer) || (ch == auction->seller)))
        {
                send_to_char
                        ("Wait until you have bought/sold the item on auction.\n\r",
                         ch);
                return;
        }

        if (!IS_IMMORTAL(ch) && ch->in_room
            && !xIS_SET(ch->in_room->RoomFlags, ROOM_HOTEL)
            && !NOT_AUTHED(ch))
        {
                send_to_char("You may not quit here.\n\r", ch);
                send_to_char
                        ("You will have to find a safer resting place such as a hotel...\n\r",
                         ch);
                send_to_char("Maybe you could HAIL a speeder.\n\r", ch);
                return;
        }

        for (count = 0; count < MaxCharacters; count++)
        {
                if (d->Account->character[count] == NULL)
                        continue;
                if (!str_cmp(argument, d->Account->character[count]))
                        break;
        }
        if (d->Account->character[count] == NULL || count == MaxCharacters)
        {
                send_to_char("You do not have that character linked.\n\r",
                             ch);
                return;
        }
        if (!str_cmp(ch->name, d->Account->character[count]))
        {
                send_to_char("You are already on as them.\n\r", ch);
                return;
        }

        save_char_obj(ch);
        save_account(ch->pcdata->Account);
        save_home(ch);

        snprintf(log_buf, MSL, "%s has quit.", ch->name);
        log_string_plus(log_buf, LOG_COMM, get_trust(ch));
        if (!IS_SET(ch->act, PLR_WIZINVIS))
        {
                snprintf(log_buf, MSL, "%s has left %s", ch->name,
                         sysdata.mud_name);
                info_chan(log_buf);
        }
        ch->desc = NULL;
        d->character = NULL;
        d->Account = NULL;

        /*
         * So and so has left the game 
         */
        extract_char(ch, TRUE);
        /*
         * Check here to see if it exists in Account 
         */
        char *capitalized = strdup(capitalize(argument));
        loaded = load_char_obj(d, capitalized, FALSE, FALSE);
        free(capitalized); /* Free the temporary copy */
        if (loaded)
        {
                d->Account = d->character->pcdata->Account;
                ch = d->character;

				if (!check_reconnect(d, argument, TRUE)) {
					add_char(d->character);
				}

                if (ch->next)
                        ch->next->prev = ch;
                if (ch->prev)
                        ch->prev->next = ch;

                if (!IS_SET(ch->act, PLR_WIZINVIS))
                {
                        snprintf(log_buf, MSL, "%s has entered %s", ch->name,
                                 sysdata.mud_name);
                        info_chan(log_buf);
                }
                snprintf(log_buf, MSL, "%s@%s has connected.", ch->name,
                         d->host);
                if (ch->top_level < LevelDemi)
                {
                        log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
                }
                else
                        log_string_plus(log_buf, LOG_COMM, ch->top_level);
                if (ch->pcdata->area)
                        do_loadarea(ch, "");


				if (!IS_IMMORTAL(ch)
						&& ch->pcdata->release_date > current_time)
				{
					if (ch->in_room)
						char_from_room(ch);
					char_to_room(ch, get_room_index(6));
				}
				else if (ch->in_room && !IS_IMMORTAL(ch)
						&& ch->in_room->vnum != 6)
				{
					RoomIndexData * room = ch->in_room;
					if (ch->in_room)
						char_from_room(ch);
					char_to_room(ch, room);
				}
				else
				{
					if (ch->in_room)
						char_from_room(ch);
					ch->in_room = get_room_index(wherehome(ch));
					char_to_room(ch, ch->in_room);
				}
                return;
        }
        else
        {
                write_to_buffer(d,
                                "\n\rYour player file could not be loaded.\n\r",
                                0);
                extract_char(ch, TRUE);
                close_socket(d, FALSE);
        }

}
