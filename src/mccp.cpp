#ifdef MCCP
/*****************************************************************************************
 *                       DDDDD        A        RRRRRRR     K    K                        *
 *                       D    D      A A       R      R    K   K                         *
 *                       D     D    A   A      R      R    KK K                          *
 *                       D     D   A     A     RRRRRRR     K K                           *
 *                       D     D  AAAAAAAAA    R    R      K  K                          *
 *                       D    D  A         A   R     R     K   K                         *
 *                       DDDDD  A           A  R      R    K    K                        *
 *                                                                                       *
 *                                                                                       *
 *W      WW      W    A        RRRRRRR   RRRRRRR   IIIIIIII    OOOO   RRRRRRR     SSSSS  *
 * W    W  W    W    A A       R      R  R      R     II      O    O  R      R   S       *
 * W    W  W    W   A   A      R      R  R      R     II     O      O R      R   S       *
 * W    W  W    W  A     A     RRRRRRR   RRRRRRR      II     O      O RRRRRRR     SSSSS  *
 *  W  W    W  W  AAAAAAAAA    R    R    R    R       II     O      O R    R           S *
 *  W W     W W  A         A   R     R   R     R      II      O    O  R     R          S *
 *   W       W  A           A  R      R  R      R  IIIIIIII    OOOO   R      R    SSSSS  *
 *                                                                                       *
 *****************************************************************************************
 *                                                                                       *
 * Dark Warrior Code additions and changes from the Star Wars Reality code copyright (c) *
 * 2003 by Michael Ervin, Mark Gottselig, Gavin Mogan                                    *
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
 *                $Id: mccp.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
/*
 * Ported to SMAUG by Garil of DOTDII Mud
 * aka Jesse DeFer <dotd@dotd.com>  http://www.dotd.com
 *
 * revision 1: MCCP v1 support
 * revision 2: MCCP v2 support
 * revision 3: Correct MMCP v2 support
 * revision 4: clean up of write_to_descriptor() suggested by Noplex@CB
 *
 * See the web site below for more info.
 */

/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 */

/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#include <zlib.h>
#include <errno.h>

/* For memmove */
#include <string.h>

#include "mud.hpp"
#ifdef MXP
#include "mxp.hpp"
#endif

bool write_to_descriptor args((int desc, char *txt, int length));

void     *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
        opaque = NULL;
        return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
        opaque = NULL;
        DISPOSE(address);
}


bool process_compressed(DescriptorData * d)
{
        int       iStart = 0, nBlock, nWrite, len;

        if (!d->OutCompress)
                return TRUE;

        len = d->OutCompress->next_out - d->OutCompressBuf;

        if (len > 0)
        {
                for (iStart = 0; iStart < len; iStart += nWrite)
                {
                        nBlock = UMIN(len - iStart, 4096);
                        if ((nWrite =
                             write(d->descriptor,
                                   d->OutCompressBuf + iStart, nBlock)) < 0)
                        {
                                if (errno == EAGAIN || errno == ENOSR)
                                        break;

                                return FALSE;
                        }

                        if (!nWrite)
                                break;
                }

                if (iStart)
                {
                        if (iStart < len)
                                memmove(d->OutCompressBuf,
                                        d->OutCompressBuf + iStart,
                                        len - iStart);

                        d->OutCompress->next_out =
                                d->OutCompressBuf + len - iStart;
                }
        }

        return TRUE;
}

/* equiv to start_mxp */
unsigned char enable_compress[] =
        { IAC, SB, TeloptCompress, WILL, SE, '\0' };
unsigned char enable_compress2[] =
        { IAC, SB, TeloptCompress2, IAC, SE, '\0' };

bool compressStart(DescriptorData * d, unsigned char telopt)
{
        z_stream *s;

        if (d->OutCompress)
                return TRUE;

/*    bug("Starting compression for descriptor %d", d->descriptor); */

        CREATE(s, z_stream, 1);
        CREATE(d->OutCompressBuf, unsigned char, CompressBufSize);

        s->next_in = NULL;
        s->avail_in = 0;

        s->next_out = d->OutCompressBuf;
        s->avail_out = CompressBufSize;

        s->zalloc = Z_NULL;
        s->zfree = Z_NULL;
        s->opaque = Z_NULL;

        if (deflateInit(s, 9) != Z_OK)
        {
                DISPOSE(d->OutCompressBuf);
                DISPOSE(s);
                return FALSE;
        }

        if (telopt == TeloptCompress)
                write_to_descriptor(d->descriptor, (char *) enable_compress,
                                    0);
        else if (telopt == TeloptCompress2)
                write_to_descriptor(d->descriptor, (char *) enable_compress2,
                                    0);
        else
                bug("compressStart: bad TELOPT passed");

        d->Compressing = telopt;
        d->OutCompress = s;
        d->ShellCompressing = 0;

        return TRUE;
}

bool compressEnd(DescriptorData * d)
{
        unsigned char dummy[1];

        if (!d->OutCompress)
                return TRUE;

/*    bug("Stopping compression for descriptor %d", d->descriptor); */

        d->OutCompress->avail_in = 0;
        d->OutCompress->next_in = dummy;

        if (deflate(d->OutCompress, Z_FINISH) != Z_STREAM_END)
                return FALSE;

        if (!process_compressed(d))
                return FALSE;

        deflateEnd(d->OutCompress);
        DISPOSE(d->OutCompressBuf);
        DISPOSE(d->OutCompress);
        d->ShellCompressing = d->Compressing;
        d->Compressing = 0;

        return TRUE;
}

CMDF do_compress(CharData * ch, char *argument)
{
        if (IsNpc(ch) || !ch->desc)
        {
                send_to_char("What descriptor?!\n", ch);
                return;
        }

        if (!str_cmp(argument, "all") && IsImmortal(ch))
        {
                char      buf[MSL];
                CharData *vch;

                send_to_pager("Compression Info:\n", ch);
                for (vch = first_char; vch; vch = vch->next)
                {
                        if (vch->desc == NULL || !IsPlaying(vch->desc))
                                continue;
                        snprintf(buf, MSL, "%s: &B[&w%s&B]&w\n", vch->name,
                                 vch->desc->Compressing ? "ON " : "OFF");
                        send_to_pager(buf, ch);
                }
                return;
        }
        else if (!str_cmp(argument, "toggle"))
        {
                if (!ch->desc->OutCompress)
                {
                        do_compress(ch, "on");
                        return;
                }
                else
                {
                        do_compress(ch, "off");
                        return;
                }
        }
        else if (!str_cmp(argument, "on"))
        {
                send_to_char("Initiating compression.\n\r", ch);
                write_to_buffer(ch->desc, (const char *) will_compress2_str,
                                0);
                write_to_buffer(ch->desc, (const char *) will_compress_str,
                                0);
        }
        else if (!str_cmp(argument, "off"))
        {
                send_to_char("Terminating compression.\n\r", ch);
                compressEnd(ch->desc);
        }
        else if (!str_cmp(argument, "auto"))
        {
                send_to_char("Not Complete yet", ch);
        }
        else
        {
                set_char_color(AtGreen, ch);
                send_to_char("Compression Info:\n\r", ch);
                send_to_char("Compression: &B[&w", ch);
                if (IsMxp(ch))
                        send_to_char(MXPTAG("mxptoggle compress"), ch);
                if (ch->desc->Compressing)
                        send_to_char("ON ", ch);
                else
                {
                        send_to_char("OFF", ch);
                }
                if (IsMxp(ch))
                        send_to_char(MXPTAG("/mxptoggle"), ch);
                send_to_char("&B]&D\n\r", ch);
                if (ch->desc->OutCompress
                    && ch->desc->OutCompress->total_in)
                        ch_printf(ch,
                                  "Total size of input compressed:  &B[&w%d&B]&D\n\r",
                                  ch->desc->OutCompress->total_in);
                if (ch->desc->OutCompress
                    && ch->desc->OutCompress->total_out)
                        ch_printf(ch,
                                  "Total size of output compressed: &B[&w%d&B]&D\n\r",
                                  ch->desc->OutCompress->total_out);
                if (ch->desc->OutCompress && ch->desc->OutCompress->total_in
                    && ch->desc->OutCompress->total_out)
                        ch_printf(ch,
                                  "Current compression ratio:       &B[&w%.2f%&B]&D&D\n\r",
                                  100.0 -
                                  (float) ((float) ch->desc->OutCompress->
                                           total_out /
                                           (float) ch->desc->OutCompress->
                                           total_in * 100));
                return;
        }


}

CMDF do_mccpstats(CharData * ch, char *argument)
{
        DescriptorData *d;
        int       count = 0, total = 0;
        float     in = 0, out = 0;

        argument = NULL;

        for (d = first_descriptor; d; d = d->next)
        {
                total++;
                if (d->Compressing && d->OutCompress
                    && d->OutCompress->total_in
                    && d->OutCompress->total_out)
                {
                        count++;
                        in += d->OutCompress->total_in;
                        out += d->OutCompress->total_out;
                }
        }

        send_to_char("&zCompression Info for Dark Warriors:\n\r", ch);
        ch_printf(ch,
                  "Total size of data before compression: &C[&z%.0fKB&C]&z\n\r",
                  in / 1024);
        ch_printf(ch,
                  "Total size of data after compression: &C[&z%.0fKB&C]&z\n\r",
                  out / 1024);
        ch_printf(ch, "Total bandwidth saved: &C[&z%.0fKB&C]&z\n\r",
                  (in - out) / 1024);
        ch_printf(ch, "Compression Ratio: &C[&z%.2f%&C]&z\n\r",
                  (100.0 - (float) (out / in * 100)));
        ch_printf(ch, "MCCP Usage: &C[&z%d/%d players&C]&z\n\r", count,
                  total);
}
#endif
