/* IMC2 Freedom Client - Developed by Mud Domain.
 *
 * Copyright (C)2004 by Roger Libiez ( Samson )
 * Contributions by Johnathan Walker ( Xorith ), Copyright (C)2004
 * Additional contributions by Jesse Defer ( Garil ), Copyright (C)2004
 * Additional contributions by Rogel, Copyright (c) 2004
 * Additional contributions by StygianRenegade, Copyright (c) 2025
 * Comments and suggestions welcome: imc@imc2.org
 * License terms are available in the imc2freedom.license file.
 */

#ifndef __IMC2_H__
#define __IMC2_H__

/* The all important version ID string, which is hardcoded for now out of laziness.
 * This name was chosen to represent the ideals of not only the code, but of the
 * network which spawned it.
 */
#define ImcVersionString "IMC2 Renegade CL-2d " // Version 2d - September 2025
#define ImcVersion 2

/* Number of entries to keep in the channel histories */
#define MaxImchistory 20
#define MaxImctellhistory 20

/* Remcon: Ask and ye shall receive. */
#define ImcDir          "../imc/"

#define ImcChannelFile ImcDir "imc.channels"
#define ImcConfigFile  ImcDir "imc.config"
#define ImcBanFile     ImcDir "imc.ignores"
#define ImcUcacheFile  ImcDir "imc.ucache"
#define ImcColorFile   ImcDir "imc.color"
#define ImcHelpFile    ImcDir "imc.help"
#define ImcCmdFile     ImcDir "imc.commands"
#define ImcHotbootFile ImcDir "imc.hotboot"

/* Make sure you set the macros in the imccfg.h file properly or things get ugly from here. */
#include "imccfg.hpp"

/* Forward declarations to avoid circular dependencies */
struct CharData;
struct FBFILE;

#define ImcBuffSize 16384

/* Connection states stuff */
enum imc_constates
{
        ImcOffline, ImcAuth1, ImcAuth2, ImcOnline
};

typedef enum
{
        ImcpermNotset, ImcpermNone, ImcpermMort, ImcpermImm,
        ImcpermAdmin, ImcpermImp
} imc_permissions;

/* Flag macros */
#define ImcisSet(var, bit)         ((var) & (bit))
#define ImcsetBit(var, bit)        ((var) |= (bit))
#define ImcremoveBit(var, bit)     ((var) &= ~(bit))

/* Channel flags, only one so far, but you never know when more might be useful */
#define ImcchanLog      (1 <<  0)

/* Player flags */
#define ImcTell         (1 <<  0)
#define ImcDenytell     (1 <<  1)
#define ImcBeep         (1 <<  2)
#define ImcDenybeep     (1 <<  3)
#define ImcInvis        (1 <<  4)
#define ImcPrivacy      (1 <<  5)
#define ImcDenyfinger   (1 <<  6)
#define ImcAfk          (1 <<  7)
#define ImcColorflag    (1 <<  8)
#define ImcPermoverride (1 <<  9)
#define ImcNotify       (1 << 10)

#define IMCPERM(ch)           (ChImcdata((ch))->imcperm)
#define IMCFLAG(ch)           (ChImcdata((ch))->imcflag)
#define FirstImcignore(ch)   (ChImcdata((ch))->imcfirst_ignore)
#define LastImcignore(ch)    (ChImcdata((ch))->imclast_ignore)
#define ImcListen(ch)        (ChImcdata((ch))->imc_listen)
#define ImcDeny(ch)          (ChImcdata((ch))->imc_denied)
#define ImcRreply(ch)        (ChImcdata((ch))->rreply)
#define ImcRreplyName(ch)   (ChImcdata((ch))->rreply_name)
#define ImcEmail(ch)         (ChImcdata((ch))->email)
#define ImcHomepage(ch)      (ChImcdata((ch))->homepage)
#define ImcAim(ch)           (ChImcdata((ch))->aim)
#define ImcIcq(ch)           (ChImcdata((ch))->icq)
#define ImcYahoo(ch)         (ChImcdata((ch))->yahoo)
#define ImcMsn(ch)           (ChImcdata((ch))->msn)
#define ImcComment(ch)       (ChImcdata((ch))->comment)
#define IMCTELLHISTORY(ch,x)  (ChImcdata((ch))->imc_tellhistory[(x)])
#define IMCISINVIS(ch)        ( ImcisSet( IMCFLAG((ch)), ImcInvis ) || IsSet((ch)->act, PlrWizinvis) )
#define IMCAFK(ch)            ( ImcisSet( IMCFLAG((ch)), ImcAfk ) || IsSet(ch->act, PlrAfk) )

/* Should not need to edit anything below this point */

#define LGST 4096   /* Large String */
#define SMST 1024   /* Small String */


// Modern C++: use smart pointers and containers for memory management.
// IMCFCLOSE: use std::unique_ptr<FILE, fclose_deleter> for file safety.
// IMCCREATE/IMCRECREATE/IMCDISPOSE: use std::make_unique, std::vector, std::string.
// IMCSTRALLOC/IMCSTRFREE: std::string handles allocation and disposal automatically.


// Modern C++: use std::list or std::vector for double-linked list management.
// Remove manual macros; use container member functions for linking/unlinking.


/* No real functional difference in alot of this, but double linked lists DO seem to handle better,
 * and they look alot neater too. Yes, readability IS important! - Samson
 */
struct imc_channel;
struct imc_packet;
struct imc_packet_data;
struct imc_siteinfo;
struct imc_remoteinfo;
struct imc_ban_data;
struct imcchar_data;
struct imc_ignore;
struct imcucache_data;
struct imc_color_table;
struct imc_command_table;
struct imc_help_table;
struct imc_cmd_alias;
struct imc_packet_handler;

using ImcChannel = imc_channel;
using ImcPacket = imc_packet;
using ImcPdata = imc_packet_data;
using SITEINFO = imc_siteinfo;
using REMOTEINFO = imc_remoteinfo;
using ImcBan = imc_ban_data;
using ImcChardata = imcchar_data;
using ImcIgnore = imc_ignore;
using ImcucacheData = imcucache_data;
using ImcColor = imc_color_table;
using ImcCmdData = imc_command_table;
using ImcHelpData = imc_help_table;
using ImcAlias = imc_cmd_alias;
using ImcPhandler = imc_packet_handler;

using ImcFun = void(CharData*, const std::string&);
using PacketFun = void(ImcPacket*, const std::string&);

extern REMOTEINFO *first_rinfo;
extern REMOTEINFO *last_rinfo;
extern SITEINFO *this_imcmud;

/* Oh yeah, baby, that raunchy looking Merc structure just got the facelift of the century.
 * Thanks to Thoric and friends for the slick idea.
 */
struct imc_cmd_alias
{
        ImcAlias *next;
        ImcAlias *prev;
   std::string name;
};

struct imc_command_table
{
        ImcCmdData *next;
        ImcCmdData *prev;
        ImcAlias *first_alias;
        ImcAlias *last_alias;
        ImcFun  *function;
   std::string name;
        int       level;
        bool      connected;
};

struct imc_help_table
{
        ImcHelpData *next;
        ImcHelpData *prev;
        int       level;
   std::string name;
   std::string text;
};

struct imc_color_table
{
        ImcColor *next;
        ImcColor *prev;
   std::string name;
   std::string mudtag;
   std::string imctag;
};

struct imc_ignore
{
        ImcIgnore *next;
        ImcIgnore *prev;
   std::string name;
};

struct imcucache_data
{
        ImcucacheData *next;
        ImcucacheData *prev;
        time_t    time;
        int       gender;
   std::string name;
};

struct imcchar_data
{
        ImcIgnore *imcfirst_ignore;    /* List of ignored people */
        ImcIgnore *imclast_ignore;
   std::string rreply;
   std::string rreply_name;
   std::string imc_listen;
   std::string imc_denied;
   std::vector<std::string> imc_tellhistory = std::vector<std::string>(MaxImctellhistory);
   std::string email;
   std::string homepage;
   std::string aim;
   std::string yahoo;
   std::string msn;
   std::string comment;
   long imcflag = 0;
   int icq = 0;
   int imcperm = 0;
};

struct imc_channel
{
        ImcChannel *next;
        ImcChannel *prev;
        long      flags;
        short     level;
        bool      open;
        bool      refreshed;
   std::string name;
   std::string owner;
   std::string operators;
   std::string invited;
   std::string excluded;
   std::string local_name;
   std::string regformat;
   std::string emoteformat;
   std::string socformat;
   std::vector<std::string> history = std::vector<std::string>(MaxImchistory);
};

struct imc_packet_data
{
        ImcPdata *next;
        ImcPdata *prev;
   std::string field;
};

struct imc_packet
{
   ImcPdata *first_data;
   ImcPdata *last_data;
   std::string from;
   std::string to;
   std::string type;
   std::string route; // This is only used internally and not sent
};

/* The mud's connection data for the router */
struct imc_siteinfo
{
   std::string routername;
   std::string rhost;
   std::string network;
   std::string serverpw;
   std::string clientpw;
   std::string localname;
   std::string fullname;
   std::string ihost;
   std::string email;
   std::string www;
   std::string base;
   std::string details;
   int iport = 0;
   int minlevel = 0;
   int immlevel = 0;
   int adminlevel = 0;
   int implevel = 0;
   unsigned short rport = 0;
   bool md5 = false;
   bool md5pass = false;
   bool autoconnect = false;

        /*
         * Conection parameters - These don't save in the config file 
         */
        char      InBuf[ImcBuffSize]; /* input buffer */
        char      InComm[ImcBuffSize];
        char     *OutBuf;   /* output buffer */
        char     *versionid;    /* Transient version id for the imclist */
        unsigned long OutSize;
        int       OutTop;
        int       desc; /* descriptor */
        unsigned short state;   /* connection state */
};

struct imc_remoteinfo
{
        REMOTEINFO *next;
        REMOTEINFO *prev;
        bool      expired;
   std::string name;
   std::string version;
   std::string network;
   std::string path;
   std::string url;
};

/* A mudwide ban */
struct imc_ban_data
{
        ImcBan  *next;
        ImcBan  *prev;
   std::string name;
};

struct imc_packet_handler
{
        ImcPhandler *next;
        ImcPhandler *prev;
        PacketFun *func;
   std::string name;
};

bool imc_command_hook(CharData* ch, const std::string& command, const std::string& argument);
void imc_hotboot();
void imc_startup(bool Force, int desc, bool connected);
void imc_shutdown(bool reconnect);
void imc_initchar(CharData* ch);
bool imc_loadchar(CharData* ch, FILE* fp, const std::string& word);
void imc_savechar(CharData* ch, FILE* fp);
void imc_freechardata(CharData* ch);
void imc_loop();
ImcChannel* imc_findchannel(const std::string& name); // Externalized for comm.c spamguard checks
void imc_register_packet_handler(const std::string& name, PacketFun* func);
std::string imc_funcname(ImcFun* func);
ImcFun* imc_function(const std::string& func);

#if defined(_DISKIO_H_)
void imc_load_pfile(CharData* ch, const std::string& tag, int num, const std::string& line);
void imc_save_pfile(CharData* ch, FBFILE* fp);
#endif

#endif
