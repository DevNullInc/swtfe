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
 *****************************************************************************************
 *                          SWR Hotboot module                                           *
 ****************************************************************************************/


#include <string>
#include <string_view>
#include <cstring>
#include <cctype>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include "mud.hpp"


DnsData *first_cache = nullptr;
DnsData *last_cache = nullptr;

void      save_dns(void);


void prune_dns()
{
        DnsData *cache = first_cache;
        while (cache) {
                DnsData *cache_next = cache->next;
                if (current_time - cache->time >= 1209600
                        || cache->ip == "Unknown??"
                        || cache->name == "Unknown??") {
                        // Remove from linked list
                        if (cache->prev) cache->prev->next = cache->next;
                        if (cache->next) cache->next->prev = cache->prev;
                        if (cache == first_cache) first_cache = cache->next;
                        if (cache == last_cache) last_cache = cache->prev;
                        delete cache;
                }
                cache = cache_next;
        }
        save_dns();
}

void check_dns(void)
{
        if (current_time >= new_boot_time_t)
                prune_dns();
        return;
}


void add_dns(const std::string& dhost, const std::string& address)
{
        auto* cache = new DnsData;
        cache->ip = dhost;
        cache->name = address;
        cache->time = current_time;
        cache->prev = last_cache;
        cache->next = nullptr;
        if (last_cache) last_cache->next = cache;
        last_cache = cache;
        if (!first_cache) first_cache = cache;
        save_dns();
}


std::string in_dns_cache(const std::string& ip)
{
        for (DnsData* cache = first_cache; cache; cache = cache->next) {
                if (ip == cache->ip) {
                        return cache->name;
                }
        }
        return "";
}

void fread_dns(DnsData * cache, FILE * fp)
{
        char     *word;
        bool      fMatch;

        for (;;)
        {
                word = feof(fp) ? (char *) "End" : fread_word(fp);
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
                                if (!cache->ip)
                                        cache->ip = STRALLOC("Unknown??");
                                if (!cache->name)
                                        cache->name = STRALLOC("Unknown??");
                                return;
                        }
                        break;

                case 'I':
                        KEY("IP", cache->ip, fread_string(fp));
                        break;

                case 'N':
                        KEY("Name", cache->name, fread_string(fp));
                        break;

                case 'T':
                        KEY("Time", cache->time, fread_number(fp));
                        break;
                }

                if (!fMatch)
                        bug("fread_dns: no match: %s", word);
        }
}

void load_dns(void)
{
        char      filename[256];
        DnsData *cache;
        FILE     *fp;

        first_cache = NULL;
        last_cache = NULL;

        snprintf(filename, 256, "%s", DnsFile);

        if ((fp = fopen(filename, "r")) != NULL)
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
                                bug("load_dns: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "CACHE"))
                        {
                                CREATE(cache, DnsData, 1);
                                fread_dns(cache, fp);
                                LINK(cache, first_cache, last_cache, next,
                                     prev);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("load_dns: bad section: %s.", word);
                                continue;
                        }
                }
                FCLOSE(fp);
                fp = NULL;
        }
        prune_dns();    /* Clean out entries beyond 14 days */
        return;
}

void save_dns(void)
{
        DnsData *cache;
        FILE     *fp = NULL;
        char      filename[256];

        snprintf(filename, 256, "%s", DnsFile);

        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_dns: fopen", 0);
                perror(filename);
        }
        else
        {
                for (cache = first_cache; cache; cache = cache->next)
                {
                        fprintf(fp, "#CACHE\n");
                        fprintf(fp, "IP		%s~\n", cache->ip);
                        fprintf(fp, "Name		%s~\n", cache->name);
                        fprintf(fp, "Time		%ld\n", cache->time);
                        fprintf(fp, "End\n\n");
                }
                fprintf(fp, "#END\n");
                FCLOSE(fp);
                fp = NULL;
        }
        return;
}

/* DNS Resolver code by Trax of Forever's End */
/*
 * Almost the same as read_from_buffer...
 */
bool read_from_dns(int Fd, char *buffer)
{
        static char InBuf[MaxStringLength * 2];
        int       iStart, i, j, k;

        /*
         * Check for overflow. 
         */
        iStart = strlen(InBuf);
        if (iStart >= (int) sizeof(InBuf) - 10)
        {
                bug("DNS input overflow!!!", 0);
                return FALSE;
        }

        /*
         * Snarf input. 
         */
        for (;;)
        {
                int       nRead;

                nRead = read(Fd, InBuf + iStart, sizeof(InBuf) - 10 - iStart);
                if (nRead > 0)
                {
                        iStart += nRead;
                        if (InBuf[iStart - 2] == '\n'
                            || InBuf[iStart - 2] == '\r')
                                break;
                }
                else if (nRead == 0)
                {
                        return FALSE;
                }
                else if (errno == EWOULDBLOCK)
                        break;
                else
                {
                        perror("Read_from_dns");
                        return FALSE;
                }
        }

        InBuf[iStart] = '\0';

        /*
         * Look for at least one new line.
         */
        for (i = 0; InBuf[i] != '\n' && InBuf[i] != '\r'; i++)
        {
                if (InBuf[i] == '\0')
                        return FALSE;
        }

        /*
         * Canonical input processing.
         */
        for (i = 0, k = 0; InBuf[i] != '\n' && InBuf[i] != '\r'; i++)
        {
                if (InBuf[i] == '\b' && k > 0)
                        --k;
                else if (isascii(InBuf[i]) && isprint(InBuf[i]))
                        buffer[k++] = InBuf[i];
        }

        /*
         * Finish off the line.
         */
        if (k == 0)
                buffer[k++] = ' ';
        buffer[k] = '\0';

        /*
         * Shift the input buffer.
         */
        while (InBuf[i] == '\n' || InBuf[i] == '\r')
                i++;
        for (j = 0; (InBuf[j] = InBuf[i + j]) != '\0'; j++)
                ;

        return TRUE;
}

/* DNS Resolver code by Trax of Forever's End */
/*
 * Process input that we got from resolve_dns.
 */
void process_dns(DescriptorData * d)
{
        char      address[MaxInputLength];
        int       status;

        address[0] = '\0';

        if (!read_from_dns(d->IFd, address) || address[0] == '\0')
                return;

        if (address[0] != '\0')
        {
                add_dns(d->host, address);  /* Add entry to DNS cache */
                STRFREE(d->host);
                d->host = STRALLOC(address);
        }

        /*
         * close descriptor and kill dns Process 
         */
        if (d->IFd != -1)
        {
                close(d->IFd);
                d->IFd = -1;
        }

        /*
         * we don't have to check here, 
         * cos the child is probably dead already. (but out of safety we do)
         * 
         * (later) I found this not to be true. The call to waitpid( ) is
         * necessary, because otherwise the child processes become zombie
         * and keep lingering around... The waitpid( ) removes them.
         */
        if (d->IPid != -1)
        {
                waitpid(d->IPid, &status, 0);
                d->IPid = -1;
        }
        return;
}

/* DNS Resolver hook. Code written by Trax of Forever's End */
void resolve_dns(DescriptorData * d, long ip)
{
        int       fds[2];
        pid_t     pid;

        /*
         * create pipe first 
         */
        if (pipe(fds) != 0)
        {
                perror("resolve_dns: pipe: ");
                return;
        }

        if (dup2(fds[1], StdoutFileno) != StdoutFileno)
        {
                perror("resolve_dns: dup2(stdout): ");
                return;
        }

        if ((pid = fork()) > 0)
        {
                /*
                 * parent Process 
                 */
                d->IFd = fds[0];
                d->IPid = pid;
                close(fds[1]);
        }
        else if (pid == 0)
        {
                /*
                 * child Process 
                 */
                char      str_ip[64];
                int       i;

                d->IFd = fds[0];
                d->IPid = pid;

                for (i = 2; i < 255; ++i)
                        close(i);

                snprintf(str_ip, 64, "%ld", ip);
                execl("../src/resolver", "AFKMud Resolver", str_ip, NULL);
                /*
                 * Still here --> hmm. An error. 
                 */
                bug("resolve_dns: Exec failed; Closing child.", 0);
                d->IFd = -1;
                d->IPid = -1;
                exit(0);
        }
        else
        {
                /*
                 * error 
                 */
                perror("resolve_dns: failed fork");
                close(fds[0]);
                close(fds[1]);
        }
}


void do_cache(CharData *ch, char *argument)
{
        // Only allow access for authorized users (immortal/admin)
        if (!IsImmortal(ch)) {
                send_to_char("You do not have permission to view DNS cache information.\n\r", ch);
                return;
        }
        int ip_count = 0;
        send_to_pager("&YCached DNS Information\n\r", ch);
        send_to_pager("IP               | Address\n\r", ch);
        send_to_pager("------------------------------------------------------------------------------\n\r", ch);
        for (DnsData* cache = first_cache; cache; cache = cache->next) {
                pager_printf(ch, "&W%16.16s  &Y%s\n\r", cache->ip.c_str(), cache->name.c_str());
                ip_count++;
        }
        pager_printf(ch, "\n\r&W%d IPs in the cache.\n\r", ip_count);
}


void free_dns(DnsData *cache)
{
        delete cache;
}
