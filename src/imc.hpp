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
#define IMC_VERSION_STRING "IMC2 Renegade CL-2d " // Version 2d - September 2025
#define IMC_VERSION 2

/* Number of entries to keep in the channel histories */
#define MAX_IMCHISTORY 20
#define MAX_IMCTELLHISTORY 20

/* Remcon: Ask and ye shall receive. */
#define IMC_DIR          "../imc/"

#define IMC_CHANNEL_FILE IMC_DIR "imc.channels"
#define IMC_CONFIG_FILE  IMC_DIR "imc.config"
#define IMC_BAN_FILE     IMC_DIR "imc.ignores"
#define IMC_UCACHE_FILE  IMC_DIR "imc.ucache"
#define IMC_COLOR_FILE   IMC_DIR "imc.color"
#define IMC_HELP_FILE    IMC_DIR "imc.help"
#define IMC_CMD_FILE     IMC_DIR "imc.commands"
#define IMC_HOTBOOT_FILE IMC_DIR "imc.hotboot"

/* Make sure you set the macros in the imccfg.h file properly or things get ugly from here. */
#include "imccfg.hpp"

/* Forward declarations to avoid circular dependencies */
struct CharData;
struct FBFILE;

#define IMC_BUFF_SIZE 16384

/* Connection states stuff */
enum imc_constates
{
        IMC_OFFLINE, IMC_AUTH1, IMC_AUTH2, IMC_ONLINE
};

typedef enum
{
        IMCPERM_NOTSET, IMCPERM_NONE, IMCPERM_MORT, IMCPERM_IMM,
        IMCPERM_ADMIN, IMCPERM_IMP
} imc_permissions;

/* Flag macros */
#define IMCIS_SET(var, bit)         ((var) & (bit))
#define IMCSET_BIT(var, bit)        ((var) |= (bit))
#define IMCREMOVE_BIT(var, bit)     ((var) &= ~(bit))

/* Channel flags, only one so far, but you never know when more might be useful */
#define IMCCHAN_LOG      (1 <<  0)

/* Player flags */
#define IMC_TELL         (1 <<  0)
#define IMC_DENYTELL     (1 <<  1)
#define IMC_BEEP         (1 <<  2)
#define IMC_DENYBEEP     (1 <<  3)
#define IMC_INVIS        (1 <<  4)
#define IMC_PRIVACY      (1 <<  5)
#define IMC_DENYFINGER   (1 <<  6)
#define IMC_AFK          (1 <<  7)
#define IMC_COLORFLAG    (1 <<  8)
#define IMC_PERMOVERRIDE (1 <<  9)
#define IMC_NOTIFY       (1 << 10)

#define IMCPERM(ch)           (CH_IMCDATA((ch))->imcperm)
#define IMCFLAG(ch)           (CH_IMCDATA((ch))->imcflag)
#define FIRST_IMCIGNORE(ch)   (CH_IMCDATA((ch))->imcfirst_ignore)
#define LAST_IMCIGNORE(ch)    (CH_IMCDATA((ch))->imclast_ignore)
#define IMC_LISTEN(ch)        (CH_IMCDATA((ch))->imc_listen)
#define IMC_DENY(ch)          (CH_IMCDATA((ch))->imc_denied)
#define IMC_RREPLY(ch)        (CH_IMCDATA((ch))->rreply)
#define IMC_RREPLY_NAME(ch)   (CH_IMCDATA((ch))->rreply_name)
#define IMC_EMAIL(ch)         (CH_IMCDATA((ch))->email)
#define IMC_HOMEPAGE(ch)      (CH_IMCDATA((ch))->homepage)
#define IMC_AIM(ch)           (CH_IMCDATA((ch))->aim)
#define IMC_ICQ(ch)           (CH_IMCDATA((ch))->icq)
#define IMC_YAHOO(ch)         (CH_IMCDATA((ch))->yahoo)
#define IMC_MSN(ch)           (CH_IMCDATA((ch))->msn)
#define IMC_COMMENT(ch)       (CH_IMCDATA((ch))->comment)
#define IMCTELLHISTORY(ch,x)  (CH_IMCDATA((ch))->imc_tellhistory[(x)])
#define IMCISINVIS(ch)        ( IMCIS_SET( IMCFLAG((ch)), IMC_INVIS ) || IS_SET((ch)->act, PLR_WIZINVIS) )
#define IMCAFK(ch)            ( IMCIS_SET( IMCFLAG((ch)), IMC_AFK ) || IS_SET(ch->act, PLR_AFK) )

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

using IMC_CHANNEL = imc_channel;
using IMC_PACKET = imc_packet;
using IMC_PDATA = imc_packet_data;
using SITEINFO = imc_siteinfo;
using REMOTEINFO = imc_remoteinfo;
using IMC_BAN = imc_ban_data;
using IMC_CHARDATA = imcchar_data;
using IMC_IGNORE = imc_ignore;
using IMCUCACHE_DATA = imcucache_data;
using IMC_COLOR = imc_color_table;
using IMC_CMD_DATA = imc_command_table;
using IMC_HELP_DATA = imc_help_table;
using IMC_ALIAS = imc_cmd_alias;
using IMC_PHANDLER = imc_packet_handler;

using IMC_FUN = void(CharData*, const std::string&);
using PACKET_FUN = void(IMC_PACKET*, const std::string&);

extern REMOTEINFO *first_rinfo;
extern REMOTEINFO *last_rinfo;
extern SITEINFO *this_imcmud;

/* Oh yeah, baby, that raunchy looking Merc structure just got the facelift of the century.
 * Thanks to Thoric and friends for the slick idea.
 */
struct imc_cmd_alias
{
        IMC_ALIAS *next;
        IMC_ALIAS *prev;
   std::string name;
};

struct imc_command_table
{
        IMC_CMD_DATA *next;
        IMC_CMD_DATA *prev;
        IMC_ALIAS *first_alias;
        IMC_ALIAS *last_alias;
        IMC_FUN  *function;
   std::string name;
        int       level;
        bool      connected;
};

struct imc_help_table
{
        IMC_HELP_DATA *next;
        IMC_HELP_DATA *prev;
        int       level;
   std::string name;
   std::string text;
};

struct imc_color_table
{
        IMC_COLOR *next;
        IMC_COLOR *prev;
   std::string name;
   std::string mudtag;
   std::string imctag;
};

struct imc_ignore
{
        IMC_IGNORE *next;
        IMC_IGNORE *prev;
   std::string name;
};

struct imcucache_data
{
        IMCUCACHE_DATA *next;
        IMCUCACHE_DATA *prev;
        time_t    time;
        int       gender;
   std::string name;
};

struct imcchar_data
{
        IMC_IGNORE *imcfirst_ignore;    /* List of ignored people */
        IMC_IGNORE *imclast_ignore;
   std::string rreply;
   std::string rreply_name;
   std::string imc_listen;
   std::string imc_denied;
   std::vector<std::string> imc_tellhistory = std::vector<std::string>(MAX_IMCTELLHISTORY);
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
        IMC_CHANNEL *next;
        IMC_CHANNEL *prev;
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
   std::vector<std::string> history = std::vector<std::string>(MAX_IMCHISTORY);
};

struct imc_packet_data
{
        IMC_PDATA *next;
        IMC_PDATA *prev;
   std::string field;
};

struct imc_packet
{
   IMC_PDATA *first_data;
   IMC_PDATA *last_data;
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
        char      InBuf[IMC_BUFF_SIZE]; /* input buffer */
        char      InComm[IMC_BUFF_SIZE];
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
        IMC_BAN  *next;
        IMC_BAN  *prev;
   std::string name;
};

struct imc_packet_handler
{
        IMC_PHANDLER *next;
        IMC_PHANDLER *prev;
        PACKET_FUN *func;
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
IMC_CHANNEL* imc_findchannel(const std::string& name); // Externalized for comm.c spamguard checks
void imc_register_packet_handler(const std::string& name, PACKET_FUN* func);
std::string imc_funcname(IMC_FUN* func);
IMC_FUN* imc_function(const std::string& func);

#if defined(_DISKIO_H_)
void imc_load_pfile(CharData* ch, const std::string& tag, int num, const std::string& line);
void imc_save_pfile(CharData* ch, FBFILE* fp);
#endif

#endif
