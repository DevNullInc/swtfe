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
 *                                SWR Main Header                                        *
 ****************************************************************************************/


#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <math.h>
#include <string>
#include <string_view>
#include <cstring>
#include <stdint.h>
#include <format>
#include <memory>
#include <array>
#include <utility>
#include <cstddef>
#include <type_traits>
#include <functional>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "utils.hpp"

/* Make sure cpp_compat.h exists and is properly included */
#include "cpp_compat.hpp"

#ifdef MCCP
#include <zlib.h>
#endif

#include <list>
#include <map>
#include <chrono>
#include <bitset>
//#include <bits/stl_alloc.h>

typedef int ch_ret;
typedef int obj_ret;

/*
 * MCCP defines
 */
#ifdef MCCP
constexpr int CompressBufSize = 1024;
constexpr int TeloptCompress = 85;
constexpr int TeloptCompress2 = 86;
#endif // End of MCCP block

/*
 * GMCP (Generic MUD Communication Protocol)
 */
constexpr int TeloptGcmp = 201;

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */

constexpr bool False = false;
constexpr bool True = true;

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int sh_int;
typedef int bool;

#define unix
#else
typedef short int sh_int;

#if !defined(__cplusplus)
typedef unsigned char bool;
#endif
#endif

#define REVISION __STRING($Revision: 1356 $)

/*
* Time keeping
*/

// Forward declaration for HourMinSec
class HourMinSec;

extern std::chrono::system_clock::time_point pfile_time;
extern std::unique_ptr<HourMinSec> set_pfile_time;
extern std::unique_ptr<std::tm> new_pfile_time;
extern std::chrono::system_clock::time_point new_pfile_time_t;
extern std::int16_t num_pfiles;

void check_pfiles(std::chrono::system_clock::time_point reset);
void init_pfile_scan_time();
// End time keeping


/**
 * \section Structure types
 * Modernized forward declarations and type aliases for all major game structures.
 */
class BodyData;
class RaceData;
class AffectData;
class AreaData;
class AuctionData;
class WatchData;
class ExtractCharData;
struct CharData; // Forward declaration for macros and pointer usage
class HHFData;
class FightData;
class DescriptorData;
class ExitData;
class ExtraDescrData;
class HelpData;
class MobIndexData;
class CommentData;
class ObjData;
class ObjIndexData;
class PCData;
class ResetData;
class RoomIndexData;
class ShopData;
class RepairData;
class TimeInfoData;
class HourMinSec;
class WeatherData;
class PlanetData;
class SpaceData;
class ClanData;
class ShipData;
class MissileData;
class MobProgData;
class MPSleepData;
class MobProgActList;
class Timer;
class GodData;
class SystemData;
class SmaugAff;
class WhoData;
class SkillType;
class SocialType;
class CMDType;
class KilledData;
class WizEnt;
class SpecList;

class LogData;
class WebDescriptor;

#ifdef IMC
class IMCCharData;
#endif

using BodyList = std::list<BodyData*>;
using AreaList = std::list<AreaData*>;
typedef void DoFun(CharData * ch, char *argument);
typedef bool SpecFun(CharData * ch);
typedef ch_ret SpellFun(int sn, int level, CharData * ch, void *vo);

using ClanList = std::list<ClanData*>;
extern BodyList bodies;

/**
 * \section Function pointer types
 * Modernized using declarations for function pointer types, preserving C linkage.
 */
#ifdef __cplusplus
extern "C" {
#endif

using DoFun = void(CharData *ch, char *argument);
using SpecFun = bool(CharData *ch);
using SpellFun = ch_ret(int sn, int level, CharData *ch, void *vo);

#ifdef __cplusplus
}
#endif

// Banking
#define BankInterest	1.00028571428571
#define BankDir	"../banks/"
#define BAccountDir	"../banks/accounts/"
#define BAccountList	"accounts.lst"
typedef struct BankAccount BankAccount;

struct BankAccount
{
        BankAccount *next;
        BankAccount *prev;
        char     *code;
        char     *creator;
        char     *owner;
        char     *trustees;
        float interest;
        long flags;
        long amounthi;
        long amountlo;

};

#ifdef __cplusplus
#define CMDFV void // renamed from CMDF to CMDFV to prevent conflict with typedef
#define SPELLF ch_ret
#else
#define CMDF void
#define SPELLF ch_ret
#endif
#define SPECF bool

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#else
#define args( list )			list
#endif

/* Duuude */
#include "commands.hpp"


#define DurConv	23.333333333333333333333333
#define HiddenTilde	'*'


// Modernized bitvector macros for x64 (uint64_t). TODO: Refactor to std::bitset or enum class for >32 bits.
constexpr uint64_t BV00 = (1ULL <<  0);
constexpr uint64_t BV01 = (1ULL <<  1);
constexpr uint64_t BV02 = (1ULL <<  2);
constexpr uint64_t BV03 = (1ULL <<  3);
constexpr uint64_t BV04 = (1ULL <<  4);
constexpr uint64_t BV05 = (1ULL <<  5);
constexpr uint64_t BV06 = (1ULL <<  6);
constexpr uint64_t BV07 = (1ULL <<  7);
constexpr uint64_t BV08 = (1ULL <<  8);
constexpr uint64_t BV09 = (1ULL <<  9);
constexpr uint64_t BV10 = (1ULL << 10);
constexpr uint64_t BV11 = (1ULL << 11);
constexpr uint64_t BV12 = (1ULL << 12);
constexpr uint64_t BV13 = (1ULL << 13);
constexpr uint64_t BV14 = (1ULL << 14);
constexpr uint64_t BV15 = (1ULL << 15);
constexpr uint64_t BV16 = (1ULL << 16);
constexpr uint64_t BV17 = (1ULL << 17);
constexpr uint64_t BV18 = (1ULL << 18);
constexpr uint64_t BV19 = (1ULL << 19);
constexpr uint64_t BV20 = (1ULL << 20);
constexpr uint64_t BV21 = (1ULL << 21);
constexpr uint64_t BV22 = (1ULL << 22);
constexpr uint64_t BV23 = (1ULL << 23);
constexpr uint64_t BV24 = (1ULL << 24);
constexpr uint64_t BV25 = (1ULL << 25);
constexpr uint64_t BV26 = (1ULL << 26);
constexpr uint64_t BV27 = (1ULL << 27);
constexpr uint64_t BV28 = (1ULL << 28);
constexpr uint64_t BV29 = (1ULL << 29);
constexpr uint64_t BV30 = (1ULL << 30);
constexpr uint64_t BV31 = (1ULL << 31);
// NOTE: 32 USED! Audit and refactor for >32 bits as needed for x64.

/*
 * String and memory management parameters.
 */
#define MaxKeyHash		 2048
#define MaxStringLength	 4096   /* buf */
#define MSL                  MaxStringLength  /* Centralized MSL definition */
#define MaxInputLength	 1024   /* arg */
#define MaxInbufSize		 1024
#define MaxFileLength          256   /* file */
#define MFL                  MaxFileLength /* Centralized MFL definition */

#define HASHSTR /* use string hashing */
#define xIsSet(var, bit)       ((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM)) // Possible conflict?
#define xSET_BIT(var, bit)      ((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)     (ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)   ((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))


constexpr int MaxLayers = 8;  // maximum clothing layers
constexpr int MaxNest = 100;  // maximum container nesting
constexpr int MaxKillTrack = 20;  // track mob vnums killed

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */

constexpr int MaxExpWorth = 500000;
constexpr int MinExpWorth = 25;
constexpr int MaxRExits = 20;   // Maximum exits allowed in 1 room
constexpr int MaxSkill = 400;
constexpr int MaxAbility = 11;
constexpr int MaxRace = 29;
constexpr int MaxNpcRace = 91;
constexpr int MaxVnums = 2000000000;
constexpr int MaxLevel = 155;
constexpr int MaxClan = 50;
constexpr int MaxPlanet = 100;
constexpr int MaxShip = 1000;
constexpr int MaxBounty = 255;
constexpr int MaxGov = 255;
constexpr int MaxIgnore = 1000;
constexpr int MaxHerb = 20;

constexpr int LevelHero = MaxLevel - 5;
constexpr int LevelImmortal = MaxLevel - 4;
constexpr int LevelSupreme = MaxLevel;
constexpr int LevelInfinite = MaxLevel - 1;
constexpr int LevelEternal = MaxLevel - 1;
constexpr int LevelImplementor = MaxLevel - 1;
constexpr int LevelSubImplem = MaxLevel - 1;
constexpr int LevelAscendant = MaxLevel - 2;
constexpr int LevelGreater = MaxLevel - 2;
constexpr int LevelGod = MaxLevel - 2;
constexpr int LevelLesser = MaxLevel - 3;
constexpr int LevelTrueImm = MaxLevel - 3;
constexpr int LevelDemi = MaxLevel - 3;
constexpr int LevelSavior = MaxLevel - 3;
constexpr int LevelCreator = MaxLevel - 3;
constexpr int LevelNeophyte = MaxLevel - 4;
constexpr int LevelAvatar = MaxLevel - 5;
constexpr int LevelBuilder = LevelCreator;

#include "shell.hpp"
#include "autobuild.hpp"
#include "color.hpp"
#include "hotboot.hpp"
#include "implants.hpp"

#ifdef CALLOC
#undef CALLOC
#endif
#define CALLOC calloc

#ifdef MALLOC
#undef MALLOC
#endif
#define MALLOC malloc

#ifdef REALLOC
#undef REALLOC
#endif
#define REALLOC realloc

#ifdef FREE
#undef FREE
#endif
#define FREE free


constexpr int LevelLog = LevelLesser;
constexpr int LevelHiGod = LevelGod;

constexpr int MobVnumVendor = 6;   // vnum of vendor
constexpr int LevelBuyVendor = 50; // minimum level to buy a vendor
constexpr int CostBuyVendor = 75000; // cost of a vendor
constexpr int ObjVnumDeed = 5;     // vnum of deed
constexpr int RoomVnumVenstor = 5; // where messed up vendors go!
constexpr const char* VendorDir = "../vendor/";

constexpr int PulsePerSecond = 2;
constexpr int PulseMinute = 60 * PulsePerSecond;
constexpr int PulseViolence = 3 * PulsePerSecond;
constexpr int PulseMobile = 4 * PulsePerSecond;
constexpr int PulseTick = 70 * PulsePerSecond;
constexpr int PulseArea = 60 * PulsePerSecond;
constexpr int PulseAuction = 10 * PulsePerSecond;
constexpr int PulseSpace = 10 * PulsePerSecond;
constexpr int PulseTaxes = 60 * PulseMinute;
constexpr int PulseAuth = 3 * PulseMinute;
constexpr int PulseCrashOver = 45 * PulsePerSecond;

/*
 * Command logging types.
 */
typedef enum
{
        LogNormal, LogAlways, LogNever, LogBuild, LogHigh, LogComm,
        LogAll
} log_types;

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
        rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
        rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
        rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
        rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE, rCHAR_AND_OBJ_EXTRACTED = 128,
        rERROR = 255, rSTOP
} ret_types;


// Echo types for echo_to_all
constexpr int EchoTarAll = 0;
constexpr int EchoTarPc = 1;
constexpr int EchoTarImm = 2;

// Types for new do_who
constexpr int WtMortal = 0;
constexpr int WtAvatar = 1;
constexpr int WtImm = 2;
constexpr int WtNewbie = 3;

// Defines for extended bitvectors
#ifndef IntBits
constexpr int IntBits = 32;
#endif
constexpr int Xbi = 4;   // integers in an extended bitvector
constexpr int Rsv = 5;   // right-shift value  ( sqrt(Xbm+1) )
constexpr int Xbm = 31;  // extended bitmask   ( IntBits - 1 )
constexpr int MaxBits = Xbi * IntBits;
/*
 * Structure for extended bitvectors -- StygianRenegade
 */
using ExtBV = std::bitset<128>;  // Clean, type-safe, STL bitset for extended bitvectors

#include "olc.hpp"
#include "dns.hpp"
#ifdef IMC
#include "imc.hpp"
#endif

/*
 * do_who output structure -- Narn
 */
struct WhoData
{
        WhoData *prev;
        WhoData *next;
        char     *text;
        int type;
};

/*
 * Player watch data structure  --Gorog
 */
struct WatchData
{
        WatchData *next;
        WatchData *prev;
        sh_int imm_level;
        char     *imm_name; /* imm doing the watching */
        char     *target_name;  /* player or command being watched   */
        char     *player_site;  /* site being watched     */
#ifdef ACCOUNT
        char     *player_account;   /* Account being watched */
#endif
};

/*
 * Time and weather stuff.
 */
typedef enum
{
        SunDark, SunRise, SunLight, SunSet
} SunPositions;

typedef enum
{
        SkyCloudless, SkyCloudy, SkyRaining, SkyLightning
} SkyConditions;

struct TimeInfoData
{
        int hour;
        int day;
        int month;
        int year;
};

struct HourMinSec {
public:
    HourMinSec() = default;
    HourMinSec(int h, int m, int s) : hour_(h), min_(m), sec_(s) {}

    int get_hour() const { return hour_; }
    void set_hour(int h) { hour_ = h; }

    int get_min() const { return min_; }
    void set_min(int m) { min_ = m; }

    int get_sec() const { return sec_; }
    void set_sec(int s) { sec_ = s; }

private:
    int hour_{0};
    int min_{0};
    int sec_{0};
};

struct WeatherData
{
        int mmhg;
        int change;
        int sky;
        int sunlight;
};




/*
 * Structure used to build wizlist
 */
struct WizEnt
{
        WizEnt   *next;
        WizEnt   *last;
        char     *name;
        sh_int level;
        int flags;
};

/*
 * Connected state for a channel.
 */
typedef enum
{
        ConGetName, ConGetOldPassword,
        ConConfirmNewName, ConGetNewPassword, ConConfirmNewPassword,
        ConAccountGetEmail,
        ConGetNewSex, ConReadMotd, ConGetNewRace,
        ConGetEmulation, ConGetWantRipAnsi,
        ConTitle, ConPressEnter, ConWait1,
        ConWait2, ConWait3, ConAccepted,
        ConGetPkill, ConReadImotd, ConGetNewEmail,
        ConGetMsp, ConGetNewClass, ConRollStats,
        ConShowStatOptions, ConEditStats, ConStatsOk,
        ConCopyoverRecover, ConForked, ConIaForked,
        ConWizinvis, ConEditStatNum, ConMenu
#ifdef ACCOUNT
                , ConNewAccount, ConGetAccount,
        ConGetOldAccountPassword,
        ConGetAlt, ConGetNewAccountPassword,
        ConConfirmNewAccountPassword,
        ConLinkAlt, ConGetLinkPassword, ConConfirmNewAccountName,
        ConGetAccOldPass, ConGetAccNewPass, ConGetAccConfirmPass
#endif
        , ConPlaying, ConEditing
} connection_types;




/*
 * Character substates
 */
typedef enum
{
        SubNone, SubPause, SubPersonalDesc, SubObjShort, SubObjLong,
        SubObjExtra, SubMobLong, SubMobDesc, SubBanDesc,
        SubRoomDesc, SubRoomExtra,
        SubRoomExitDesc, SubWritingNote, SubMprogEdit, SubHelpEdit,
        SubPersonalBio, SubRepeatCmd, SubRestricted,
        SubDeityDesc, SubWritingEmail, SubAliasMsg, SubAlias,
#ifdef RESTORE
        SubRestoreVMsg, SubRestorerMsg, SubRestoreCMsg,
#endif
        /*
         * timer types ONLY below this point 
         */
        SubTimerDoAbort = 128, SubTimerCantAbort
} CharSubstates;

/*
 * Descriptor (channel) structure.
 */
struct DescriptorData
{
        DescriptorData *next;
        DescriptorData *prev;
        DescriptorData *snoop_by;
        CharData *character;
        CharData *original;
        char     *host;
        char     *hostip;
        int descriptor;
        sh_int connected;
        sh_int idle;
        sh_int lines;
        sh_int scrlen;
        bool fcommand;
        char InBuf[MaxInbufSize];
        char InComm[MaxInputLength];
        char InLast[MaxInputLength];
        int repeat;
        char     *OutBuf;
        unsigned long OutSize;
        int OutTop;
        char     *PageBuf;
        long PageSize;
        int PageTop;
        char     *PagePoint;
        char PageCmd;
        char PageColor;
        int AuthState;
        int ATimes;
        int NewState;
        unsigned char PrevColor;
        pid_t Process;  /* Samson 4-16-98 - For new command shell code */
        sh_int Speed;   /* descriptor Speed settings */
        char     *Client;
#ifdef MCCP
        unsigned char Compressing;
        z_stream *OutCompress;
        unsigned char *OutCompressBuf;
        unsigned char ShellCompressing;
#endif
        bool MxpDetected;  /* player using MXP flag */
        bool MspDetected;  /* player using MSP flag */
#ifdef ACCOUNT
        struct AccountData *Account;
#endif
        int IFd;
        pid_t IPid;
};

struct WebDescriptor
{
        int Fd;
        char Request[2048];
        struct sockaddr_in *TheirAddr;
        int SinSize;
        WebDescriptor *next;
        WebDescriptor *prev;
        bool Valid;
};

/*
 * Attribute Bonus structures.
 */
struct StrAppType
{
        sh_int tohit;
        sh_int todam;
        sh_int carry;
        sh_int wield;
};

struct IntAppType
{
        sh_int learn;
};

struct wis_app_type
{
        sh_int practice;
};

struct dex_app_type
{
        sh_int defensive;
};

struct con_app_type
{
        sh_int hitp;
        sh_int shock;
};

struct cha_app_type
{
        sh_int charm;
};

struct lck_app_type
{
        sh_int Luck;
};

struct frc_app_type
{
        sh_int Force;
};

/* ability classes */

constexpr int AbilityNone	=	-1;
constexpr int CombatAbility	=	0;
constexpr int PilotingAbility	=	1;
constexpr int EngineeringAbility	=	2;
constexpr int HuntingAbility	=	3;
constexpr int SmugglingAbility	=	4;
constexpr int DiplomacyAbility	=	5;
constexpr int LeadershipAbility	=	6;
constexpr int ForceAbility	=	7;
constexpr int OccupationAbility	=	8;
constexpr int PiracyAbility	=	9;
constexpr int MedicAbility	=	10;


/* the races */
constexpr int RaceHuman	=	0;
constexpr int RaceWookiee	=	1;
constexpr int RaceTwiLek	=	2;
constexpr int RaceRodian	=	3;
constexpr int RaceHutt	=	4;
constexpr int RaceMonCalamari	=	5;
constexpr int RaceNoghri	=	6;
constexpr int RaceGamorrean	=	7;
constexpr int RaceJawa	=	8;
constexpr int RaceAdarian	=	9;
constexpr int RaceEwok	=	10;
constexpr int RaceVerpine	=	11;
constexpr int RaceDefel	=	12;
constexpr int RaceTrandoshan	=	13;
constexpr int RaceShistavanan	=	14;
constexpr int RaceDuinuogwuin	=	16;
constexpr int RaceDroid           	=	15;
constexpr int RaceChiss           	=	17;


/*
 * Languages -- Altrag
 */
constexpr int LangBasic        = 0;  /* Human base language */
constexpr int LangWookiee      = 1;
constexpr int LangTwiLek      = 2;
constexpr int LangRodian       = 3;
constexpr int LangHutt         = 4;
constexpr int LangMonCalamari = 5;
constexpr int LangNoghri       = 6;
constexpr int LangEwok         = 7;
constexpr int LangIthorian     = 8;
constexpr int LangDevaronian   = 9;
constexpr int LangGamorrean    = 10;
constexpr int LangJawa         = 11;
constexpr int LangClan	      = 12;
constexpr int LangAdarian	  = 13;
constexpr int LangVerpine	  = 14;
constexpr int LangDefel        = 15;
constexpr int LangTrandoshan   = 16;
constexpr int LangShistavanan  = 17;
constexpr int LangBinary       = 18;
constexpr int LangDuinuogwuin  = 19;
constexpr int LangCsillian     = 20;
constexpr int LangKelDor      = 21;
constexpr int LangBothan       = 22;
constexpr int LangBarabel      = 23;
constexpr int LangDurosian     = 24;
constexpr int LangGotal        = 25;
constexpr int LangTalz         = 26;
constexpr int LangHoDin       = 27;
constexpr int LangFalleen      = 28;
constexpr int LangGivin        = 29;
constexpr int LangUnknown    =    0;   /* Anything that doesnt fit a category */
#define ValidLangs    ( LangBasic | LangWookiee | LangTwiLek | LangRodian  \
		       | LangHutt | LangMonCalamari | LangNoghri | LangGamorrean \
		       | LangJawa | LangAdarian | LangEwok | LangVerpine | LangDefel \
		       | LangTrandoshan | LangShistavanan | LangBinary | LangDuinuogwuin \
			   | LangCsillian| LangKelDor | LangBothan | LangBarabel | LangIthorian \
			   | LangDevaronian | LangDurosian | LangGotal | LangTalz | LangHoDin	\
			   | LangFalleen | LangGivin)
/*  26 Languages */

/*
 * TO types for act.
 */

/* if > IcTo && < OocTo */
#define IsIcAct(act) (act > IcTo && act < OocTo)
#define IsOocAct(act) (act > OocTo)
#define IsContraband(cargo) ((cargo) > ContrabandNone && (cargo) < ContrabandMax)
typedef enum
{
        IcTo, ToRoom, ToNotvict, ToVict, ToChar, ToMud,
        OocTo, ToRoomOoc, ToNotvictOoc, ToVictOoc, ToCharOoc,
        ToMudOoc
} ActToTypes;

#define InitWeaponCondition    12
#define MaxItemImpact		 30

/*
 * Help table types.
 */
struct help_data
{
        HelpData *next;
        HelpData *prev;
        sh_int level;
        char     *keyword;
        char     *text;
        char     *author;
        char     *date;
};



/*
 * Shop types.
 */
#define MaxTrade	 5

struct shop_data
{
        ShopData *next;    /* Next shop in list        */
        ShopData *prev;    /* Previous shop in list    */
        int keeper; /* Vnum of shop keeper mob  */
        sh_int buy_type[MaxTrade]; /* Item types shop will buy */
        sh_int profit_buy;  /* Cost multiplier for buying   */
        sh_int profit_sell; /* Cost multiplier for selling  */
        sh_int open_hour;   /* First opening hour       */
        sh_int close_hour;  /* First closing hour       */
};

#define MaxFix		3
#define ShopFix	1
#define ShopRecharge	2

struct repairshop_data
{
        RepairData *next;  /* Next shop in list        */
        RepairData *prev;  /* Previous shop in list    */
        int keeper; /* Vnum of shop keeper mob  */
        sh_int fix_type[MaxFix];   /* Item types shop will fix */
        sh_int profit_fix;  /* Cost multiplier for fixing   */
        sh_int shop_type;   /* Repair shop type     */
        sh_int open_hour;   /* First opening hour       */
        sh_int close_hour;  /* First closing hour       */
};


/* Mob program structures */
/* Mob program structures and defines */
/* Moved these defines here from mud_prog.cpp as I need them -TSR */
#define MaxIfs 20  /* should always be generous */
#define InIf 0
#define InElse 1
#define DoIf 2
#define DoElse 3

#define MaxProgNest 20

struct act_prog_data
{
        struct act_prog_data *next;
        void     *vo;
};

struct MProgActList
{
        MProgActList *next;
        char     *buf;
        CharData *ch;
        ObjData *obj;
        void     *vo;
};

struct MProgData
{
        MProgData *next;
        int type;
        bool triggered;
        int resetdelay;
        char     *arglist;
        char     *comlist;
};

/* Used to store sleeping mud progs. -rkb */
typedef enum
{ MpMob, MpRoom, MpObj } mp_types;
struct mpsleep_data
{
        MPSleepData *next;
        MPSleepData *prev;

        int timer;  /* Pulses to sleep */
        mp_types type;  /* Mob, Room or Obj prog */
        RoomIndexData *room;  /* Room when type is MpRoom */

        /*
         * mprog_driver state variables 
         */
        int ignorelevel;
        int iflevel;
        bool ifstate[MaxIfs][DoElse + 1];

        /*
         * mprog_driver arguments 
         */
        char     *com_list;
        CharData *mob;
        CharData *actor;
        ObjData *obj;
        void     *vo;
        bool single_step;
};


extern bool MOBtrigger;

/* race dedicated stuff*/
struct race_type
{
        char race_name[16];
        int affected;
        sh_int str_plus;
        sh_int dex_plus;
        sh_int wis_plus;
        sh_int int_plus;
        sh_int con_plus;
        sh_int cha_plus;
        sh_int lck_plus;
        sh_int frc_plus;
        sh_int hit;
        sh_int endurance;
        sh_int resist;
        sh_int suscept;
        int class_restriction;
        int language;
};

struct log_data
{
        char     *name;
        char     *message;
        time_t time;
        LanguageData *language;
};

typedef enum
{
        ClanPlain, ClanCrime, ClanGuild, ClanSubclan
} clan_types;

#define HasClanPerm(ch, clan, permission) \
		( (ch) && (ch)->PCData && (clan) && (ch)->PCData->bestowments && \
		 ( !str_cmp((ch)->name, (clan)->leader) || \
		   !str_cmp((ch)->name, (clan)->number1) || \
		   !str_cmp((ch)->name, (clan)->number2) ||  \
		   is_name((permission), (ch)->PCData->bestowments)  \
		 ) \
		)

enum class ShipType {
    Civilian,
    Republic,
    Imperial,
    MobShip,
    PlayerShip,
    ClanMobShip
};
// Modernized enums for C++23: use enum class for type safety, scoped values, and underlying types.
// Also, use PascalCase for enum names and values for clarity and consistency.

enum class ShipState : uint8_t {
        Docked,
        Ready,
        Busy,
        Busy2,
        Busy3,
        Refuel,
        Launch,
        Launch2,
        Land,
        Land2,
        Hyperspace,
        Disabled,
        Flying
};

enum class MissileState : uint8_t {
        Ready,
        Fired,
        Reload,
        Reload2,
        Damaged
};

enum class ShipClass : uint8_t {
        Fighter,
        Midsize,
        Capital,
        Platform
};

enum class MissileType : uint8_t {
        ConcussionMissile,
        ProtonTorpedo,
        HeavyRocket,
        HeavyBomb
};

enum class GroupType : uint8_t {
        Clan,
        Council,
        Guild
};

#define LaserDamaged    -1
#define LaserReady       0

struct DockData;
struct space_data
{
        space_data();
        ~space_data();
        SpaceData *next;
        SpaceData *prev;
        ShipData *first_ship;
        ShipData *last_ship;
        MissileData *first_missile;
        MissileData *last_missile;
        PlanetData *first_planet;
        PlanetData *last_planet;
        DockData *first_dock;
        DockData *last_dock;
        BodyList bodies;
        char     *filename;
        char     *name;
        int xpos;
        int ypos;

};

/* cargo types */
typedef enum
{
        CargoNone, CargoOre, CargoProduce, CargoMeat, CargoMetal,
        CargoMinerals, CargoComponents, CargoFuelCells, CargoTabanna,
        CargoCultured, CargoProcessed, CargoDuracrete, CargoDurasteel,
        CargoElectronics, CargoSerind, CargoTetrali, CargoComputers,
        CargoMonothelene, CargoAtomicCells, CargoBacta,
        CargoTransparisteel,
        CargoDroidParts, CargoArmour, CargoDelicacies, CargoAlazhi,
        CargoMax,
        ContrabandNone, ContrabandBlasters, ContrabandSpice,
        ContrabandSlaves,
        ContrabandExplosives, ContrabandMax
} cargo_types;

/* disease types */
constexpr int IllnessNone        =    0;
constexpr int IllnessSniffles        =    1;
constexpr int IllnessGas             =    2;
constexpr int IllnessCough           =    3;
constexpr int IllnessCold            =    4;
constexpr int IllnessFlu             =    5;
constexpr int IllnessStrep           =    6;
constexpr int IllnessPnumonia        =    7;
constexpr int IllnessInfecteye       =    8;
constexpr int IllnessInfectear       =    9;
constexpr int IllnessInfectthroat    =   10;
constexpr int IllnessBachularia      =   11;
constexpr int IllnessAngina          =   12;
constexpr int IllnessDepression      =   13;
constexpr int IllnessCancer          =   14;
constexpr int IllnessBubonicPlague  =   15;
constexpr int IllnessAids            =   16;
constexpr int IllnessMax             =   17;


struct InstallationData;
struct PlanetData
{
        PlanetData *next;
        PlanetData *prev;
        PlanetData *next_in_system;
        PlanetData *prev_in_system;
        InstallationData *first_install;
        InstallationData *last_install;
        SpaceData *starsystem;
        AreaData *first_area;
        AreaData *last_area;
        BodyData *body;
        char     *bodyname;
        char     *name;
        char     *filename;
        long base_value;
        ClanData *governed_by;
        int population;
        int turbolasers;
        int ioncannons;
        int shields;
        int budgetpolice;
        int budgetenter;
        int budgetmaintenance;
        int budgeteducation;
        int budgetantiesp;
        int flags;
        int pop_support;
        int cargoimport[ContrabandMax];
        int cargoexport[ContrabandMax];
        int resource[ContrabandMax];
        int consumes[ContrabandMax];
        int produces[ContrabandMax];
        int defbattalions;
        int attbattalions;
        ClanData *attgovern;
        int PlanetType;
        int jail;
};


constexpr int PlanetNocapture  = 0;
constexpr int PlanetShield     = 1;

/* NEVER USE MAGIC NUMBERS */
/* Pretty sure if we are doing 0-13, this can be 13 */
#define MaxRank 14

struct ClanData
{
        ClanData *next;    /* next clan in list            */
        ClanData *prev;    /* previous clan in list        */
        ClanList subclans;
        ClanData *next_subclan;
        ClanData *prev_subclan;
        ClanData *first_subclan;
        ClanData *last_subclan;
        ClanData *mainclan;
        char     *filename; /* Clan filename            */
        char     *name; /* Clan name                */
        char     *description;  /* A brief description of the clan  */
        char     *motto;    /* Clan Motto           */
        char     *leader;   /* Head clan leader         */
        char     *number1;  /* First officer            */
        char     *number2;  /* Second officer           */
        char     *enemy_name;   /* Enemy            */
        char     *ally_name;    /* Ally         */
        char     *roster;   /* A list of all members of that clan */
        ClanData *enemy;   /* Enemy            */
        ClanData *ally;    /* Ally         */
        int pkills; /* Number of pkills on behalf of clan   */
        int pdeaths;    /* Number of pkills against clan    */
        int mkills; /* Number of mkills on behalf of clan   */
        int mdeaths;    /* Number of clan deaths due to mobs    */
        sh_int ClanType;   /* See clan type defines        */
        sh_int members; /* Number of clan members       */
        int enlistroom; /* VNUM of the enlistment room */
        int board;  /* Vnum of clan board           */
        int storeroom;  /* Vnum of clan's store room        */
        int alignment;  /* Alignment requirement for enlistment */
        int enliston;   /* Enlist   */
        long int funds;
        int spacecraft;
        int jail;
        char     *tmpstr;
        char     *rank[MaxRank];
        int       salary[MaxRank];
		/*       rank.allowed should be a bitset
		 *       could contain flags for different commands
		 *       as well as maybe different areas
		 */
};

struct ShipData
{
        ShipData *next;
        ShipData *prev;
        /*
         * Next in body ? 
         */
        ShipData *next_in_starsystem;
        ShipData *prev_in_starsystem;
        ShipData *next_in_room;
        ShipData *prev_in_room;
        RoomIndexData *in_room;
        SpaceData *starsystem;
        ProtoshipData *prototype;
        ShipData *dockedto;
        ShipData *tractorby;
        ShipData *target0;
        ShipData *target1;
        ShipData *target2;
        SpaceData *currjump;
        ClanData *clan;
		char     *filename;
        char     *name;
        char     *home;
        char     *description;
        char     *owner;
        char     *pilot;
        char     *copilot;
        char     *dest;
        char     *selfdestruct;
		int type;
        int ship_class;
        int comm;
        int sensor;
        int astro_array;
        int hyperspeed;
        int hyperdistance;
        int realspeed;
        int currspeed;
        int shipstate;
        int statet0;
        int statet1;
        int statet2;
        int statet0i;
        int missiletype;
        int missilestate;
        int missiles;
        int maxmissiles;
        int torpedos;
        int maxtorpedos;
        int rockets;
        int maxrockets;
        int lasers;
        int tractorbeam;
        int ions;
        int manuever;
        int selfdpass;
        int maxcargo;
        int cargo;
        int cargotype;
        bool bayopen;
        bool hatchopen;
        bool autorecharge;
        bool autotrack;
        bool autospeed;
        int stealth;
        int cloak;
        int interdictor;
        float vx, vy, vz;
        float hx, hy, hz;
        float jx, jy, jz;
        int maxenergy;
        int energy;
        int shield;
        int maxshield;
        int hull;
        int maxhull;
        int cockpit;
        int turret1;
        int turret2;
        int location;
        int lastdoc;
        int shipyard;
        int entrance;
        int hanger;
        int engineroom;
        int firstroom;
        int lastroom;
        int navseat;
        int pilotseat;
        int coseat;
        int evasive;
        int gunseat;
        long collision;
        int chaff;
        int maxchaff;
        bool chaff_released;
        bool autopilot;
        int flags;
        int sim_vnum;
        int battalions;
        int maxbattalions;
		int lastbuilt;
        int bombs;
        int maxbombs;
};

struct missile_data
{
        MissileData *next;
        MissileData *prev;
        MissileData *next_in_starsystem;
        MissileData *prev_in_starsystem;
        SpaceData *starsystem;
        ShipData *target;
        ShipData *fired_from;
        char     *fired_by;
        sh_int missiletype;
        sh_int age;
        int Speed;
        int mx, my, mz;
};


/*
 * An affect.
 */
struct AffectData
{
        AffectData *next;
        AffectData *prev;
        sh_int type;
        int duration;
        sh_int location;
        int modifier;
        int bitvector;
};


/*
 * A SMAUG spell
 */
struct smaug_affect
{
        SmaugAff *next;
        char     *duration;
        sh_int location;
        char     *modifier;
        int bitvector;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
constexpr int MobVnumAnimatedCorpse = 5;
constexpr int MobVnumPolyWolf = 10;

constexpr int MobVnumGuard = 21;
constexpr int MobVnumPatrol = 23;
constexpr int MobVnumEliteGuard = 18;
constexpr int MobVnumSpecialForces = 19;
constexpr int MobVnumMercinary = 24;
constexpr int MobVnumBouncer = 25;


/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
constexpr int ActIsNpc		 = 0;   /* Auto set for mobs    */
constexpr int ActSentinel		 = 1;   /* Stays in one room    */
constexpr int ActScavenger		 = 2;   /* Picks up objects */
constexpr int ActNorunsnipe            = 3;   /* Won't run and snipe */
constexpr int ActAggressive		 = 5;   /* Attacks PC's     */
constexpr int ActStayArea		 = 6;   /* Won't leave area */
constexpr int ActWimpy		 = 7;   /* Flees when hurt  */
constexpr int ActPet			 = 8;   /* Auto set for pets    */
constexpr int ActTrain		 = 9;   /* Can train PC's   */
constexpr int ActPractice		 = 10;   /* Can practice PC's    */
constexpr int ActImmortal		 = 11;   /* Cannot be killed */
constexpr int ActDeadly		 = 12;   /* Has a deadly poison  */
constexpr int ActSpeaksall		 = 13;
constexpr int ActMetaAggr		 = 14;   /* Extremely aggressive */
constexpr int ActGuardian		 = 15;   /* Protects master  */
constexpr int ActRunning		 = 16;   /* Hunts quickly    */
constexpr int ActNowander		 = 17;   /* Doesn't wander   */
constexpr int ActMountable		 = 18;   /* Can be mounted   */
constexpr int ActMounted		 = 19;   /* Is mounted       */
constexpr int ActScholar               = 20;   /* Can teach languages  */
constexpr int ActSecretive		 = 21;   /* actions aren't seen  */
constexpr int ActPolymorphed		 = 22;   /* Mob is a ch      */
constexpr int ActMobinvis		 = 23;   /* Like wizinvis    */
constexpr int ActNoassist		 = 24;   /* Doesn't assist mobs  */
constexpr int ActNokill                = 25;   /* Mob can't die */
constexpr int ActDroid                 = 26;   /* mob is a droid */
constexpr int ActNocorpse              = 27;
constexpr int ActMail			 = 28;   /* Mail */
constexpr int ActCitizen		 = 29;   /* Planet Citizen */
constexpr int ActPrototype		 = 30;   /* A prototype mob  */
/* 20 acts */

/* bits for vip flags */

constexpr int VipCoruscant            = 0;
constexpr int VipKashyyyk          	= 1;
constexpr int VipRyloth            	= 2;
constexpr int VipRodia             	= 3;
constexpr int VipNalHutta             = 4;
constexpr int VipMonCalamari       	= 5;
constexpr int VipHonoghr              = 6;
constexpr int VipGamorr               = 7;
constexpr int VipTatooine             = 8;
constexpr int VipAdari           	= 9;
constexpr int VipByss		        = 10;
constexpr int VipEndor	        = 11;
constexpr int VipRoche                = 12;
constexpr int VipAfEl		        = 13;
constexpr int VipTrandosha	        = 14;
constexpr int VipChad		        = 15;
constexpr int VipHoth		        = 16;

/* player wanted bits */

constexpr int WantedMonCalamari   =    	VipMonCalamari;
constexpr int WantedCoruscant   	= 	VipCoruscant;
constexpr int WantedAdari   		= 	VipAdari;
constexpr int WantedRodia   		= 	VipRodia;
constexpr int WantedRyloth   	        = 	VipRyloth;
constexpr int WantedGamorr   	        = 	VipGamorr;
constexpr int WantedTatooine   	= 	VipTatooine;
constexpr int WantedByss   		= 	VipByss;
constexpr int WantedNalHutta   	= 	VipNalHutta;
constexpr int WantedKashyyyk   	= 	VipKashyyyk;
constexpr int WantedHonoghr   	= 	VipHonoghr;
constexpr int WantedEndor		= 11;
constexpr int WantedRoche		= 12;
constexpr int WantedAfEl		= 13;
constexpr int WantedTrandosha		= 14;
constexpr int WantedChad		= 15;
constexpr int WantedHoth		= 16;

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
constexpr int AffNone             =     0;

constexpr int AffBlind		  = 0;
constexpr int AffInvisible		  = 1;
constexpr int AffDetectEvil		  = 2;
constexpr int AffDetectInvis	          = 3;
constexpr int AffDetectMagic	          = 4;
constexpr int AffDetectHidden	          = 5;
constexpr int AffWeaken		  = 6;
constexpr int AffSanctuary		  = 7;
constexpr int AffFaerieFire		  = 8;
constexpr int AffInfrared		  = 9;
constexpr int AffCurse		  = 10;
constexpr int AffSecretive		  = 11;  /* Unused   */
constexpr int AffPoison		  = 12;
constexpr int AffProtect		  = 13;
constexpr int AffParalysis		  = 14;
constexpr int AffSneak		  = 15;
constexpr int AffHide		          = 16;
constexpr int AffSleep		  = 17;
constexpr int AffCharm		  = 18;
constexpr int AffFlying		  = 19;
constexpr int AffPassDoor		  = 20;
constexpr int AffFloating		  = 21;
constexpr int AffTruesight		  = 22;
constexpr int AffDetecttraps		  = 23;
constexpr int AffScrying	          = 24;
constexpr int AffFireshield	          = 25;
constexpr int AffShockshield	          = 26;
constexpr int AffRestrained             = 27; /* not used */
constexpr int AffIceshield  		  = 28;
constexpr int AffPossess		  = 29;
constexpr int AffBerserk		  = 30;
constexpr int AffAquaBreath		  = 31;

/* 31 aff's (1 left.. :P) */
/* make that none - ugh - time for another field? :P */
/*
 * Resistant Immune Susceptible flags
 */
constexpr int RisFire		          = 0;
constexpr int RisCold		          = 1;
constexpr int RisElectricity		  = 2;
constexpr int RisEnergy		  = 3;
constexpr int RisBlunt		  = 4;
constexpr int RisPierce		  = 5;
constexpr int RisSlash		  = 6;
constexpr int RisAcid		          = 7;
constexpr int RisPoison		  = 8;
constexpr int RisDrain		  = 9;
constexpr int RisSleep		  = 10;
constexpr int RisCharm		  = 11;
constexpr int RisHold		          = 12;
constexpr int RisNonmagic		  = 13;
constexpr int RisPlus1		  = 14;
constexpr int RisPlus2		  = 15;
constexpr int RisPlus3		  = 16;
constexpr int RisPlus4		  = 17;
constexpr int RisPlus5		  = 18;
constexpr int RisPlus6		  = 19;
constexpr int RisMagic		  = 20;
constexpr int RisParalysis		  = 21;
/* 21 RIS's*/

/* 
 * Attack types
 */

constexpr int AtckBite                = 0;
constexpr int AtckClaws               = 1;
constexpr int AtckTail                = 2;
constexpr int AtckSting               = 3;
constexpr int AtckPunch               = 4;
constexpr int AtckKick                = 5;
constexpr int AtckTrip                = 6;
constexpr int AtckBackstab            = 10;


/*
 * Defense types
 */
constexpr int DfndParry               = 0;
constexpr int DfndDodge               = 1;
constexpr int DfndDisarm              = 19;
constexpr int DfndGrip                = 21;
/* 2 def's */

/*
 * Body parts
 */
typedef enum
{
        PartHead, PartArms, PartLegs, PartHeart, PartBrains,
        PartGuts, PartHangs, PartFeet, PartFingers, PartEar,
        PartEye, PartLongTongue, PartEyestalks, PartTentacles, PartFins,
        PartWings, PartTail, PartScales, PartClaws, PartFangs,
        PartHorns, PartTusks, PartTailattack, PartSharpscales, PartBeak,
        PartHaunch, PartHooves, PartForelegs, PartFeathers
} body_part;


/*
 * Autosave flags
 */
constexpr int SvDeath		          = 0;
constexpr int SvKill			  = 1;
constexpr int SvPasschg		  = 2;
constexpr int SvDrop			  = 3;
constexpr int SvPut			  = 4;
constexpr int SvGive			  = 5;
constexpr int SvAuto			  = 6;
constexpr int SvZapdrop		  = 7;
constexpr int SvAuction		  = 8;
constexpr int SvGet			  = 9;
constexpr int SvReceive		  = 10;
constexpr int SvIdle			  = 11;
constexpr int SvBackup		  = 12;
constexpr int SvWho		  	  = 13;
constexpr int SvScore		          = 14;
constexpr int SvList		  	  = 15;
constexpr int SvN		  	  = 16;
constexpr int SvE			  = 17;
constexpr int SvS			  = 18;
constexpr int SvW			  = 19;
constexpr int SvNe		  	  = 20;
constexpr int SvSe			  = 21;
constexpr int SvNw			  = 22;
constexpr int SvSw			  = 23;
constexpr int SvLook			  = 24;

/*
 * Pipe flags
 */
constexpr int PipeTamped		  = 1;
constexpr int PipeLit		          = 2;
constexpr int PipeHot		          = 3;
constexpr int PipeDirty		  = 4;
constexpr int PipeFilthy		  = 5;
constexpr int PipeGoingout		  = 6;
constexpr int PipeBurnt		  = 7;
constexpr int PipeFullofash		  = 8;

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
constexpr int SfWater		          = 11;
constexpr int SfEarth		          = 12;
constexpr int SfAir			  = 13;
constexpr int SfAstral		  = 14;
constexpr int SfArea			  = 15;  /* is an area spell     */
constexpr int SfDistant		  = 16;  /* affects something far away   */
constexpr int SfReverse		  = 17;
constexpr int SfSaveHalfDamage	  = 18;  /* save for half damage     */
constexpr int SfSaveNegates		  = 19;  /* save negates affect      */
constexpr int SfAccumulative		  = 20;  /* is accumulative      */
constexpr int SfRecastable		  = 21;  /* can be refreshed     */
constexpr int SfNoscribe		  = 22;  /* cannot be scribed        */
constexpr int SfNobrew		  = 23;  /* cannot be brewed     */
constexpr int SfGroupspell		  = 24;  /* only affects group members   */
constexpr int SfObject		  = 25;  /* directed at an object    */
constexpr int SfCharacter		  = 26;  /* directed at a character  */
constexpr int SfSecretskill		  = 27;  /* hidden unless learned    */
constexpr int SfPksensitive		  = 28;  /* much harder for plr vs. plr  */
constexpr int SfStoponfail		  = 29;  /* stops spell on first failure */

typedef enum
{ SsNone, SsPoisonDeath, SsRodWands, SsParaPetri,
        SsBreath, SsSpellStaff
} save_types;

#define AllBits		IntMax
#define SdamMask		AllBits & ~(= 0; | = 1; | = 2;)
#define SactMask		AllBits & ~(= 3; | = 4; | = 5;)
#define SclaMask		AllBits & ~(= 6; | = 7; | = 8;)
#define SpowMask		AllBits & ~(= 9; | = 10;)

typedef enum
{ SdNone, SdFire, SdCold, SdElectricity, SdEnergy, SdAcid,
        SdPoison, SdDrain
} spell_dam_types;

typedef enum
{ SaNone, SaCreate, SaDestroy, SaResist, SaSuscept,
        SaDivinate, SaObscure, SaChange
} spell_act_types;

typedef enum
{ SpNone, SpMinor, SpGreater, SpMajor } spell_power_types;

typedef enum
{ ScNone, ScLunar, ScSolar, ScTravel, ScSummon,
        ScLife, ScDeath, ScIllusion
} spell_class_types;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum
{ SexNeutral, SexMale, SexFemale, SexMax } sex_types;

typedef enum
{
        TrapTypePoisonGas =
                1, TrapTypePoisonDart, TrapTypePoisonNeedle,
        TrapTypePoisonDagger, TrapTypePoisonArrow, TrapTypeBlindnessGas,
        TrapTypeSleepingGas, TrapTypeFlame, TrapTypeExplosion,
        TrapTypeAcidSpray, TrapTypeElectricShock, TrapTypeBlade,
        TrapTypeSexChange
} trap_types;

#define MaxTraptype		   TrapTypeSexChange

constexpr int TrapRoom      		   = 0;
constexpr int TrapObj	      	           = 1;
constexpr int TrapEnterRoom		   = 2;
constexpr int TrapLeaveRoom		   = 3;
constexpr int TrapOpen		   = 4;
constexpr int TrapClose		   = 5;
constexpr int TrapGet		           = 6;
constexpr int TrapPut		           = 7;
constexpr int TrapPick		   = 8;
constexpr int TrapUnlock		   = 9;
constexpr int TrapN			   = 10;
constexpr int TrapS			   = 11;
constexpr int TrapE	      		   = 12;
constexpr int TrapW	      		   = 13;
constexpr int TrapU	      		   = 14;
constexpr int TrapD	      		   = 15;
constexpr int TrapExamine		   = 16;
constexpr int TrapNe			   = 17;
constexpr int TrapNw			   = 18;
constexpr int TrapSe			   = 19;
constexpr int TrapSw			   = 20;

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
constexpr int ObjVnumMoneyOne	      =        2;
constexpr int ObjVnumMoneySome	      =        3;

constexpr int ObjVnumDroidCorpse            =        9;
constexpr int ObjVnumCorpseNpc	     =       10;
constexpr int ObjVnumCorpsePc	             =       11;
constexpr int ObjVnumSeveredHead	     =       12;
constexpr int ObjVnumTornHeart	     =       13;
constexpr int ObjVnumSlicedArm	     =       14;
constexpr int ObjVnumSlicedLeg	     =       15;
constexpr int ObjVnumSpilledGuts	     =       16;
constexpr int ObjVnumBlood		     =       17;
constexpr int ObjVnumBloodstain	     =       18;
constexpr int ObjVnumScraps		     =       19;

constexpr int ObjVnumMushroom	             =       20;
constexpr int ObjVnumLightBall	     =       21;
constexpr int ObjVnumSpring		     =       22;

constexpr int ObjVnumSlice		     =       24;
constexpr int ObjVnumShoppingBag	     =       25;

constexpr int ObjVnumFire		     =       30;
constexpr int ObjVnumTrap		     =       31;
constexpr int ObjVnumPortal		     =       32;

constexpr int ObjVnumBlackPowder	     =       33;
constexpr int ObjVnumScrollScribing        =       34;
constexpr int ObjVnumFlaskBrewing          =       35;
constexpr int ObjVnumSharpen               =       39;

constexpr int ObjVnumDiamondRing	     =       65;
constexpr int ObjVnumRestraint             =       66;
constexpr int ObjVnumWeddingBand           =       67;
constexpr int ObjVnumMedkit                =       67;

/* Academy eq */
constexpr int ObjVnumSchoolMace	        =       10315;
constexpr int ObjVnumSchoolDagger	        =       10312;
constexpr int ObjVnumSchoolSword	        =       10313;
constexpr int ObjVnumSchoolVest	        =       10308;
constexpr int ObjVnumSchoolShield	        =       10310;
constexpr int ObjVnumSchoolBanner	        =       10311;
constexpr int ObjVnumSchoolDiploma        =       10321;
constexpr int ObjVnumSchoolMoney          =       10431;

constexpr int ObjVnumBlastechE11        =       50;

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
        ItemNone, ItemLight, ItemScroll, ItemWand, ItemStaff, ItemWeapon,
        ItemFireweapon, ItemMissile, ItemTreasure, ItemArmor, ItemPotion,
        ItemWorn, ItemFurniture, ItemTrash, ItemOldtrap, ItemContainer,
        ItemNote, ItemDrinkCon, ItemKey, ItemFood, ItemMoney, ItemPen,
        ItemBoat, ItemCorpseNpc, ItemCorpsePc, ItemFountain, ItemPill,
        ItemBlood, ItemBloodstain, ItemScraps, ItemPipe, ItemHerbCon,
        ItemHerb, ItemIncense, ItemFire, ItemBook, ItemSwitch, ItemLever,
        ItemPullchain, ItemButton, ItemBeacon, ItemTrap, ItemRunepouch,
        ItemMatch, ItemRawMetal, ItemCanister, ItemPortal, ItemPaper,
        ItemTinder, ItemLockpick, ItemSpike, ItemDisease, ItemOil, ItemFuel,
        ItemRestraint, ItemLongBow, ItemCrossbow, ItemAmmo, ItemQuiver,
        ItemShovel, ItemSalve, ItemRawspice, ItemLens, ItemCrystal, ItemDuraplast,
        ItemBattery, ItemToolkit, ItemDurasteel, ItemOven, ItemMirror,
        ItemCircuit, ItemSuperconductor, ItemComlink, ItemMedpac, ItemFabric,
        ItemRareMetal, ItemMagnet, ItemThread, ItemSpice, ItemSmut, ItemDevice, ItemSpacecraft,
        ItemGrenade, ItemLandmine, ItemGovernment, ItemDroidCorpse, ItemBolt, ItemBond,
        ItemImplant, ItemChemical, ItemBinding, ItemHolster, ItemLanddeed,
        ItemMax
} item_types;


#define MaxItemType		     (ItemMax-1)
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
constexpr int ItemGlow		= 0;
constexpr int ItemHum		        = 1;
constexpr int ItemDark		= 2;
constexpr int ItemHuttSize		= 3;
constexpr int ItemContraband		= 4;
constexpr int ItemInvis		= 5;
constexpr int ItemMagic		= 6;
constexpr int ItemNodrop		= 7;
constexpr int ItemBless		= 8;
constexpr int ItemAntiGood		= 9;
constexpr int ItemAntiEvil		= 10;
constexpr int ItemAntiNeutral	        = 11;
constexpr int ItemNoremove		= 12;
constexpr int ItemInventory		= 13;
constexpr int ItemAntiSoldier	        = 14;
constexpr int ItemAntiThief	        = 15;
constexpr int ItemAntiHunter	        = 16;
constexpr int ItemAntiJedi  	        = 17;
constexpr int ItemSmallSize		= 18;
constexpr int ItemLargeSize		= 19;
constexpr int ItemDonation		= 20;
constexpr int ItemClanobject		= 21;
constexpr int ItemAntiCitizen	        = 22;
constexpr int ItemAntiSith  	        = 23;
constexpr int ItemAntiPilot	        = 24;
constexpr int ItemHidden		= 25;
constexpr int ItemPoisoned		= 26;
constexpr int ItemCovering		= 27;
constexpr int ItemDeathrot		= 28;
constexpr int ItemBurried		= 29;    /* item is underground */
constexpr int ItemPrototype		= 30;
constexpr int ItemHumanSize           = 31;

/* Magic flags - extra extra_flags for objects that are used in spells */
constexpr int ItemReturning		= 0;
constexpr int ItemBackstabber  	= 1;
constexpr int ItemBane		= 2;
constexpr int ItemLoyal		= 3;
constexpr int ItemHaste		= 4;
constexpr int ItemDrain		= 5;
constexpr int ItemLightningBlade  	= 6;

/* Blaster settings - only saves on characters */
constexpr int BlasterNormal     =     0;
constexpr int BlasterHalf       =     2;
constexpr int BlasterFull       =     5;
constexpr int BlasterLow        =     1;
constexpr int BlasterStun      =     3;
constexpr int BlasterHigh      =     4;

/* Weapon Types */

constexpr int WeaponNone     	        =     0;
constexpr int WeaponVibroAxe	        =     1;
constexpr int WeaponVibroBlade	        =     2;
constexpr int WeaponLightsaber	        =     3;
constexpr int WeaponWhip  		=     4;
constexpr int WeaponKnife		=     5;
constexpr int WeaponBlaster		=     6;
constexpr int WeaponBludgeon		=     8;
constexpr int WeaponBowcaster           =     9;
constexpr int WeaponForcePike	        =     11;


/* Furniture Settings */
#define StandAt                1
#define StandOn                2
#define StandIn                3
#define SitAt                  1
#define SitOn                  2
#define SitIn                  3
#define RestAt                 1
#define RestOn                 2
#define RestIn                 3
#define SleepAt                1
#define SleepOn                2
#define SleepIn                3
#define PutAt                  1
#define PutOn                  2
#define PutIn                  3
#define PutInside              4


/* Lever/dial/switch/button/pullchain flags */
constexpr int TrigUp			= 0;
constexpr int TrigUnlock		= 1;
constexpr int TrigLock		= 2;
constexpr int TrigDNorth		= 3;
constexpr int TrigDSouth		= 4;
constexpr int TrigDEast		= 5;
constexpr int TrigDWest		= 6;
constexpr int TrigDUp		        = 7;
constexpr int TrigDDown		= 8;
constexpr int TrigDoor		= 9;
constexpr int TrigContainer		= 10;
constexpr int TrigOpen		= 11;
constexpr int TrigClose		= 12;
constexpr int TrigPassage		= 13;
constexpr int TrigOload		= 14;
constexpr int TrigMload		= 15;
constexpr int TrigDeath		= 19;
constexpr int TrigCast		= 20;
constexpr int TrigFakeblade		= 21;
constexpr int TrigRand4		= 22;
constexpr int TrigRand6		= 23;
constexpr int TrigTrapdoor		= 24;
constexpr int TrigAnotheroom		= 25;
constexpr int TrigUsedial		= 26;    /* Unused */
constexpr int TrigAbsolutevnum	= 27;
constexpr int TrigShowroomdesc	= 28;
constexpr int TrigAutoreturn		= 29;

/* drug types */
constexpr int SpiceGlitterstim        = 0;
constexpr int SpiceCarsanum           = 1;
constexpr int SpiceRyll               = 2;
constexpr int SpiceAndris             = 3;

/* crystal types */
constexpr int GemNonAdegen           = 0;
constexpr int GemKathracite          = 1;
constexpr int GemRelacite            = 2;
constexpr int GemDanite              = 3;
constexpr int GemMephite             = 4;
constexpr int GemPonite              = 5;
constexpr int GemIllum               = 6;
constexpr int GemCorusca             = 7;

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
constexpr int ItemTake		= 0;
constexpr int ItemWearFinger	        = 1;
constexpr int ItemWearNeck		= 2;
constexpr int ItemWearBody		= 3;
constexpr int ItemWearHead		= 4;
constexpr int ItemWearLegs		= 5;
constexpr int ItemWearFeet		= 6;
constexpr int ItemWearHands		= 7;
constexpr int ItemWearArms		= 8;
constexpr int ItemWearShield	        = 9;
constexpr int ItemWearAbout		= 10;
constexpr int ItemWearWaist		= 11;
constexpr int ItemWearWrist		= 12;
constexpr int ItemWield		= 13;
constexpr int ItemHold		= 14;
constexpr int ItemDualWield		= 15;
constexpr int ItemWearEars		= 16;
constexpr int ItemWearEyes		= 17;
constexpr int ItemMissileWield	= 18;
constexpr int ItemWearBinding         = 19;
constexpr int ItemWearHolster1	= 20;

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
        ApplyNone, ApplyStr, ApplyDex, ApplyInt, ApplyWis, ApplyCon,
        ApplySex, ApplyNull, ApplyLevel, ApplyAge, ApplyHeight,
        ApplyWeight,
        ApplyMana, ApplyHit, ApplyMove, ApplyGold, ApplyExp, ApplyAc,
        ApplyHitroll, ApplyDamroll, ApplySavingPoison, ApplySavingRod,
        ApplySavingPara, ApplySavingBreath, ApplySavingSpell, ApplyCha,
        ApplyAffect, ApplyResistant, ApplyImmune, ApplySusceptible,
        ApplyWeaponspell, ApplyLck, ApplyBackstab, ApplyPick, ApplyTrack,
        ApplySteal, ApplySneak, ApplyHide, ApplyPalm, ApplyDetrap, ApplyDodge,
        ApplyPeek, ApplyScan, ApplyGouge, ApplySearch, ApplyMount, ApplyDisarm,
        ApplyKick, ApplyParry, ApplyBash, ApplyStun, ApplyPunch, ApplyClimb,
        ApplyGrip, ApplyScribe, ApplyBrew, ApplyWearspell, ApplyRemovespell,
        ApplyEmotion, ApplyMentalstate, ApplyStripsn, ApplyRemove, ApplyDig,
        ApplyFull, ApplyThirst, ApplyDrunk, ApplyBlood, ApplySecretive, MaxApplyType
} apply_types;

#define ReverseApply		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define ContCloseable		      1
#define ContPickproof		      2
#define ContClosed		      4
#define ContLocked		      8

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
constexpr int RoomVnumLimbo		    = 2;
constexpr int RoomVnumPoly		    = 3;
constexpr int RoomCloneEnd	        = 10000;
constexpr int RoomCloneBegin	        = 10001;
constexpr int RoomVnumChat	        = 32144;
constexpr int RoomVnumTemple	        = 32144;
constexpr int RoomVnumAltar	        = 32144;
constexpr int RoomVnumSchool	        = 227;
constexpr int RoomAuthStart	        = 227;
constexpr int RoomStartHuman	        = 211;
constexpr int RoomStartWookiee	    = 28600;
constexpr int RoomStartTwilek         = 32148;
constexpr int RoomStartRodian         = 32148;
constexpr int RoomStartHutt           = 32148;
constexpr int RoomStartMonCalamarian  = 21069;
constexpr int RoomStartNoghri          = 1015;
constexpr int RoomStartGamorrean      = 28100;
constexpr int RoomStartJawa           = 31819;
constexpr int RoomStartAdarian        = 29000;
constexpr int RoomStartEwok           = 32148;
constexpr int RoomStartVerpine        = 32148;
constexpr int RoomStartCsillian       = 32148;
constexpr int RoomStartDefel          = 32148;
constexpr int RoomStartTrandoshan     = 32148;
constexpr int RoomStartShistavanan    = 32148;
constexpr int RoomStartDuinuogwuin    = 32148;
constexpr int RoomStartDroid          = 21069;
constexpr int RoomStartImmortal         = 100;
constexpr int RoomLimboShipyard          = 45;
constexpr int RoomDefaultCrash        = 28025;

constexpr int RoomPluogusQuit         = 32148;

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 * Leave RoomReserved alone, its reserved for tracking.
 */
typedef enum
{
        RoomDark, RoomReserved, RoomNoMob, RoomIndoors, RoomCanLand,
        RoomCanFly, RoomNoDriving, RoomNoMagic, RoomBank,
        RoomPrivate, RoomSafe, RoomInstallation, RoomPetShop,
        RoomNoHailTo, RoomDonation, RoomNodropall,
        RoomSilence, RoomLogspeech, RoomNodrop, RoomClanstoreroom,
        RoomPlrHome, RoomEmptyHome, RoomNothing, RoomHotel,
        RoomNofloor, RoomRefinery, RoomFactory, RoomRecruit, RoomERecruit,
        RoomSpacecraft, RoomPrototype, RoomAuction, RoomBar, RoomInn,
        RoomOffice, RoomCafe, RoomKitchen, RoomExecutive, RoomBoardroom,
        RoomBacta, RoomImport, RoomBactaCharge, RoomArena, RoomBounty,
        RoomTempCockpit, RoomTempTurret1, RoomTempTurret2, RoomTempHangar,
        RoomTempEngineroom, RoomTempNavseat, RoomTempPilotseat, RoomTempCoseat,
        RoomTempGunseat, RoomTempCorridor, RoomTempBedroom, RoomTempWorkshop,
        RoomTempTurbolift, RoomPlayershop, RoomMonitor, RoomEmptyplot, MaxRoomFlag
} roomflags;

/*Ship Flags*/
constexpr int ShipSimulator	   = 0;
constexpr int ShipCloak          = 1;
constexpr int ShipStealth        = 2;
constexpr int ShipInterdictor    = 3;
constexpr int ShipRepublic       = 4;
constexpr int ShipEmpire         = 5;
constexpr int ShipHunter         = 6;
constexpr int ShipSmuggler       = 7;
constexpr int ShipPirate         = 8;
constexpr int ShipPolice         = 9;
constexpr int ShipClan1          = 10;
constexpr int ShipClan2          = 11;
constexpr int ShipClan3          = 12;
constexpr int ShipClan4          = 13;
constexpr int ShipClan5          = 14;
constexpr int ShipTroop          = 15;
constexpr int ShipTaxi           = 16;



/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
        DirNorth, DirEast, DirSouth, DirWest, DirUp, DirDown,
        DirNortheast, DirNorthwest, DirSoutheast, DirSouthwest,
        DirSomewhere
} dir_types;

#define MaxDir			DirSouthwest   /* max for normal walking */
#define DirPortal		DirSomewhere   /* portal direction   */


/*
 * Exit flags.
 * Used in #ROOMS.
 */
constexpr int ExIsdoor		  = 0;
constexpr int ExClosed		  = 1;
constexpr int ExLocked		  = 2;
constexpr int ExSecret		  = 3;
constexpr int ExSwim			  = 4;
constexpr int ExPickproof		  = 5;
constexpr int ExFly			  = 6;
constexpr int ExClimb		          = 7;
constexpr int ExDig			  = 8;
constexpr int ExRes1                    = 9;  /* are these res[1-4] important? */
constexpr int ExNopassdoor		  = 10;
constexpr int ExHidden		  = 11;
constexpr int ExPassage		  = 12;
constexpr int ExPortal 		  = 13;
constexpr int ExRes2			  = 14;
constexpr int ExRes3			  = 15;
constexpr int EX_xCLIMB		  = 16;
constexpr int EX_xENTER		  = 17;
constexpr int EX_xLEAVE		  = 18;
constexpr int EX_xAUTO		  = 19;
constexpr int ExRes4	  		  = 20;
constexpr int EX_xSEARCHABLE		  = 21;
constexpr int ExBashed                  = 22;
constexpr int ExBashproof               = 23;
constexpr int ExNomob		          = 24;
constexpr int ExWindow		  = 25;
constexpr int EX_xLOOK		  = 26;
constexpr int ExRubble                  = 27;
#define MaxExflag		    27

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
        SectInside, SectCity, SectField, SectForest, SectHills, SectMountain,
        SectWaterSwim, SectWaterNoswim, SectUnderwater, SectAir, SectDesert,
        SectDunno, SectOceanfloor, SectUnderground, SectSpacecraft, SectMax
} sector_types;

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
        WearNone = -1, WearLight =
                0, WearFingerL, WearFingerR, WearNeck1,
        WearNeck2, WearBody, WearHead, WearLegs, WearFeet, WearHands,
        WearArms, WearShield, WearAbout, WearWaist, WearWristL, WearWristR,
        WearWield, WearHold, WearDualWield, WearEars, WearEyes,
        WearMissileWield, WearBinding, WearHolsterL, WearHolsterR, MaxWear
} wear_locations;

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
        CondDrunk, CondFull, CondThirst, CondBloodthirst, MaxConds
} conditions;

/*
 * Positions.
 */
typedef enum
{
        PosDead, PosMortal, PosIncap, PosStunned, PosSleeping, PosResting,
        PosSitting, PosFighting, PosStanding, PosMounted, PosShove, PosDrag,
        PosMax
} positions;

/*
 * ACT bits for players.
 */


constexpr int PlrIsNpc		   = 0;  /* Don't EVER set.  */
constexpr int PlrBoughtPet		   = 1;
constexpr int PlrShovedrag		   = 2;
constexpr int PlrAutoexit		   = 3;
constexpr int PlrAutoloot		   = 4;
constexpr int PlrForsaken                = 5;
constexpr int PlrBlank		   = 6;
constexpr int PlrSecretive 		   = 7;
constexpr int PlrBrief		   = 8;
constexpr int PlrCombine		   = 9;
constexpr int PlrPrompt		   = 10;
constexpr int PlrTelnetGa		   = 11;
constexpr int PlrHolylight		   = 12;
constexpr int PlrWizinvis		   = 13;
constexpr int PlrRoomvnum		   = 14;
constexpr int PlrSilence		   = 15;
constexpr int PlrNoEmote		   = 16;
constexpr int PlrAttacker    	           = 17;
constexpr int PlrNoTell		   = 18;
constexpr int PlrLog			   = 19;
constexpr int PlrDeny		           = 20;
constexpr int PlrFreeze		   = 21;
constexpr int PlrKiller    	           = 22;
constexpr int PlrQuestor  	           = 23;
constexpr int PlrLitterbug	           = 24;
constexpr int PlrAnsi	                   = 25;
constexpr int PlrSound	           = 26;
constexpr int PlrNice	                   = 27;
constexpr int PlrFlee	                   = 28;
constexpr int PlrAutogold                = 29;
constexpr int PlrMxp		           = 30;
constexpr int PlrAfk                     = 31;

/* Bits for pc_data->flags. */
constexpr int PcflagR1                   = 0;
constexpr int PcflagNohunger	    	   = 1;
constexpr int PcflagUnauthed	    	   = 2;
constexpr int PcflagNorecall             = 3;
constexpr int PcflagNointro              = 4;
constexpr int PcflagGag	           = 5;
constexpr int PcflagRetired              = 6;
constexpr int PcflagGuest                = 7;
constexpr int PcflagNosummon	           = 8;
constexpr int PcflagPageron	           = 9;
constexpr int PcflagNotitle              = 10;
constexpr int PcflagRoom                 = 11;
constexpr int PcflagWorking              = 12;
constexpr int PcflagGotmail              = 13;
constexpr int PcflagExempt               = 14;
constexpr int PcflagFastengineer         = 15;
constexpr int PcflagWatch                = 16; /* see function "do_watch" */
constexpr int PcflagMarried              = 17;
constexpr int PcflagNewbguide            = 18;
constexpr int PcflagAutodraw             = 19;

/* Bits for ch->PCData->godflags */
constexpr int ImmAdmin                  = 0;
constexpr int ImmBuilder                = 1;
constexpr int ImmHighbuilder            = 2;
constexpr int ImmEnforcer               = 3;
constexpr int ImmQuest                  = 4;
constexpr int ImmCoder                  = 5;
constexpr int ImmAll                    = 6;
constexpr int ImmOwner                  = 7;
constexpr int ImmHighenforcer           = 8;

/* Bits for command->flags */
constexpr int CommandAdmin                  = 0;
constexpr int CommandBuilder                = 1;
constexpr int CommandHighbuilder            = 2;
constexpr int CommandEnforcer               = 3;
constexpr int CommandQuest                  = 4;
constexpr int CommandCoder                  = 5;
constexpr int CommandAll                    = 6;
constexpr int CommandOwner                  = 7;
constexpr int CommandHighenforcer           = 8;



typedef enum
{
        TimerNone, TimerRecentfight, TimerShovedrag, TimerDoFun,
        TimerApplied, TimerPkilled
} timer_types;

#include <functional>

struct timer_data
{
        Timer* prev; // Raw pointer for previous node
        std::unique_ptr<Timer> next;
        std::function<void(CharData*, char*)> do_fun;
        std::int32_t value;
        std::int16_t type;
        std::int16_t count;
};


/*
 * Channel bits.
 */
typedef enum
{
        ChannelLog, ChannelBuild, ChannelComm, ChannelTells,
        ChannelAuction
} channels;

/* Area defines - Scryn 8/11
 *
 */
#define AreaDeleted		   = 0;
#define AreaLoaded                = 1;

/* Area flags - Narn Mar/96 */
#define AflagNopkill               = 0;
#define AflagNoquest               = 1;
#define AflagPrototype             = 2;

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct MobIndexData
{
        MobIndexData *next;
        MobIndexData *next_sort;
        SpecFun *spec_fun;
        SpecFun *spec_2;
        ShopData *pShop;
        RepairData *rShop;
        MProgData *mudprogs;
        RaceData *race;
        int progtypes;
        char     *PlayerName;
        char     *short_descr;
        char     *long_descr;
        char     *description;
        char     *spec_funname;
        char     *spec2_funname;
        char     *mob_clan;
        int vnum;
        int max_hit;
        sh_int count;
        sh_int killed;
        sh_int sex;
        sh_int level;
//      int act; // Unused
        ExtBV act;
        int affected_by;
        sh_int alignment;
        sh_int mobthac0;    /* Unused */
        sh_int ac;
        sh_int hitnodice;
        sh_int hitsizedice;
        sh_int hitplus;
        sh_int damnodice;
        sh_int damsizedice;
        sh_int damplus;
        sh_int numattacks;
        unsigned int gold;
        int exp;
        ExtBV xflags;
        int resistant;
        int immune;
        int susceptible;
        int attacks;
        int defenses;
        LanguageData *speaking;
        int position;
        sh_int defposition;
        sh_int height;
        sh_int weight;
        sh_int Hitroll;
        sh_int Damroll;
        sh_int perm_str;
        sh_int perm_int;
        sh_int perm_wis;
        sh_int perm_dex;
        sh_int perm_con;
        sh_int perm_cha;
        sh_int perm_lck;
        sh_int perm_frc;
        sh_int SavingPoisonDeath;
        sh_int SavingWand;
        sh_int SavingParaPetri;
        sh_int SavingBreath;
        sh_int SavingSpellStaff;
};


struct hunt_hate_fear
{
        char     *name;
        CharData *who;
};

struct fighting_data
{
        CharData *who;
        int xp;
        sh_int align;
        sh_int duration;
        sh_int timeskilled;
};

struct extracted_char_data
{
        ExtractCharData *next;
        CharData *ch;
        RoomIndexData *room;
        ch_ret retcode;
        bool extract;
};



/*
 * Body Parts Stuff
 */
constexpr int BodyNone  = 0;
constexpr int BodyLLeg   = 1;
constexpr int BodyRLeg   = 2;
constexpr int BodyLFoot  = 3;
constexpr int BodyRFoot  = 4;
constexpr int BodyLArm   = 5;
constexpr int BodyRArm   = 6;
constexpr int BodyLWrist = 7;
constexpr int BodyRWrist = 8;
constexpr int BodyLKnee  = 9;
constexpr int BodyRKnee  = 10;
constexpr int BodyLAnkle = 11;
constexpr int BodyRAnkle = 12;
constexpr int BodyLShoulder = 13;
constexpr int BodyRShoulder = 14;
constexpr int BodyLHand  = 15;
constexpr int BodyRHand  = 16;
constexpr int BodyNose   = 17;
constexpr int BodyRibs   = 18;
constexpr int BodyJaw    = 19;
constexpr int BodyStomach= 20;
constexpr int BodyChest  = 21;
constexpr int MaxBodyParts = 22;

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
struct PCData;
struct CharData
{
        CharData *next;
        CharData *prev;
        CharData *next_in_room;
        CharData *prev_in_room;
        CharData *next_in_area;
        CharData *prev_in_area;
        CharData *master;
        CharData *leader;
        FightData *fighting;
        CharData *heldby;  /* Hold/bind/struggle/subdue. -Ulthrax */
        bool held;
        CharData *holding;
        CharData *reply;
        CharData *oreply;
        CharData *switched;
        CharData *mount;
        CharData *questgiver;  /* Vassago */
        HHFData *hunting;
        HHFData *fearing;
        HHFData *hating;
        SpecFun *spec_fun;
        SpecFun *spec_2;
        RaceData *race;
        char     *spec_funname;
        char     *spec2_funname;
        MProgActList *mpact;
        int mpactnum;
        sh_int mpscriptpos;
        MobIndexData *pIndexData;
        DescriptorData *desc;
        AffectData *first_affect;
        AffectData *last_affect;
        struct note_data *pnote;
        ObjData *first_carrying;
        ObjData *last_carrying;
        RoomIndexData *in_room;
        std::unique_ptr<RoomIndexData> was_in_room;
        std::unique_ptr<RoomIndexData> was_sentinel;
        std::unique_ptr<RoomIndexData> plr_home;
        std::unique_ptr<PcData> PCData;
        std::unique_ptr<DoFun>   last_cmd;
        std::unique_ptr<DoFun>   prev_cmd; /* mapping */
        std::unique_ptr<char[]>     dest_buf;
        void     *dest_buf_2;
        void     *dest_buf_3;
        void     *spare_ptr;
        char     *alloc_ptr;
        int tempnum;
        ExtBV act; // used to be long int, but act is 32 bits of flags
        struct editor_data *editor;
        Timer    *first_timer;
        Timer    *last_timer;
        char     *name;
        char     *short_descr;
        char     *long_descr;
        char     *description;
        sh_int num_fighting;
        sh_int substate;
        sh_int sex;
        sh_int top_level;
        sh_int skill_level[MaxAbility];
        sh_int trust;
        ObjData *on;
        ExtBV played;
        time_t logon;
        time_t save_time;
        sh_int timer;
        sh_int wait;
        sh_int hit;
        sh_int max_hit;
        sh_int endurance;
        sh_int max_endurance;
        sh_int numattacks;
        sh_int nextquest;   /* Vassago */
        sh_int countdown;   /* Vassago */
        sh_int questobj;    /* Vassago */
        sh_int questmob;    /* Vassago */
        sh_int questhp; /* Greven */
        long int gold;
        long experience[MaxAbility];
 //       ExtBV act; // duplicate?
        ExtBV affected_by;
        ExtBV carry_weight;
        ExtBV carry_number;
        ExtBV xflags;
        ExtBV resistant;
        ExtBV immune;
        ExtBV susceptible;
        ExtBV attacks;
        ExtBV defenses;
        LanguageData *speaking;
        sh_int SavingPoisonDeath;
        sh_int SavingWand;
        sh_int SavingParaPetri;
        sh_int SavingBreath;
        sh_int SavingSpellStaff;
        sh_int alignment;
        sh_int barenumdie;
        sh_int baresizedie;
        sh_int mobthac0;
        sh_int Hitroll;
        sh_int Damroll;
        sh_int hitplus;
        sh_int damplus;
        sh_int position;
        sh_int defposition;
        sh_int height;
        sh_int weight;
        sh_int Armor;
        sh_int wimpy;
        ExtBV deaf;
        sh_int perm_str;
        sh_int perm_int;
        sh_int perm_wis;
        sh_int perm_dex;
        sh_int perm_con;
        sh_int perm_cha;
        sh_int perm_lck;
        sh_int perm_frc;
        sh_int bonus_str;
        sh_int bonus_int;
        sh_int bonus_wis;
        sh_int bonus_dex;
        sh_int bonus_con;
        sh_int bonus_cha;
        sh_int bonus_lck;
        sh_int bonus_frc;
        sh_int mod_str;
        sh_int mod_int;
        sh_int mod_wis;
        sh_int mod_dex;
        sh_int mod_con;
        sh_int mod_cha;
        sh_int mod_lck;
        sh_int mod_frc;
        sh_int mental_state;    /* simplified */
        sh_int emotional_state; /* simplified */
        ExtBV retran;
        ExtBV regoto;
        ExtBV gpoint; /* personal goto point */
        sh_int mobinvis;    /* Mobinvis level SB */
        sh_int backup_wait; /* reinforcements */
        ExtBV backup_mob; /* reinforcements */
        sh_int was_stunned;
        char     *mob_clan; /* for spec_clan_guard.. set by postguard */
        sh_int main_ability;
        char     *owner;
        RoomIndexData *home;
        sh_int colors[MaxColors];
        ExtBV bodyparts;
        ExtBV home_vnum;  /* hotboot tracker */
        char     *following;
        char     *groupleader;
        sh_int Speed;
};

struct specfun_list
{
        SpecList *next;
        SpecList *prev;
        char     *name;
};

struct KilledData
{
        int vnum;
        char count;
};

struct temp_greet_ptr;

/*
 * Data which only PC's have.
 */
class PCData
{
        public: // public access for ease of use
        ClanData *clan;
        AreaData *area;
        struct wanted_data *first_wanted;
        struct wanted_data *last_wanted;
        char     *full_name;
        char     *homepage;
        char     *pwd;
        char     *spouse;
        CharData *propose;
        char     *bamfin;
        char     *bamfout;
        char     *rank;
        char     *title;
        char     *bestowments;  /* Special bestowed commands       */
        ExtBV flags;  /* Whether the player is deadly and whatever else we add. With additional bullshit fixing     */
        int pkills; /* Number of pkills on behalf of clan */
        int pdeaths;    /* Number of times pkilled (legally)  */
        int mkills; /* Number of mobs killed           */
        int mdeaths;    /* Number of deaths due to mobs       */
        int illegal_pk; /* Number of illegal pk's committed   */
        long int restore_time;  /* The last time the char did a restore all */
        int r_range_lo; /* room range */
        int r_range_hi;
        int m_range_lo; /* mob range  */
        int m_range_hi;
        int o_range_lo; /* obj range  */
        int o_range_hi;
        sh_int wizinvis;    /* wizinvis level */
        sh_int min_snoop;   /* minimum snoop level */
        sh_int condition[MaxConds];
        sh_int learned[MaxSkill];
        KilledData killed[MaxKillTrack];
        sh_int quest_number;    /* current *QUEST BEING DONE* DON'T REMOVE! */
        sh_int quest_curr;  /* current number of quest points */
        int quest_accum;    /* quest points accumulated in players life */
        int AuthState;
        time_t release_date;    /* Auto-helling.. Altrag */
        char     *helled_by;
        char     *bio;  /* Personal Bio */
        char     *authed_by;    /* what crazy imm authed this name ;) */
        char     *prompt;   /* User config prompts */
        char     *fprompt;  /* User config fighting prompts */
        char     *subprompt;    /* Substate prompt */
        sh_int pagerlen;    /* For pager (NOT menus) */
        sh_int addiction[10];
        sh_int drug_level[10];
        long bank;
        int clanrank;   /* current clan rank */
        int recall;
        int rp; /* rp points */
        int illness;    /* illness value */
        int realage;    /* rp points */
        sh_int arousal; /* arousal points */
        int weight; /* temper points */
        int height; /* temper points */
        int eye;    /* temper points */
        int hair;   /* temper points */
        int build;  /* temper points */
        int complextion;    /* temper points */
        char     *ignore[MaxIgnore];
        char     *sendmail; /*buffer used to send an email */
        bool hotboot;   /* hotboot tracker */
        int comchan;    /* for talk tune */
        int implants[MaxImplantTypes];
        sh_int statpoints;
        sh_int statedit;
#ifdef ACCOUNT
        struct AccountData *Account;
#endif
        TimeInfoData birthday;
        sh_int age;
        int godflags;
        char     *pose;
        char     *listening;
#ifdef IMC
        ImcChardata *imcchardata;
#endif
		  char     *email;
        char     *realname;
        char     *icq;
        char     *msn;
        char     *aolim;
        char     *yahoo;

        temp_greet_ptr * greet_info;

};



/*
 * Liquids.
 */
#define LiqWater        0
#define LiqMax		19

struct liq_type
{
        const char     *liq_name;
        const char     *liq_color;
        sh_int liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
        ExtraDescrData *next; /* Next in list                     */
        ExtraDescrData *prev; /* Previous in list                 */
        char     *keyword;  /* Keyword in look/examine          */
        char     *description;  /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct obj_index_data
{
        ObjIndexData *next;
        ObjIndexData *next_sort;
        ExtraDescrData *first_extradesc;
        ExtraDescrData *last_extradesc;
        AffectData *first_affect;
        AffectData *last_affect;
        MProgData *mudprogs;   /* objprogs */
        int progtypes;  /* objprogs */
        char     *name;
        char     *short_descr;
        char     *description;
        char     *action_desc;
        int vnum;
        sh_int level;
        sh_int item_type;
        int extra_flags;
        int magic_flags;    /*Need more bitvectors for spells - Scryn */
        int wear_flags;
        sh_int count;
        sh_int weight;
        int cost;
        int value[6];
        sh_int layers;
        int rent;   /* Unused */
};


/*
 * One object.
 */
struct ObjData
{
        ObjData *next;
        ObjData *prev;
        ObjData *next_content;
        ObjData *prev_content;
        ObjData *first_content;
        ObjData *last_content;
        ObjData *in_obj;
        CharData *carried_by;
        ExtraDescrData *first_extradesc;
        ExtraDescrData *last_extradesc;
        AffectData *first_affect;
        AffectData *last_affect;
        ObjIndexData *pIndexData;
        RoomIndexData *in_room;
        char     *armed_by;
        char     *name;
        char     *short_descr;
        char     *description;
        char     *action_desc;
        sh_int item_type;
        sh_int mpscriptpos;
        int extra_flags;
        int magic_flags;    /*Need more bitvectors for spells - Scryn */
        int wear_flags;
        int blaster_setting;
        MProgActList *mpact;  /* mudprogs */
        int mpactnum;   /* mudprogs */
        sh_int wear_loc;
        sh_int weight;
        int cost;
        sh_int level;
        sh_int timer;
        int value[6];
        sh_int count;   /* support for object grouping */
        int RoomVnum;  /* hotboot tracker */
};


/*
 * Exit data.
 */
struct ExitData
{
        ExitData *prev;    /* previous exit in linked list */
        ExitData *next;    /* next exit in linked list */
        ExitData *rexit;   /* Reverse exit pointer     */
        RoomIndexData *to_room;   /* Pointer to destination room  */
        char     *keyword;  /* Keywords for exit or door    */
        char     *description;  /* Description of exit      */
        int vnum;   /* Vnum of room exit leads to   */
        int rvnum;  /* Vnum of room in opposite dir */
        int exit_info;  /* door states & other flags    */
        int key;    /* Key vnum         */
        sh_int vdir;    /* Physical "direction"     */
        sh_int distance;    /* how far to the next room */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data
{
        ResetData *next;
        ResetData *prev;
        char command;
        int extra;
        int arg1;
        int arg2;
        int arg3;
};

/* Constants for arg2 of 'B' resets. */
constexpr int BitResetDoor		= 0;
constexpr int BitResetObject		= 1;
constexpr int BitResetMobile		= 2;
constexpr int BitResetRoom		= 3;
constexpr int BitResetTypeMask	= 0xFF;    /* 256 should be enough */
constexpr int BitResetDoorThreshold	= 8;
constexpr int BitResetDoorMask	= 0xFF00;  /* 256 should be enough */
constexpr int BitResetSet		= 30;
constexpr int BitResetToggle		= 31;
constexpr int BitResetFreebits        = 0x3FFF0000;    /* For reference */



/*
 * Area definition.
 */
#define AreaVersion 	2
struct area_data
{
        int version;
        AreaData *next;
        AreaData *prev;
        AreaData *next_sort;
        AreaData *prev_sort;
        ResetData *first_reset;
        ResetData *last_reset;
        PlanetData *planet;
        BodyData *body;
        AreaData *next_on_planet;
        AreaData *prev_on_planet;
        AreaData *next_on_body;
        AreaData *prev_on_body;
        /*
         * Person vs Player to be consistant 
         */
        CharData *first_person;
        CharData *last_person;
        char     *name;
        char     *filename;
        int flags;
        sh_int status;  /* h, 8/11 */
        sh_int age;
        sh_int nplayer;
        sh_int reset_frequency;
        int low_r_vnum;
        int hi_r_vnum;
        int low_o_vnum;
        int hi_o_vnum;
        int low_m_vnum;
        int hi_m_vnum;
        int low_soft_range;
        int hi_soft_range;
        int low_hard_range;
        int hi_hard_range;
        char     *author;   /* Scryn */
        char     *resetmsg; /* Rennard */
        ResetData *last_mob_reset;
        ResetData *last_obj_reset;
        sh_int max_players;
        int mkills;
        int mdeaths;
        int pkills;
        int pdeaths;
        int gold_looted;
        int illegal_pk;
        int high_economy;
        int low_economy;
};



/*
 * Load in the gods building data. -- Altrag
 */
struct godlist_data
{
        GodData *next;
        GodData *prev;
        int level;
        int low_r_vnum;
        int hi_r_vnum;
        int low_o_vnum;
        int hi_o_vnum;
        int low_m_vnum;
        int hi_m_vnum;
};


/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct system_data
{
        int maxplayers; /* Maximum players this boot   */
        int alltimemax; /* Maximum players ever   */
        char     *time_of_max;  /* Time of max ever */
        bool NoNameResolving; /* Hostnames are not resolved  */
        bool DenyNewPlayers;  /* New players cannot connect  */
        bool WaitForAuth; /* New players must be auth'ed */
        sh_int read_all_mail;   /* Read all player mail(was 54) */
        sh_int read_mail_free;  /* Read mail for free (was 51) */
        sh_int write_mail_free; /* Write mail for free(was 51) */
        sh_int take_others_mail;    /* Take others mail (was 54)   */
        sh_int muse_level;  /* Level of muse channel */
        sh_int think_level; /* Level of think channel LevelHiGod */
        sh_int build_level; /* Level of build channel LevelBuild */
        sh_int log_level;   /* Level of log channel LEVEL LOG */
        sh_int level_modify_proto;  /* Level to modify prototype stuff LevelLesser */
        sh_int level_override_private;  /* override private flag */
        sh_int level_mset_player;   /* Level to mset a player */
        sh_int stun_plr_vs_plr; /* Stun Mod player vs. player */
        sh_int stun_regular;    /* Stun difficult */
        sh_int dam_plr_vs_plr;  /* Damage Mod player vs. player */
        sh_int dam_plr_vs_mob;  /* Damage Mod player vs. mobile */
        sh_int dam_mob_vs_plr;  /* Damage Mod mobile vs. player */
        sh_int dam_mob_vs_mob;  /* Damage Mod mobile vs. mobile */
        sh_int level_getobjnotake;  /* Get objects without take flag */
        sh_int level_forcepc;   /* The level at which you can use Force on players. */
        sh_int max_sn;  /* Max skills */
        int save_flags; /* Toggles for saving conditions */
        sh_int save_frequency;  /* How old to autosave someone */
        void     *dlHandle;
        sh_int newbie_purge;    /* Level to auto-purge newbies at - Samson 12-27-98 */
        sh_int regular_purge;   /* Level to purge normal players at - Samson 12-27-98 */
        bool CLEANPFILES;   /* Should the mud clean up pfiles daily? - Samson 12-27-98 */
        bool PORT;  /* Should the mud clean up pfiles daily? - Greven 9-13-03 */
        int log_size;   /*Added to control size of log files. */
        char     *mud_name; /* Name fo mud for the webserver, maybe more - Greven 11-20-03 */
        char     *mud_url;  /* Mud Websites URL. ie, http://darkwars.wolfpaw.net/ - Gavin 12-17-03 */
        char     *mud_email;    /* Mud email address. ie, darkwarsmud@hotmail.com - Gavin 12-22-03 */
        char     *mail_path;    /* Path to sendmail type program */
        sh_int ban_site_level;  /* Level to ban sites - Gavin 12-21-03 */
        sh_int ban_class_level; /* Level to ban classes - Gavin 12-21-03 */
        sh_int ban_race_level;  /* Level to ban races - Gavin 12-21-03 */
        sh_int channellog;  /* Amount of items to save into ooc log Green 09/03/04 */
        bool web;   /* Is the web server on? 20/10/04 */

        int DEBUG; /* Cset to toggle backtrace and other debugging information */
        int GREET; /* Toggle Greet System - 2005-11-26 - Gavin */


        /* Pulses */
        int pulse_taxes;
        int pulse_area;
        int pulse_mobile;
        int pulse_violence;
        int pulse_point;
        int pulse_second;
        int pulse_space;
        int pulse_ship;
        int pulse_recharge;
};


struct HomeData;
/*
 * Room type.
 */
// Modernized: RoomIndexData (formerly room_index_data)
class RoomIndexData {
public:
        RoomIndexData *next;
        RoomIndexData *next_sort;
        CharData *first_person;
        CharData *last_person;
        ObjData *first_content;
        ObjData *last_content;
        ExtraDescrData *first_extradesc;
        ExtraDescrData *last_extradesc;
        AreaData *area;
        ExitData *first_exit;
        ExitData *last_exit;
        ShipData *first_ship;
        ShipData *last_ship;
#ifdef OlcShuttle
        struct shuttle_data *first_shuttle;
        struct shuttle_data *last_shuttle;
#endif
        char     *name;
        char     *description;
        int vnum;
        ExtBV RoomFlags;
        MProgActList *mpact;  /* mudprogs */
        int mpactnum;   /* mudprogs */
        MProgData *mudprogs;   /* mudprogs */
        sh_int mpscriptpos;
        int progtypes;  /* mudprogs */
        sh_int light;
        sh_int sector_type;
        sh_int tunnel;  /* max people that will fit */
        /* Eventually change this to a union or struct pointer for other types
         * such as room->isa->ship
         */
        HomeData * home;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TypeUndefined                  -1
#define TypeHit                      1000   /* allows for 1000 skills/spells */
#define TypeHerb		     2000   /* allows for 1000 attack types  */
#define TypePersonal		     3000   /* allows for 1000 herb types    */

/*
 *  Target types.
 */
typedef enum
{
        TarIgnore, TarCharOffensive, TarCharDefensive, TarCharSelf,
        TarObjInv
} target_types;

typedef enum
{
        SkillUnknown, SkillSpell, SkillSkill, SkillWeapon, SkillTongue,
        SkillHerb
} skill_types;



struct timerset
{
        int num_uses;
        struct timeval total_time;
        struct timeval min_time;
        struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
        char     *name; /* Name of skill        */
        SpellFun *spell_fun;   /* Spell pointer (for spells)   */
        char     *spell_fun_name;   /* Spell function name - Trax */
        DoFun   *skill_fun;    /* Skill pointer (for skills)   */
        char     *skill_fun_name;   /* Skill function name - Trax */
        sh_int target;  /* Legal targets        */
        sh_int minimum_position;    /* Position for caster / user   */
        sh_int slot;    /* Slot for #OBJECT loading */
        sh_int min_endurance;   /* Minimum endurance used       */
        sh_int beats;   /* Rounds required to use skill */
        char     *noun_damage;  /* Damage message       */
        char     *msg_off;  /* Wear off message     */
        sh_int guild;   /* Which guild the skill belongs to */
        sh_int min_level;   /* Minimum level to be able to cast */
        sh_int type;    /* Spell/Skill/Weapon/Tongue    */
        int flags;  /* extra stuff          */
        char     *hit_char; /* Success message to caster    */
        char     *hit_vict; /* Success message to victim    */
        char     *hit_room; /* Success message to room  */
        char     *miss_char;    /* Failure message to caster    */
        char     *miss_vict;    /* Failure message to victim    */
        char     *miss_room;    /* Failure message to room  */
        char     *die_char; /* Victim death msg to caster   */
        char     *die_vict; /* Victim death msg to victim   */
        char     *die_room; /* Victim death msg to room */
        char     *imm_char; /* Victim immune msg to caster  */
        char     *imm_vict; /* Victim immune msg to victim  */
        char     *imm_room; /* Victim immune msg to room    */
        char     *dice; /* Dice roll            */
        int value;  /* Misc value           */
        char saves; /* What saving spell applies    */
        char difficulty;    /* Difficulty of casting/learning */
        SmaugAff *affects; /* Spell affects, if any    */
        char     *components;   /* Spell components, if any */
        char     *teachers; /* Skill requires a special teacher */
        char     *races;    /* Races that can use this skill */
        char participants;  /* # of required participants   */
        struct timerset userec; /* Usage record         */
        int alignment;  /* for jedi powers */
        ExtBV body_parts;
        bool held;
};


struct AuctionData
{
        ObjData *item; /* a pointer to the item */
        CharData *seller;  /* a pointer to the seller - which may NOT quit */
        CharData *buyer;   /* a pointer to the buyer - which may NOT quit */
        int bet;    /* last bet - or 0 if noone has bet anything */
        sh_int going;   /* 1,2, sold */
        sh_int pulse;   /* how many pulses (.25 sec) until another call-out ? */
        int starting;
};


/*
 * These are skill_lookup return values for common skills and spells.
 */

/* Piloting */
extern sh_int gsn_starfighters;
extern sh_int gsn_midships;
extern sh_int gsn_capitalships;
extern sh_int gsn_weaponsystems;
extern sh_int gsn_navigation;
extern sh_int gsn_shipsystems;
extern sh_int gsn_tractorbeams;
extern sh_int gsn_shipmaintenance;
extern sh_int gsn_spacecombat;
extern sh_int gsn_spacecombat2;
extern sh_int gsn_spacecombat3;
extern sh_int gsn_bomb;
extern sh_int gsn_boardship;

extern sh_int gsn_recruit;
extern sh_int gsn_juke;
extern sh_int gsn_extrapolate;
extern sh_int gsn_evade;
extern sh_int gsn_roll;
extern sh_int gsn_blockdoor;
extern sh_int gsn_modifyexit;
extern sh_int gsn_reinforcements;
extern sh_int gsn_postguard;

extern sh_int gsn_add_patrol;
extern sh_int gsn_elite_guard;
extern sh_int gsn_special_forces;
extern sh_int gsn_jail;
extern sh_int gsn_smalltalk;
extern sh_int gsn_propeganda;
extern sh_int gsn_bribe;
extern sh_int gsn_seduce;
extern sh_int gsn_masspropeganda;
extern sh_int gsn_gather_intelligence;

extern sh_int gsn_torture;
extern sh_int gsn_snipe;
extern sh_int gsn_throwsaber;
extern sh_int gsn_throw;
extern sh_int gsn_disguise;
extern sh_int gsn_changesex;
extern sh_int gsn_mine;
extern sh_int gsn_grenades;
extern sh_int gsn_contraband;

extern sh_int gsn_beg;
extern sh_int gsn_makeblade;
extern sh_int gsn_implant;
extern sh_int gsn_makeimplant;
extern sh_int gsn_shipdesign;
extern sh_int gsn_dismantle_ship;
extern sh_int gsn_makejewelry;
extern sh_int gsn_makeblaster;
extern sh_int gsn_makelight;
extern sh_int gsn_makefurniture;
extern sh_int gsn_makebinding;
extern sh_int gsn_modifyship;
extern sh_int gsn_makebowcaster;
extern sh_int gsn_makecomlink;
extern sh_int gsn_makegrenade;
extern sh_int gsn_makelandmine;
extern sh_int gsn_makearmor;
extern sh_int gsn_makeshield;
extern sh_int gsn_makepike;
extern sh_int gsn_makeknife;
extern sh_int gsn_makegoggles;
extern sh_int gsn_makecontainer;
extern sh_int gsn_makebeacon;
extern sh_int gsn_gemcutting;
extern sh_int gsn_lightsaber_crafting;
extern sh_int gsn_spice_refining;
extern sh_int gsn_identify;
extern sh_int gsn_makebase;
extern sh_int gsn_roomconstruction;

extern sh_int gsn_detrap;
extern sh_int gsn_backstab;
extern sh_int gsn_circle;
extern sh_int gsn_dodge;
extern sh_int gsn_hide;
extern sh_int gsn_peek;
extern sh_int gsn_slight;
extern sh_int gsn_pick_lock;
extern sh_int gsn_scan;
extern sh_int gsn_sneak;
extern sh_int gsn_steal;
extern sh_int gsn_gouge;
extern sh_int gsn_track;
extern sh_int gsn_search;
extern sh_int gsn_dig;
extern sh_int gsn_mount;
extern sh_int gsn_bashdoor;
extern sh_int gsn_berserk;
extern sh_int gsn_hitall;
extern sh_int gsn_pickshiplock;
extern sh_int gsn_hijack;
extern sh_int gsn_break;
extern sh_int gsn_imprison;

extern sh_int gsn_craftpike;
extern sh_int gsn_craftknife;
extern sh_int gsn_longcall;
extern sh_int gsn_cuteness;
extern sh_int gsn_trophy;
extern sh_int gsn_jimmyshiplock;
extern sh_int gsn_systemmaintenance;
extern sh_int gsn_hydrolic_strength;
extern sh_int gsn_camouflage;
extern sh_int gsn_hone;
extern sh_int gsn_gowithout;
extern sh_int gsn_gamorrean_strength;
extern sh_int gsn_jab;
extern sh_int gsn_savant_mid_ships;
extern sh_int gsn_savant_capital_ships;
extern sh_int gsn_coerce;
extern sh_int gsn_repair;
extern sh_int gsn_cajole;
extern sh_int gsn_pretend;
extern sh_int gsn_placebeacon;
extern sh_int gsn_fleet_command1;

extern sh_int gsn_disarm;
extern sh_int gsn_enhanced_damage;
extern sh_int gsn_kick;
extern sh_int gsn_trip;
extern sh_int gsn_parry;
extern sh_int gsn_rescue;
extern sh_int gsn_second_attack;
extern sh_int gsn_third_attack;
extern sh_int gsn_dual_wield;


extern sh_int gsn_aid;

/* used to do specific lookups */
extern sh_int gsn_first_spell;
extern sh_int gsn_first_skill;
extern sh_int gsn_first_weapon;
extern sh_int gsn_first_tongue;
extern sh_int gsn_top_sn;

/* spells */
extern sh_int gsn_blindness;
extern sh_int gsn_potential;
extern sh_int gsn_force_healing;
extern sh_int gsn_sense_force;
extern sh_int gsn_charm_person;
extern sh_int gsn_aqua_breath;
extern sh_int gsn_invis;
extern sh_int gsn_mass_invis;
extern sh_int gsn_poison;
extern sh_int gsn_sleep;
extern sh_int gsn_possess;
extern sh_int gsn_fireball; /* for fireshield  */
extern sh_int gsn_lightning_bolt;   /* for shockshield */

/* newer attack skills */
extern sh_int gsn_punch;
extern sh_int gsn_bash;
extern sh_int gsn_stun;

extern sh_int gsn_poison_weapon;
extern sh_int gsn_climb;

extern sh_int gsn_blasters;
extern sh_int gsn_force_pikes;
extern sh_int gsn_addpersonel;
extern sh_int gsn_bowcasters;
extern sh_int gsn_sharpen;
extern sh_int gsn_lightsabers;
extern sh_int gsn_vibro_blades;
extern sh_int gsn_knives;
extern sh_int gsn_lockdoor;
extern sh_int gsn_makekey;
extern sh_int gsn_flexible_arms;
extern sh_int gsn_talonous_arms;
extern sh_int gsn_bludgeons;
extern sh_int gsn_sabotage;
extern sh_int gsn_meditate;
extern sh_int gsn_claw;
extern sh_int gsn_bite;
extern sh_int gsn_sting;
extern sh_int gsn_tail;

extern sh_int gsn_grip;

/* languages */
extern sh_int gsn_basic;
extern sh_int gsn_wookiee;
extern sh_int gsn_twilek;
extern sh_int gsn_rodian;
extern sh_int gsn_hutt;
extern sh_int gsn_bothan;
extern sh_int gsn_kel_dor;
extern sh_int gsn_mon_calamari;
extern sh_int gsn_noghri;
extern sh_int gsn_csillian;
extern sh_int gsn_gamorrean;
extern sh_int gsn_jawa;
extern sh_int gsn_adarian;
extern sh_int gsn_ewok;
extern sh_int gsn_verpine;
extern sh_int gsn_defel;
extern sh_int gsn_trandoshan;
extern sh_int gsn_shistavanan;
extern sh_int gsn_binary;
extern sh_int gsn_duinduogwuin;
extern sh_int gsn_barabel;
extern sh_int gsn_ithorian;
extern sh_int gsn_devaronian;
extern sh_int gsn_durosian;
extern sh_int gsn_gotal;
extern sh_int gsn_talz;
extern sh_int gsn_ho_din;
extern sh_int gsn_falleen;
extern sh_int gsn_givin;

/* Occupations */
extern sh_int gsn_dishwasher;
extern sh_int gsn_busser;
extern sh_int gsn_cook;
extern sh_int gsn_waiter;
extern sh_int gsn_chef;
extern sh_int gsn_bartender;
extern sh_int gsn_streetcleaner;
extern sh_int gsn_interiorcleaner;
extern sh_int gsn_hotelcleaner;
extern sh_int gsn_secretary;
extern sh_int gsn_clerk;
extern sh_int gsn_commmarketer;
extern sh_int gsn_marketer;
extern sh_int gsn_solicitor;
extern sh_int gsn_advertiser;
extern sh_int gsn_banker;
extern sh_int gsn_realitor;
extern sh_int gsn_accountant;
extern sh_int gsn_investor;
extern sh_int gsn_broker;
extern sh_int gsn_boardmember;
extern sh_int gsn_ceo;
extern sh_int gsn_youmom;

/* Pirate */
extern sh_int gsn_scan1;
extern sh_int gsn_scan2;
extern sh_int gsn_scan3;
extern sh_int gsn_jam;
extern sh_int gsn_transfercargo;
extern sh_int gsn_dock;
extern sh_int gsn_boardship;
extern sh_int gsn_cloak;
extern sh_int gsn_stealth;
extern sh_int gsn_disrupt;
extern sh_int gsn_causedesertion;
extern sh_int gsn_causedissension;
extern sh_int gsn_boostmorale;

/* Medical */
extern sh_int gsn_splint;
extern sh_int gsn_first_aid;
extern sh_int gsn_autopsy;
extern sh_int gsn_diagnose;
extern sh_int gsn_makemedkit;

#include "utils.hpp"

/*
 * Macros for accessing virtually unlimited bitvectors.		-Thoric
 *
 * Note that these macros use the bit number rather than the bit value
 * itself -- which means that you can only access _one_ bit at a time
 *
 * This code uses an array of integers
 */

bool ext_is_empty args((ExtBV * bits));
void ext_clear_bits args((ExtBV * bits));
int ext_has_bits args((ExtBV * var, ExtBV * bits));
bool ext_same_bits args((ExtBV * var, ExtBV * bits));
void ext_set_bits args((ExtBV * var, ExtBV * bits));
void ext_remove_bits args((ExtBV * var, ExtBV * bits));
void ext_toggle_bits args((ExtBV * var, ExtBV * bits));
ExtBV fread_bitvector args((FILE * fp));
char     *print_bitvector args((ExtBV * bits));
void fwrite_bitvector args((ExtBV * bits, FILE * fp));
ExtBV meb args((int bit));
ExtBV multimeb args((int bit, ...));
char     *ext_flag_string args((ExtBV * bitvector, const char *const flagarray[]));
char     *show_ext_flag_string args((int len, const char *const flagarray[]));

/*
 * Here are the extended bitvector macros:
 */
// Replace your old macros with std::bitset versions:
#define IsSet(var, bit)         ((var).test(bit)) // Changed to use std::bitset
#define SetBit(var, bit)        ((var).set(bit)) // Changed to use std::bitset
#define RemoveBit(var, bit)     ((var).reset(bit)) // Changed to use std::bitset
#define ToggleBit(var, bit)     ((var).flip(bit)) // Changed to use std::bitset
#define ClearBits(var)          ((var).reset()) // Changed to use std::bitset
#define IsEmpty(var)            ((var).none()) // Changed to use std::bitset

// For the more complex ones, you might need helper functions:
#define SetBits(var, bit)       ((var) |= (bit)) // Assuming bit is also a std::bitset
#define RemoveBits(var, bit)    ((var) &= ~(bit)) // Assuming bit is also a std::bitset
#define HasBits(var, bit)       (((var) & (bit)) == (bit)) // Assuming bit is also a std::bitset
#define SameBits(var, bit)      ((var) == (bit)) // Assuming bit is also a std::bitset
#define __STRING(x) #x // Stringify macro
#define meb(bit)                (ExtBV().set(bit)) // Changed to use std::bitset
#define multimeb(...)           ([](int bits...) { ExtBV bv; va_list args; va_start(args, bits); for (int b = bits; b != -1; b = va_arg(args, int)) bv.set(b); va_end(args); return bv; }(__VA_ARGS__, -1)) // Changed to use std::bitset
#define fread_bitvector(fp)     fread_bitvector(fp) // Fix macro definition
#define fwrite_bitvector(var, fp) fwrite_bitvector(var, fp) // Fix macro definition
#define print_bitvector(var)      print_bitvector(var) // Fix macro definition
#define ext_flag_string(var, bit) show_ext_flag_string(sizeof(var) * 8, bit) // Fix macro definition
#define ext_has_bits(var, bit)  HasBits(var, bit) // Fix macro definition
#define ext_set_bits(var, bit)  SetBits(var, bit) // Fix macro definition
#define ext_remove_bits(var, bit) RemoveBits(var, bit) // Fix macro definition
#define ext_toggle_bits(var, bit) ((var) ^= (bit)) // Fix macro definition
#define ext_is_empty(var)       IsEmpty(var) // Fix macro definition
#define ext_clear_bits(var)     ClearBits(var) // Fix macro definition
#define ext_same_bits(var, bit) SameBits(var, bit) // Fix macro definition

/*
 * Character macros.
 */

#define IsNpc(ch)              (IsSet((ch)->pIndexData->act, ActIsNpc) || (ch)->PCData == NULL)
#define IsQuestor(ch)          (IsSet((ch)->pIndexData->act, PlrQuestor))
#define IsImmortal(ch)         (get_trust((ch)) >= LevelImmortal)
#define IsHero(ch)             (get_trust((ch)) >= LevelHero)
#define IsPlaying(d)           ((d)->connected == ConPlaying || \
                                                           (d)->connected == ConForked || (d)->connected == ConIaForked )
#define IsAffected(ch, sn)     (IsSet((ch)->affected_by, (sn)))
#define HasBodyPart(ch, part)  ((ch)->xflags == 0 || IsSet((ch)->xflags, (part)))

#define IsGood(ch)             ((ch)->alignment >= 350)
#define IsEvil(ch)             ((ch)->alignment <= -350)
#define IsNeutral(ch)          (!IsGood(ch) && !IsEvil(ch))

#define IsAwake(ch)            ((ch)->position > PosSleeping || IsAffected( (ch), AffCharm ))
#define GetAc(ch)              ( (ch)->armor + ( IsAwake(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) \
                                                           - ( !str_cmp((ch)->race->name(), "defel") ? (ch)->skill_level[COMBAT_ABILITY]*2+5 : (ch)->skill_level[COMBAT_ABILITY]/2 ) )
#define GetHitroll(ch)         ((ch)->hitroll \
                                                           +str_app[get_curr_str(ch)].tohit \
                                                           +(2-(abs((ch)->mental_state)/10)))
#define GetDamroll(ch)         ((ch)->damroll \
                                                           +str_app[get_curr_str(ch)].todam \
                                                           +(((ch)->mental_state > 5 \
                                                           &&(ch)->mental_state < 15) ? 1 : 0) )

#define IsOutside(ch)          (IsOutsideRoom((ch)->in_room))
        /*
         * (!IsSet(                   \
         * (ch)->in_room->room_flags,           \
         * ROOM_INDOORS) && !IsSet(               \
         * (ch)->in_room->room_flags,              \
         * ROOM_SPACECRAFT) )
         */

#define IsOutsideRoom(room)    (!IsSet((room)->room_flags, \
                                                           ROOM_INDOORS) && !IsSet( \
                                                           (room)->room_flags, \
                                                           ROOM_SPACECRAFT) )
#define IsDrunk(ch, drunk)     (number_percent() < \
                                                           ( (ch)->PCData->condition[COND_DRUNK] \
                                                           * 2 / (drunk) ) )

#define IsClanned(ch)          (!IsNpc((ch)) \
                                                           && (ch)->PCData->clan )

#define WaitState(ch, npulse)  ((ch)->wait = UMax((ch)->wait, (IsImmortal(ch) ? 0 :(npulse))))


#define Exit(ch, door)         ( get_exit( (ch)->in_room, door ) )

#define CanGo(ch, door)        (Exit((ch),(door)) \
                                                           && (Exit((ch),(door))->to_room != NULL)  \
                                                           && !IsSet(Exit((ch), (door))->exit_info, EX_CLOSED))

#define IsValidSn(sn)          ( (sn) >=0 && (sn) < MAX_SKILL \
                                                           && skill_table[(sn)] \
                                                           && skill_table[(sn)]->name )

#define IsValidHerb(sn)        ( (sn) >=0 && (sn) < MAX_HERB \
                                                           && herb_table[(sn)] \
                                                           && herb_table[(sn)]->name )


#define DefImmFlags                (ImmAll | ImmOwner)
#define ImmHighBuilder             ImmHighBuilder
#define ImmHighEnforcer            ImmHighEnforcer
#define ImmEnforcer                ImmEnforcer
#define ImmAdmin                   ImmAdmin
#define ImmCoder                   ImmCoder
#define ImmQuest                   ImmQuest
#define IsImmBuilder(ch)           ( !IsNpc((ch)) && (ch)->PCData->godflags & ( ImmBuilder | ImmHighBuilder | DefImmFlags ) )
#define IsImmHighEnforcer(ch)      ( !IsNpc((ch)) && (ch)->PCData->godflags & ( ImmHighEnforcer | DefImmFlags ) )
#define IsImmEnforcer(ch)          ( !IsNpc((ch)) && (ch)->PCData->godflags & ( ImmEnforcer | DefImmFlags ) )
#define IsImmAdmin(ch)             ( !IsNpc((ch)) && (ch)->PCData->godflags & ( ImmAdmin | DefImmFlags ) )
#define IsImmHighBuilder(ch)       ( !IsNpc((ch)) && (ch)->PCData->godflags & ( ImmHighBuilder | DefImmFlags ) )
#define IsImmCoder(ch)             ( !IsNpc((ch)) && (ch)->PCData->godflags & ( ImmCoder | DefImmFlags ) )
#define IsImmQuest(ch)             ( !IsNpc((ch)) && (ch)->PCData->godflags & ( ImmQuest | DefImmFlags ) )

#define SpellFlag(skill, flag)     ( IsSet((skill)->flags, (flag)) )
#define SpellDamage(skill)         ( ((skill)->flags     ) & 7 )
#define SpellAction(skill)         ( ((skill)->flags >> 3) & 7 )
#define SpellClass(skill)          ( ((skill)->flags >> 6) & 7 )
#define SpellPower(skill)          ( ((skill)->flags >> 9) & 3 )
#define SetSdam(skill, val)        ( (skill)->flags =  ((skill)->flags & SDAM_MASK) + ((val) & 7) )
#define SetSact(skill, val)        ( (skill)->flags =  ((skill)->flags & SACT_MASK) + (((val) & 7) << 3) )
#define SetScla(skill, val)        ( (skill)->flags =  ((skill)->flags & SCLA_MASK) + (((val) & 7) << 6) )
#define SetSpow(skill, val)        ( (skill)->flags =  ((skill)->flags & SPOW_MASK) + (((val) & 3) << 9) )

/* Retired and guest imms. */
#define IsRetired(ch)              ((ch)->PCData && IsSet((ch)->PCData->flags,PcFlagRetired))
#define IsGuest(ch)                ((ch)->PCData && IsSet((ch)->PCData->flags,PcFlagGuest))

/* RIS by gsn lookups. -- Altrag. */
#define IsFire(dt)                 ( IsValidSn(dt) && SpellDamage(skill_table[(dt)]) == SdFire )
#define IsCold(dt)                 ( IsValidSn(dt) && SpellDamage(skill_table[(dt)]) == SdCold )
#define IsAcid(dt)                 ( IsValidSn(dt) && SpellDamage(skill_table[(dt)]) == SdAcid )
#define IsElectricity(dt)          ( IsValidSn(dt) && SpellDamage(skill_table[(dt)]) == SdElectricity )
#define IsEnergy(dt)               ( IsValidSn(dt) && SpellDamage(skill_table[(dt)]) == SdEnergy )
#define IsDrain(dt)                ( IsValidSn(dt) && SpellDamage(skill_table[(dt)]) == SdDrain )
#define IsPoison(dt)               ( IsValidSn(dt) && SpellDamage(skill_table[(dt)]) == SdPoison )

#define NotAuthed(ch)              (!IsNpc(ch) && (ch)->PCData->AuthState <= 3  \
                                   && IsSet((ch)->PCData->flags, PcflagUnauthed) )

#define IsWaitingForAuth(ch)       (!IsNpc(ch) && (ch)->desc \
                                   && (ch)->PCData->AuthState == 1 \
                                   && IsSet((ch)->PCData->flags, PcflagUnauthed) )

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	    \
				{					                    \
				    field  = value;			            \
				    fMatch = TRUE;			            \
				    break;				                \
				}



/*
 * Object macros.
 */

#define CanWear(obj, part)        (IsSet((obj)->wear_flags,  (part)))
#define IsObjStat(obj, stat)      (IsSet((obj)->extra_flags, (stat)))



/*
 * Description macros.
 */
#define Pers(ch, looker)          ( can_see( (looker), (ch) ) ? \
                                  ( IsNpc(ch) ? (ch)->short_descr \
                                  : (ch)->PCData->full_name ) : (IsImmortal(ch) ? "An Immortal" : "someone") )



#define log_string( txt )	( log_string_plus( (txt), LogNormal, LevelLog ) )


#define CmdOoc			= 0;
#define CmdHeld		= 1;
#define CmdWatch               = 2;
#define CmdFullname            = 3;
/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
        CMDType  *next;
        char     *name;
        DoFun   *do_fun;
        char     *fun_name;
        int flags;
        sh_int position;
        sh_int level;
        sh_int log;
        struct timerset userec;
/*        sh_int held;
        sh_int ooc;*/
        int perm_flags;
};



/*
 * Structure for a social in the socials table.
 */
struct social_type
{
        SocialType *next;
        char     *name;
        char     *char_no_arg;
        char     *others_no_arg;
        char     *char_found;
        char     *others_found;
        char     *vict_found;
        char     *char_auto;
        char     *others_auto;
        sh_int arousal;
        sh_int minarousal;
        sh_int sex;
};



/*
 * Global constants.
 */
extern time_t last_restore_all_time;
extern time_t boot_time;    /* this should be moved down */
extern HourMinSec *set_boot_time;
extern struct tm *new_boot_time;
extern time_t new_boot_time_t;

extern const struct StrAppType StrApp[41];
extern const struct IntAppType IntApp[41];
extern const struct wis_app_type WisApp[41];
extern const struct dex_app_type DexApp[41];
extern const struct con_app_type ConApp[41];
extern const struct cha_app_type ChaApp[41];
extern const struct lck_app_type LckApp[41];
extern const struct frc_app_type frc_app[41];

extern const char* const wear_locs[];
extern const char* const ex_flags[];

extern const struct race_type race_table[];
extern const struct liq_type liq_table[LiqMax];
extern const char *const attack_table[13];
extern const char *const ability_name[MaxAbility];

extern const char *const skill_tname[];
extern sh_int const movement_loss[SectMax];
extern const char *const dir_name[];
extern const char* const where_name[];
extern const sh_int rev_dir[];
extern const int trap_door[];
extern const char* const r_flags[];
extern const char* const part_flags[];
extern const char* const w_flags[];
extern const char* const o_flags[];
extern const char* const a_flags[];
extern const char* const o_types[];
extern const char* const a_types[];
extern const char* const act_flags[];
extern const char *const PlanetFlags[];
extern const char *const weapon_table[13];
extern const char *const spice_table[];
extern const char* const plr_flags[];
extern const char* const pc_flags[];
extern const char* const trap_flags[];
extern const char* const ris_flags[];
extern const char* const trig_flags[];
extern const char* const part_flags[];
extern const char *const npc_race[];
extern const char* const defense_flags[];
extern const char* const attack_flags[];
extern const char* const area_flags[];
extern const char *const ShipFlags[];
extern const char *const cargo_names[ContrabandMax];
extern const char *const hair_list[];
extern const char *const eye_list[];
extern const char *const build_list[];
extern const char *const height_list[];
extern const char *const weight_list[];
extern const char *const complextion_list[];
extern const char *const illness_list[];
extern const char *const body_parts[MaxBodyParts];
extern const char *const npc_sex[SexMax];
extern const char *const npc_position[PosMax];
extern const char *const log_flag[];
extern const char *const true_false[];

extern int const lang_array[];
extern const char* const lang_names[];

/*
 * Global variables.
 */
extern int numobjsloaded;
extern int nummobsloaded;
extern int physicalobjects;
extern int num_descriptors;
extern struct system_data sysdata;
extern int top_sn;
extern int top_vroom;
extern int top_herb;

extern CMDType *command_hash[126];

extern SkillType *skill_table[MaxSkill];
extern SocialType *social_index[27];
/* Optional double-xp globals (safe defaults if feature not used) */
extern bool double_exp;
extern int global_exp_ticks;
extern sh_int display_ticks;

/* Optional command prototypes */
CMDF do_doublexp(CharData *ch, char *argument);
CMDF do_gpoint(CharData *ch, char *argument);
extern ch_ret global_retcode;
extern SkillType *herb_table[MaxHerb];

extern int cur_obj;
extern int cur_obj_serial;
extern bool cur_obj_extracted;
extern obj_ret global_objcode;

extern HelpData *first_help;
extern HelpData *last_help;
extern ShopData *first_shop;
extern ShopData *last_shop;
extern RepairData *first_repair;
extern RepairData *last_repair;
extern BankAccount *first_baccount;
extern BankAccount *last_baccount;
extern WatchData *first_watch;
extern WatchData *last_watch;
extern CharData *first_char;
extern CharData *last_char;
extern DescriptorData *first_descriptor;
extern DescriptorData *last_descriptor;
extern ObjData *first_object;
extern ObjData *last_object;
extern ClanData *first_clan;
extern ClanData *last_clan;
extern ShipData *first_ship;
extern ShipData *last_ship;
extern SpaceData *first_starsystem;
extern SpaceData *last_starsystem;
extern PlanetData *first_planet;
extern PlanetData *last_planet;
extern AreaData *first_area;
extern AreaData *last_area;
extern AreaData *first_build;
extern AreaData *last_build;
extern AreaData *first_asort;
extern AreaData *last_asort;
extern AreaData *first_bsort;
extern AreaData *last_bsort;
extern ObjData *extracted_obj_queue;
extern ExtractCharData *extracted_char_queue;
extern ObjData *save_equipment[MaxWear][MaxLayers];
extern CharData *quitting_char;
extern CharData *loading_char;
extern CharData *saving_char;
extern ObjData *all_obj;
extern char bug_buf[];
extern time_t current_time;
extern bool fLogAll;
extern FILE *fpReserve;
extern FILE *fpLOG;
extern char log_buf[];
extern TimeInfoData time_info;
extern WeatherData weather_info;
extern MPSleepData *first_mpwait;
extern MPSleepData *last_mpwait;
extern MPSleepData *current_mpwait;
extern AuctionData *auction;
extern struct act_prog_data *mob_act_list;
extern SpecList *first_specfun;
extern SpecList *last_specfun;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(apollo)
int atoi args((const char *string));
void     *calloc args((unsigned nelem, size_t size));
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(hpux)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(interactive)
#endif

#if	defined(linux)
#if !defined(__cplusplus)
char     *crypt args((const char *key, const char *salt));
#endif
#endif

#if	defined(MipsOs)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(NeXT)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(sequent)
char     *crypt args((const char *key, const char *salt));
int fclose args((FILE * stream));
int fprintf args((FILE * stream, const char *format, ...));
int fread args((void *ptr, int size, int n, FILE * stream));
int fseek args((FILE * stream, long offset, int ptrname));
void perror args((const char *s));
int ungetc args((int c, FILE * stream));
#endif

#if	defined(sun)
char     *crypt args((const char *key, const char *salt));
int fclose args((FILE * stream));
int fprintf args((FILE * stream, const char *format, ...));

/*
#if 	defined(SYSV)
*/
size_t fread args((void *ptr, size_t size, size_t n,
/*				FILE *stream ) );
#else
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
*/
                   int fseek args((FILE * stream, long offset, int ptrname));
                   void perror args((const char *s));
                   int ungetc args((int c, FILE * stream));
#endif
#if	defined(ultrix)
                   char *crypt args((const char *key, const char *salt));
#endif
/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif
/*
 * Data files used by the server.
 *
 * AreaList contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NullFile is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define WatchDir    "../watch/"    /* Imm watch files --Gorog */
#define PlayerDir	"../player/"    /* Player files         */
#define BackupDir	"../backup/"    /* Backup Player files      */
#define GodDir		"../gods/"  /* God Info Dir         */
#define BoardDir	"../boards/"    /* Board data dir       */
#define ClanDir	"../clans/" /* Clan data dir        */
#define ShipDir        "../space/"
#define SpaceDir       "../space/"
#define PlanetDir      "../planets/"
#define GuardDir       "../planets/"
#define AreaDir       "../areas/"
#define GuildDir       "../guilds/"    /* Guild data dir               */
#define BuildDir       "../building/"  /* Online building save dir     */
#define SystemDir	"../system/"    /* Main system files        */
#define LogDir	"../log/"   /* Main system files        */
#define ProgDir    "../mudprogs/"  /* MUDProg files     */
#define CorpseDir	"../corpses/"   /* Corpses          */
#define NullFile	"/dev/null" /* To reserve one stream    */
#define MailDir      "../mail/"
#define BugFile	SystemDir "bugs.txt"   /* For bug( ) */
#define BugsFile	SystemDir "reported_bugs.txt"  /* For 'bug' */
#define EmailFile              SystemDir "email.dat"
#define FILE_AreaList	"area.lst"  /* List of areas        */
#define WatchList      "watch.lst" /* List of watches                        */
#define BanList        "ban.lst"   /* List of bans                 */
#define ClanList	"clan.lst"  /* List of clans        */
#define ShipList       "ship.lst"
#define PlanetList      "planet.lst"
#define SpaceList      "space.lst"
#define DisintigrationList	"disintigration.lst"
#define GuildList      "guild.lst" /* List of guilds               */
#define GodList	"gods.lst"  /* List of gods         */
#define GuardList	"guard.lst"
#define PlanetHtmlList	"planets.html"
#define ClanHtmlList	"clans.html"
#define ShipHtmlList	"ships.html"
#define CopyoverFile	SystemDir "copyover.dat"   /* for warm reboots    */
#define ExeFile	"../src/swr"    /*  Executable Path */
#define MainExeFile	"/home/mud/darkwars/dwadmins/src/swr"   /*  Executable Path */
#define PortExeFile	"../port/src/swr"   /*  Executable Path */
#define PshipArea            "pships.are"
#define BoardFile	"boards.txt"    /* For bulletin boards   */
#define ShutdownFile	"../log/shutdown/shutdown.txt"  /* For 'shutdown'    */
#define BootlogFile	SystemDir "boot.txt"   /* Boot up error file  */
#define IdeaFile	SystemDir "ideas.txt"  /* For 'idea'      */
#define MemoryFile	SystemDir "memory.txt" /* For 'wiznet'        */
#define TypoFile	SystemDir "typos.txt"  /* For 'typo'      */
#define PlanetsHtmlFile	SystemDir "planets.html"   /* For beginning html test         */
#define LogFile	SystemDir "log.txt"    /* For talking in logged rooms */
#define WizlistFile	SystemDir "WIZLIST"    /* Wizlist         */
#define SkillFile	SystemDir "skills.dat" /* Skill table     */
#define HerbFile	SystemDir "herbs.dat"  /* Herb table      */
#define SocialFile	SystemDir "socials.dat"    /* Socials         */
#define CommandFile	SystemDir "commands.dat"   /* Commands        */
#define UsageFile	SystemDir "usage.txt"  /* How many people are on 
                                             * every half hour - trying to
                                             * determine best reboot time */
#define HelpFile      SystemDir "help.txt"    /* For undefined helps */
#define TempFile      VendorDir "temp.txt"    /* For undefined helps */
#define PidFile	"../swr.pid"    /* Pid File */
/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CharData
#define MID	MobIndexData
#define OD	ObjData
#define OID	ObjIndexData
#define RID	RoomIndexData
#define SF	SpecFun
#define CL	ClanData
#define EDD	ExtraDescrData
#define RD	ResetData
#define	ST	SocialType
#define	CO	CouncilData
#define DE	DeityData
#define SK	SkillType
#define SH      ShipData
#ifdef MCCP
/* mccp.cpp */
                   bool compressStart
                   args((DescriptorData * d, unsigned char telopt));
                   bool compressEnd args((DescriptorData * d));
#endif
/* act_comm.cpp */
                   void log_printf args((const char *fmt,...));
                   void copyover_recover args((void));
                   bool has_comlink args((CharData * ch));
                   bool circle_follow
                   args((CharData * ch, CharData * victim));
                   void add_follower
                   args((CharData * ch, CharData * master));
                   void stop_follower args((CharData * ch));
                   void die_follower args((CharData * ch));
                   bool is_same_group
                   args((CharData * ach, CharData * bch));
                   void send_rip_screen args((CharData * ch));
                   void send_rip_title args((CharData * ch));
                   void send_ansi_title args((CharData * ch));
                   void send_ascii_title args((CharData * ch));
                   void to_channel
                   args((const char *argument, int channel, const char *verb,
                         sh_int level));
                   void info_chan args((char *argument));
                   void web_broadcast args((char *argument));
                   void auto_save args((CharData * ch));
                   void talk_auction args((char *argument));
                   bool knows_language
                   args((CharData * ch, LanguageData * lang,
                         CharData * cch));
                   bool can_learn_lang
                   args((CharData * ch, LanguageData * lang));
                   char *translate
                   args((CharData * ch, CharData * victim,
                         const char *argument));
                   char *obj_short args((ObjData * obj));
/* act_info.cpp */
                   int get_door args((char *arg));
                   char *format_obj_to_char
                   args((ObjData * obj, CharData * ch, bool fShort));
                   void show_list_to_char
                   args((ObjData * list, CharData * ch, bool fShort,
                         bool fShowNothing));
                   void save_help args((void));
/* finger2.cpp */
                   void read_finger args((CharData * ch, char *argument));
                   void fread_finger
                   args((CharData * ch, FILE * fp, char *laston));
                   void save_finger args((CharData * ch));
/* act_move.cpp */
                   void clear_vrooms args((void));
                   ExitData * find_door args((CharData * ch, char *arg, bool quiet));
                   ExitData * get_exit args((RoomIndexData * room, sh_int dir));
                   ExitData * get_exit_to args((RoomIndexData * room, sh_int dir, int vnum));
                   ExitData * get_exit_num args((RoomIndexData * room, sh_int count));
                   ch_ret move_char args((CharData * ch, ExitData * pexit, int fall,bool running));
                   sh_int encumbrance args((CharData * ch, sh_int move));
                   bool will_fall args((CharData * ch, int fall));
                   int wherehome args((CharData * ch));
                   CMDF do_hold_person args((CharData * ch, const char *argument));
                   CMDF do_release args((CharData * ch, const char *argument));
                   char *wordwrap args((char *txt, sh_int wrap));
/* act_obj.cpp */
                   obj_ret damage_obj args((ObjData * obj));
                   sh_int get_obj_resistance args((ObjData * obj));
                   void save_clan_storeroom
                   args((CharData * ch, ClanData * clan));
                   void obj_fall args((ObjData * obj, bool through));
/* act_wiz.cpp */
                   void close_area args((AreaData * pArea));
                   RID * find_location args((CharData * ch, char *arg));
                   void echo_to_room
                   args((sh_int AtColor, RoomIndexData * room,
                         char *argument));
                   void echo_to_all
                   args((sh_int AtColor, char *argument, sh_int tar));
                   void echo_to_clan
                   args((sh_int AtColor, char *argument, ClanData * clan));
                   void add_to_wizlist
                   args((char *name, int level, int flags));
                   void really_destroy_ship args((ShipData * ship));
                   void really_destroy_mob_ship args((ShipData * ship));
                   CMDF do_webroom(char *argument);
                   void transship(ShipData * ship, int destination);
                   bool validate_spec_fun(char *name);
                   void get_reboot_string args((void));
                   struct tm * update_time args((struct tm * old_time));
                   void free_social args((SocialType * social));
                   void add_social args((SocialType * social));
                   void free_command args((CMDType * command));
                   void unlink_command args((CMDType * command));
                   void add_command args((CMDType * command));
/* astral.cpp */
                   
/* build.cpp */
                   char *flag_string
                   args((int bitvector, const char *const flagarray[]));
                   int get_mpflag args((char *flag));
                   int get_dir args((char *txt));
                   char *strip_cr args((char *str));
                   int get_vip_flag args((char *flag));
                   int get_wanted_flag args((char *flag));
                   void set_bexit_flag args((ExitData * pexit, int flag));
/* clans.cpp */
                   CL * get_clan args((char *name));
                   void load_clans args((void));
                   void save_clan args((ClanData * clan));
                   long get_taxes args((PlanetData * planet));
                   bool is_clan_ally
                   args((CharData * ch, CharData * victim));
                   bool is_clan_enemy
                   args((CharData * ch, CharData * victim));
                   bool is_same_clan
                   args((CharData * ch, CharData * victim));
/* space.cpp */
                   SH * get_ship args((char *name));
                   void load_ships args((void));
                   void save_ship args((ShipData * ship));
                   void load_space args((void));
                   void save_starsystem args((SpaceData * starsystem));
                   SpaceData * starsystem_from_name args((char *name));
                   SpaceData * starsystem_from_vnum args((int vnum));
                   ShipData * ship_from_obj args((int vnum));
                   ShipData * ship_from_entrance args((int vnum));
                   ShipData * ship_from_hanger args((int vnum));
                   ShipData * ship_from_cockpit args((int vnum));
                   ShipData * ship_from_navseat args((int vnum));
                   ShipData * ship_from_coseat args((int vnum));
                   ShipData * ship_from_pilotseat args((int vnum));
                   ShipData * ship_from_gunseat args((int vnum));
                   ShipData * ship_from_turret args((int vnum));
                   ShipData * ship_from_engine args((int vnum));
                   ShipData *
                   get_ship_here args((char *name, SpaceData * starsystem));
                   void update_space args((void));
                   void recharge_ships args((void));
                   void move_ships args((void));
                   void update_traffic args((void));
                   bool check_pilot args((CharData * ch, ShipData * ship));
                   bool is_rental args((CharData * ch, ShipData * ship));
                   void echo_to_ship
                   args((int color, ShipData * ship, char *argument));
                   void echo_to_cockpit
                   args((int color, ShipData * ship, char *argument));
                   void echo_to_system
                   args((int color, ShipData * ship, char *argument,
                         ShipData * ignore));
                   bool extract_ship args((ShipData * ship));
                   bool ship_to_room args((ShipData * ship, int vnum));
                   long get_ship_value args((ShipData * ship));
                   bool rent_ship args((CharData * ch, ShipData * ship));
                   void damage_ship
                   args((ShipData * ship, int min, int max));
                   void damage_ship_ch
                   args((ShipData * ship, int min, int max, CharData * ch));
                   void destroy_ship args((ShipData * ship, CharData * ch));
                   void ship_to_starsystem
                   args((ShipData * ship, SpaceData * starsystem));
                   void ship_from_starsystem
                   args((ShipData * ship, SpaceData * starsystem));
                   void new_missile
                   args((ShipData * ship, ShipData * target, CharData * ch,
                         int missiletype));
                   void extract_missile args((MissileData * missile));
                   ShipData *
                   ship_in_room args((RoomIndexData * room, char *name));
                   void damage_ship_ch_ion
                   args((ShipData * ship, int min, int max, CharData * ch));
                   void damage_ship_ion
                   args((ShipData * ship, int min, int max));
                   ShipData * ship_from_room args((int vnum));
                   void wipe_resets
                   args((AreaData * pArea, RoomIndexData * pRoom));
                   bool autofly args((ShipData * ship));
/* comm.cpp */
                   void close_socket
                   args((DescriptorData * dclose, bool Force));
                   bool write_to_buffer
                   args((DescriptorData * d, const char *txt, int length));
                   void write_to_pager
                   args((DescriptorData * d, const char *txt, int length));
                   void send_to_char args((const char *txt, CharData * ch));
                   void center_to_char
                   args((char *argument, CharData * ch, int columns));
                   char *center_str args((const char *argument, int columns));
                   void send_to_char_color
                   args((const char *txt, CharData * ch));
                   void send_to_pager args((const char *txt, CharData * ch));
                   void send_to_pager_color
                   args((const char *txt, CharData * ch));
                   void set_char_color args((sh_int AType, CharData * ch));
                   void set_pager_color args((sh_int AType, CharData * ch));
                   void ch_printf args((CharData * ch, const char *fmt,...));
                   void pager_printf args((CharData * ch, const char *fmt,...));
                   void act
                   args((sh_int AType, const char *format, CharData * ch,
                         void *arg1, void *arg2, int type));
                   int strlen_color args((const char *argument));
                   extern const unsigned char do_termtype_str[];
                   extern const unsigned char will_compress_str[];
                   extern const unsigned char will_compress2_str[];
/* reset.cpp */
                   RD *
                   make_reset
                   args((char letter, int extra, int arg1, int arg2,
                         int arg3));
                   RD *
                   add_reset
                   args((AreaData * tarea, char letter, int extra, int arg1,
                         int arg2, int arg3));
                   RD *
                   place_reset
                   args((AreaData * tarea, char letter, int extra, int arg1,
                         int arg2, int arg3));
                   void reset_area args((AreaData * pArea));
                   void instaroom
                   args((AreaData * pArea, RoomIndexData * pRoom,
                         bool dodoors));
                   void add_obj_reset
                   args((AreaData * pArea, char cm, ObjData * obj, int v2,
                         int v3));
/* swskills.cpp */
                   void add_reinforcements args((CharData * ch));
                   void load_races args((void));
/* web-server.cpp */
                   void init_web(int port); void handle_web(void);
                   void shutdown_web(void);

/* db.cpp */
                   void boot_log args((const char *str,...));
                   void clear_file args((CharData * ch, char *filename));
                   void show_file args((CharData * ch, const char *filename));
                   int file_size args((char *buf));
                   bool file_exist args((char *name));
                   char *str_dup args((char const *str));
                   void boot_db args((bool fCopyOver));
                   void area_update args((void));
                   void add_char args((CharData * ch));
                   CD * create_mobile args((MobIndexData * pMobIndex));
                   OD *
                   create_object
                   args((ObjIndexData * pObjIndex, int level));
                   void clear_char args((CharData * ch));
                   void free_char args((CharData * ch));
                   char *get_extra_descr
                   args((const char *name, ExtraDescrData * ed));
                   MID * get_mob_index args((int vnum));
                   OID * get_obj_index args((int vnum));
                   RID * get_room_index args((int vnum));
                   char fread_letter args((FILE * fp));
                   int fread_number args((FILE * fp));
                   float fread_float args((FILE * fp));
                   char *fread_string args((FILE * fp));
                   char *fread_string_nohash args((FILE * fp));
                   char *fread_string_noalloc args((FILE * fp));
                   void fread_to_eol args((FILE * fp));
                   char *fread_word args((FILE * fp));
                   char *fread_line args((FILE * fp));
                   float fread_float args((FILE * fp));
                   long fread_long args((FILE * fp));
                   int number_fuzzy args((int number));
                   int number_range args((int from, int to));
                   int number_percent args((void));
                   int number_door args((void));
                   int number_bits args((int width));
                   int number_mm args((void));
                   int dice args((int number, int size));
                   int interpolate
                   args((int level, int value_00, int value_32));
                   void smash_tilde args((char *str));
                   void hide_tilde args((char *str));
                   char *show_tilde args((char *str));
                   bool str_cmp args((const char *astr, const char *bstr));
                // Modern C++23 overloads for str_cmp
                   inline bool str_cmp(const std::string& a, const std::string& b) { return str_cmp(a.c_str(), b.c_str()); }
                   inline bool str_cmp(const std::string_view a, const std::string_view b) { return str_cmp(a.data(), b.data()); }
                   inline bool str_cmp(const std::string& a, const char* b) { return str_cmp(a.c_str(), b); }
                   inline bool str_cmp(const char* a, const std::string& b) { return str_cmp(a, b.c_str()); }
                   bool str_prefix args((const char *astr, const char *bstr));
                   bool str_infix args((const char *astr, const char *bstr));
                   bool str_suffix args((const char *astr, const char *bstr));
                   const char *capitalize args((const char *str));
                   char *strlower args((const char *str));
                   char *strupper args((const char *str));
                   char *aoran args((const char *str));
                   void append_file
                   args((CharData * ch, const char *file, const char *str));
                   void append_to_file args((const char *file, const char *str));
                   void bug args((const char *str,...));
                   void log_string_plus
                   args((const char *str, sh_int log_type, sh_int level));
                   RID *make_room( int vnum, AreaData *area );
                   OID * make_object args((int vnum, int cvnum, char *name));
                   MID * make_mobile args((int vnum, int cvnum, char *name));
                   ExitData * make_exit args((RoomIndexData * pRoomIndex, RoomIndexData* to_room, sh_int door));
                   void delete_help args((HelpData * pHelp));
                   void add_help args((HelpData * pHelp));
                   void fix_area_exits args((AreaData * tarea));
                   void load_area_file
                   args((AreaData * tarea, char *filename));
                   void randomize_exits
                   args((RoomIndexData * room, sh_int maxdir));
                   void make_wizlist args((void));
                   void tail_chain args((void));
                   bool delete_room args((RoomIndexData * room));
                   bool delete_obj args((ObjIndexData * obj));
                   bool delete_mob args((MobIndexData * mob));
/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	args( ( MobIndexData *pMob ) );
void	obj_sort	args( ( ObjIndexData *pObj ) );
void	room_sort	args( ( RoomIndexData *pRoom ) );*/
                   void sort_area args((AreaData * pArea, bool proto));
                   void strdup_printf args((char **pointer, char *fmt,...));
                   void stralloc_printf args((char **pointer, char *fmt,...));
                   void command_printf args((CharData * ch, char *fmt,...));
                   size_t mudstrlcpy
                   args((char *dst, const char *src, size_t siz));
                   size_t mudstrlcat
                   args((char *dst, const char *src, size_t siz));
/* build.cpp */
                   bool can_rmodify
                   args((CharData * ch, RoomIndexData * room));
                   bool can_omodify args((CharData * ch, ObjData * obj));
                   bool can_mmodify args((CharData * ch, CharData * mob));
                   bool can_medit
                   args((CharData * ch, MobIndexData * mob));
                   void free_reset args((AreaData * are, ResetData * res));
                   void free_area args((AreaData * are));
                   void assign_area args((CharData * ch));
                   EDD *
                   SetRExtra args((RoomIndexData * room, char *keywords));
                   bool DelRExtra
                   args((RoomIndexData * room, char *keywords));
                   EDD * SetOExtra args((ObjData * obj, char *keywords));
                   bool DelOExtra args((ObjData * obj, char *keywords));
                   EDD *
                   SetOExtraProto
                   args((ObjIndexData * obj, char *keywords));
                   bool DelOExtraProto
                   args((ObjIndexData * obj, char *keywords));
                   void fold_area
                   args((AreaData * tarea, char *filename, bool install,
                         bool dolog));
                   int get_otype args((char *type));
                   int get_atype args((char *type));
                   int get_aflag args((char *flag));
                   int get_oflag args((char *flag));
                   int get_wflag args((char *flag));
                   RaceData * get_race args((const char *string));
                   LanguageData * get_language args((const char *string));
                   RaceData * get_race_number args((int number));
/* channels.cpp */
                   void removename args((char **list, const char *name));
                   void addname args((char **list, const char *name));
                   int hasname args((const char *list, const char *name));
                   const char *getarg
                   args((const char *argument, char *arg, int length));
/* fight.cpp */
                   int max_fight args((CharData * ch));
                   void violence_update args((void));
                   ch_ret multi_hit
                   args((CharData * ch, CharData * victim, int dt));
                   sh_int ris_damage
                   args((CharData * ch, sh_int dam, int ris));
                   ch_ret damage
                   args((CharData * ch, CharData * victim, int dam,
                         int dt));
                   void update_pos args((CharData * victim));
                   void set_fighting
                   args((CharData * ch, CharData * victim));
                   void stop_fighting args((CharData * ch, bool fBoth));
                   void free_fight args((CharData * ch));
                   CD * who_fighting args((CharData * ch));
                   void check_killer
                   args((CharData * ch, CharData * victim));
                   void check_attacker
                   args((CharData * ch, CharData * victim));
                   void death_cry args((CharData * ch));
                   void stop_hunting args((CharData * ch));
                   void stop_hating args((CharData * ch));
                   void stop_fearing args((CharData * ch));
                   void start_hunting
                   args((CharData * ch, CharData * victim));
                   void start_hating
                   args((CharData * ch, CharData * victim));
                   void start_fearing
                   args((CharData * ch, CharData * victim));
                   bool is_hunting args((CharData * ch, CharData * victim));
                   bool is_hating args((CharData * ch, CharData * victim));
                   bool is_fearing args((CharData * ch, CharData * victim));
                   bool is_safe args((CharData * ch, CharData * victim));
                   bool is_safe_nm args((CharData * ch, CharData * victim));
                   bool legal_loot args((CharData * ch, CharData * victim));
                   bool check_illegal_pk
                   args((CharData * ch, CharData * victim));
                   void raw_kill args((CharData * ch, CharData * victim));
                   bool in_arena args((CharData * ch));
                   int in_hash_table args((char *str));
/* makeobjs.cpp */
                   void make_corpse
                   args((CharData * ch, CharData * killer));
                   void make_blood args((CharData * ch));
                   void make_bloodstain args((CharData * ch));
                   void make_scraps args((ObjData * obj));
                   void make_fire
                   args((RoomIndexData * in_room, sh_int timer));
                   OD * make_trap args((int v0, int v1, int v2, int v3));
                   OD * create_money args((int amount));
/* misc.cpp */
                   void actiondesc
                   args((CharData * ch, ObjData * obj, void *vo));
                   void jedi_checks args((CharData * ch));
                   void jedi_bonus args((CharData * ch));
                   void sith_penalty args((CharData * ch));
/* mud_comm.cpp */
                   char *mprog_type_to_name args((int type));
/* mud_prog.cpp */
#ifdef DunnoStrstr
                   char *strstr args((const char *s1, const char *s2));
#endif
                   void mprog_wordlist_check args((char *arg, CharData * mob,
                                                   CharData * actor,
                                                   ObjData * object,
                                                   void *vo, int type));
                   void mprog_percent_check
                   args((CharData * mob, CharData * actor,
                         ObjData * object, void *vo, int type));
                   void mprog_act_trigger
                   args((char *buf, CharData * mob, CharData * ch,
                         ObjData * obj, void *vo));
                   void mprog_bribe_trigger
                   args((CharData * mob, CharData * ch, int amount));
                   void mprog_entry_trigger args((CharData * mob));
                   void mprog_give_trigger
                   args((CharData * mob, CharData * ch, ObjData * obj));
                   void mprog_greet_trigger args((CharData * mob));
                   void mprog_fight_trigger
                   args((CharData * mob, CharData * ch));
                   void mprog_hitprcnt_trigger
                   args((CharData * mob, CharData * ch));
                   void mprog_death_trigger
                   args((CharData * killer, CharData * mob));
                   void mprog_random_trigger args((CharData * mob));
                   void mprog_speech_trigger
                   args((char *txt, CharData * mob));
                   void mprog_script_trigger args((CharData * mob));
                   void mprog_hour_trigger args((CharData * mob));
                   void mprog_time_trigger args((CharData * mob));
                   void progbug args((char *str, CharData * mob));
                   void rset_supermob args((RoomIndexData * room));
                   void release_supermob args((void));
                   void mpsleep_update args((void));
/* player.cpp */
                   void set_title args((CharData * ch, char *title));
                   char *convert_newline args((char *str));
/* skills.cpp */
                   bool check_skill
                   args((CharData * ch, char *command, char *argument));
                   void learn_from_success args((CharData * ch, int sn));
                   void learn_from_failure args((CharData * ch, int sn));
                   bool check_parry
                   args((CharData * ch, CharData * victim));
                   bool check_dodge
                   args((CharData * ch, CharData * victim));
                   bool check_grip args((CharData * ch, CharData * victim));
                   void disarm args((CharData * ch, CharData * victim));
                   void trip args((CharData * ch, CharData * victim));
/* handler.cpp */
                   void explode args((ObjData * obj));
                   int get_exp args((CharData * ch, int ability));
                   int get_exp_worth args((CharData * ch));
                   int exp_level args((sh_int level));
                   sh_int get_trust args((CharData * ch));
                   sh_int get_age args((CharData * ch));
                   sh_int get_curr_str args((CharData * ch));
                   sh_int get_curr_int args((CharData * ch));
                   sh_int get_curr_wis args((CharData * ch));
                   sh_int get_curr_dex args((CharData * ch));
                   sh_int get_curr_con args((CharData * ch));
                   sh_int get_curr_cha args((CharData * ch));
                   sh_int get_curr_lck args((CharData * ch));
                   sh_int get_curr_frc args((CharData * ch));
                   sh_int max_languages args((CharData * ch));
                   bool can_take_proto args((CharData * ch));
                   int can_carry_n args((CharData * ch));
                   int can_carry_w args((CharData * ch));
                   bool is_name args((const char *str, char *namelist));
                   bool is_name_prefix
                   args((const char *str, char *namelist));
                   bool nifty_is_name args((char *str, char *namelist));
                   bool nifty_is_name_prefix
                   args((char *str, char *namelist));
                   void affect_modify
                   args((CharData * ch, AffectData * paf, bool fAdd));
                   void affect_to_char
                   args((CharData * ch, AffectData * paf));
                   void affect_remove
                   args((CharData * ch, AffectData * paf));
                   void affect_strip args((CharData * ch, int sn));
                   bool is_affected args((CharData * ch, int sn));
                   void affect_join args((CharData * ch, AffectData * paf));
                   void char_from_room args((CharData * ch));
                   void char_to_room
                   args((CharData * ch, RoomIndexData * pRoomIndex));
                   OD * obj_to_char args((ObjData * obj, CharData * ch));
                   void obj_from_char args((ObjData * obj));
                   int apply_ac args((ObjData * obj, int iWear));
                   OD * get_eq_char args((CharData * ch, int iWear));
                   void equip_char
                   args((CharData * ch, ObjData * obj, int iWear));
                   void unequip_char args((CharData * ch, ObjData * obj));
                   int count_obj_list
                   args((ObjIndexData * obj, ObjData * list));
                   void obj_from_room args((ObjData * obj));
                   OD *
                   obj_to_room
                   args((ObjData * obj, RoomIndexData * pRoomIndex));
                   OD * obj_to_obj args((ObjData * obj, ObjData * obj_to));
                   void obj_from_obj args((ObjData * obj));
                   void extract_obj args((ObjData * obj));
                   void extract_exit
                   args((RoomIndexData * room, ExitData * pexit));
                   void extract_room args((RoomIndexData * room));
                   void clean_room args((RoomIndexData * room));
                   void clean_obj args((ObjIndexData * obj));
                   void clean_mob args((MobIndexData * mob));
                   void clean_resets args((AreaData * tarea));
                   void extract_char args((CharData * ch, bool fPull));
                   CD * get_char_room args((CharData * ch, char *argument));
                   CD * get_char_world args((CharData * ch, char *argument));
                   CD * get_char_world_nocheck args((char *argument));
                   OD * get_obj_type args((ObjIndexData * pObjIndexData));
                   OD *
                   get_obj_list
                   args((CharData * ch, char *argument, ObjData * list));
                   OD *
                   get_obj_list_rev
                   args((CharData * ch, char *argument, ObjData * list));
                   OD * get_obj_carry args((CharData * ch, char *argument));
                   OD *
                   get_obj_carry_type
                   args((CharData * ch, char *argument, int type));
                   OD * get_obj_wear args((CharData * ch, char *argument));
                   OD * get_obj_here args((CharData * ch, char *argument));
                   OD * get_obj_world args((CharData * ch, char *argument));
                   int get_obj_number args((ObjData * obj));
                   int get_obj_weight args((ObjData * obj));
                   bool room_is_dark args((RoomIndexData * pRoomIndex));
                   bool room_is_private
                   args((CharData * ch, RoomIndexData * pRoomIndex));
                   bool can_see args((CharData * ch, CharData * victim));
                   bool can_see_ooc
                   args((CharData * ch, CharData * victim));
                   bool can_see_obj args((CharData * ch, ObjData * obj));
                   bool can_drop_obj args((CharData * ch, ObjData * obj));
                   char *item_type_name args((ObjData * obj));
                   char *affect_loc_name args((int location));
                   char *affect_bit_name args((int vector));
                   char *extra_bit_name args((int extra_flags));
                   char *magic_bit_name args((int magic_flags));
                   ch_ret check_for_trap
                   args((CharData * ch, ObjData * obj, int flag));
                   ch_ret check_room_for_traps
                   args((CharData * ch, int flag));
                   bool is_trapped args((ObjData * obj));
                   OD * get_trap args((ObjData * obj));
                   ch_ret spring_trap args((CharData * ch, ObjData * obj));
                   void name_stamp_stats args((CharData * ch));
                   void fix_char args((CharData * ch));
                   void showaffect args((CharData * ch, AffectData * paf));
                   void set_cur_obj args((ObjData * obj));
                   bool obj_extracted args((ObjData * obj));
                   void queue_extracted_obj args((ObjData * obj));
                   void clean_obj_queue args((void));
                   bool char_died args((CharData * ch));
                   void queue_extracted_char
                   args((CharData * ch, bool extract));
                   void clean_char_queue args((void));
                   void add_timer
                   args((CharData * ch, sh_int type, sh_int count,
                         DoFun * fun, int value));
                   Timer * get_timerptr args((CharData * ch, sh_int type));
                   sh_int get_timer args((CharData * ch, sh_int type));
                   void extract_timer args((CharData * ch, Timer * timer));
                   void remove_timer args((CharData * ch, sh_int type));
                   bool in_soft_range
                   args((CharData * ch, AreaData * tarea));
                   bool in_hard_range
                   args((CharData * ch, AreaData * tarea));
                   bool chance args((CharData * ch, sh_int percent));
                   bool chance_attrib
                   args((CharData * ch, sh_int percent, sh_int attrib));
                   OD * clone_object args((ObjData * obj));
                   void split_obj args((ObjData * obj, int num));
                   void separate_obj args((ObjData * obj));
                   bool empty_obj
                   args((ObjData * obj, ObjData * destobj,
                         RoomIndexData * destroom));
                   OD *
                   find_obj
                   args((CharData * ch, char *argument, bool carryonly));
                   bool ms_find_obj args((CharData * ch));
                   void worsen_mental_state args((CharData * ch, int Mod));
                   void better_mental_state args((CharData * ch, int Mod));
                   void boost_economy args((AreaData * tarea, int gold));
                   void lower_economy args((AreaData * tarea, int gold));
                   void economize_mobgold args((CharData * mob));
                   bool economy_has args((AreaData * tarea, int gold));
                   void add_kill args((CharData * ch, CharData * mob));
                   int times_killed args((CharData * ch, CharData * mob));
                   char *num_punct args((int foo));
                   char *num_punct_long args((long int foo));
                   int count_users args((ObjData * obj));
                   char *smash_color args((char *str));
                   char *full_color args((char *str));
                   char *smash_space args((const char *str));
                   void add_request
                   args((CharData * ch, CharData * victim, char *argument,
                         char *syntax));
/* interp.cpp */
                   bool check_pos args((CharData * ch, sh_int position));
                   void interpret args((CharData * ch, char *argument));
                   bool is_number args((char *arg));
                   int number_argument args((char *argument, char *arg));
                   char *one_argument args((char *argument, char *arg_first));
                   char *one_argument2
                   args((char *argument, char *arg_first));
                   ST * find_social args((char *command));
                   ST * find_xsocial args((char *command));
                   CMDType * find_command args((char *command));
                   void hash_commands args((void));
                   void start_timer args((struct timeval * _stime));
                   time_t end_timer args((struct timeval * _stime));
                   void send_timer
                   args((struct timerset * vtime, CharData * ch));
                   void update_userec
                   args((struct timeval * time_used,
                         struct timerset * userec));
                   bool check_command
                   args((CharData * ch, CMDType * command));
/* magic.cpp */
                   bool process_spell_components
                   args((CharData * ch, int sn));
                   int ch_slookup args((CharData * ch, const char *name));
                   int find_spell
                   args((CharData * ch, const char *name, bool know));
                   int find_skill
                   args((CharData * ch, const char *name, bool know));
                   int find_weapon
                   args((CharData * ch, const char *name, bool know));
                   int find_tongue
                   args((CharData * ch, const char *name, bool know));
                   int skill_lookup args((const char *name));
                   int herb_lookup args((const char *name));
                   int personal_lookup
                   args((CharData * ch, const char *name));
                   int slot_lookup args((int slot));
                   int bsearch_skill
                   args((const char *name, int first, int top));
                   int bsearch_skill_exact
                   args((const char *name, int first, int top));
                   bool saves_poison_death
                   args((int level, CharData * victim));
                   bool saves_wand args((int level, CharData * victim));
                   bool saves_para_petri
                   args((int level, CharData * victim));
                   bool saves_breath args((int level, CharData * victim));
                   bool saves_spell_staff
                   args((int level, CharData * victim));
                   ch_ret obj_cast_spell
                   args((int sn, int level, CharData * ch,
                         CharData * victim, ObjData * obj));
                   int dice_parse
                   args((CharData * ch, int level, char *_exp));
                   SK * get_skilltype args((int sn));
/* save.cpp */
/* object saving defines for fread/write_obj. -- Altrag */
#define OsCarry	static_cast<sh_int>(0)
#define OsCorpse	static_cast<sh_int>(1)
                   void save_char_obj args((CharData * ch));
                   void save_clone args((CharData * ch));
                   bool load_char_obj
                   args((DescriptorData * d, char *name, bool preload,
                         bool copyover));
                   void set_alarm args((long seconds));
                   void requip_char args((CharData * ch));
                   void fwrite_obj
                   args((CharData * ch, ObjData * obj, FILE * fp, int iNest,
                         sh_int os_type, bool hotboot));
                   void fread_obj
                   args((CharData * ch, FILE * fp, sh_int os_type));
                   void de_equip_char args((CharData * ch));
                   void re_equip_char args((CharData * ch));
                   void save_home args((CharData * ch));
                   void load_home args((CharData * ch));
/* shops.cpp */
                   CD * find_keeper args((CharData * ch));
/* special.cpp */
                   SF * spec_lookup args((char *name));
/* tables.cpp */
                   int get_skill args((char *skilltype));
                   char *spell_name args((SpellFun * spell));
                   char *skill_name args((DoFun * skill));
                   void load_skill_table args((void));
                   void save_skill_table args((void));
                   void sort_skill_table args((void));
                   void load_socials args((void));
                   void save_socials args((void));
                   void load_commands args((void));
                   void save_commands args((void));
                   SpellFun * spell_function args((char *name));
                   DoFun * skill_function args((char *name));
                   void load_herb_table args((void));
                   void save_herb_table args((void));
/* track.cpp */
                   void found_prey args((CharData * ch, CharData * victim));
                   void hunt_victim args((CharData * ch));
/* update.cpp */
                   void advance_level args((CharData * ch, int ability));
                   void gain_exp
                   args((CharData * ch, int gain, int ability));
                   void gain_exp_new
                   args((CharData * ch, int gain, int ability,
                         bool outtext));
                   void gain_condition
                   args((CharData * ch, int iCond, int value));
                   void update_handler args((void));
                   void reboot_check args((time_t reset));
#if 0
                   void reboot_check args((char *arg));
#endif
                   void auction_update args((void));
                   void remove_portal args((ObjData * portal));
                   int max_level(CharData * ch, int ability);
/* hashstr.cpp */
                   char *str_alloc args((const char *str));
                   char *quick_link args((char *str));
                   int str_free args((char *str));
                   int allocated_strings args((void));
                   void show_hash args((int count));
                   char *hash_stats args((void));
                   char *check_hash args((char *str));
                   void hash_dump args((int hash));
                   void show_high_hash args((int top));
/* newscore.cpp */
                   void web_colourconv(char *buffer, const char *txt);

/* vendor.cpp */
                   void fwrite_vendor args((FILE * fp, CharData * mob));
                   CharData * fread_vendor args((FILE * fp));
                   void load_vendors args((void));
                   void save_vendor args((CharData * ch));
#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED
#define SubNorth DirNorth
#define SubEast  DirEast
#define SubSouth DirSouth
#define SubWest  DirWest
#define SubUp    DirUp
#define SubDown  DirDown
#define SubNe    DirNortheast
#define SubNw    DirNorthwest
#define SubSe    DirSoutheast
#define SubSw    DirSouthwest
/*
 * defines for use with this get_affect function
 */
constexpr int Ris000		= 0;
constexpr int RisR00		= 1;
constexpr int Ris0I0		= 2;
constexpr int RisRi0		= 3;
constexpr int Ris00S		= 4;
constexpr int RisR0S		= 5;
constexpr int Ris0Is		= 6;
constexpr int RisRis		= 7;
constexpr int GaAffected	= 9;
constexpr int GaResistant	= 10;
constexpr int GaImmune	= 11;
constexpr int GaSusceptible	= 12;
constexpr int GaRis          = 30;
/*
 * mudprograms stuff
 */
                   extern CharData * supermob;
                   void oprog_speech_trigger(char *txt, CharData * ch);
                   void oprog_random_trigger(ObjData * obj);
                   void oprog_wear_trigger(CharData * ch, ObjData * obj);
                   bool oprog_use_trigger(CharData * ch, ObjData * obj,
                                          CharData * vict, ObjData * targ,
                                          void *vo);
                   void oprog_remove_trigger(CharData * ch, ObjData * obj);
                   void oprog_sac_trigger(CharData * ch, ObjData * obj);
                   void oprog_damage_trigger(CharData * ch, ObjData * obj);
                   void oprog_repair_trigger(CharData * ch, ObjData * obj);
                   void oprog_drop_trigger(CharData * ch, ObjData * obj);
                   void oprog_zap_trigger(CharData * ch, ObjData * obj);
                   char *oprog_type_to_name(int type);
/*
 * MudProgs START HERE
 * (object stuff)
 */
                   void oprog_greet_trigger(CharData * ch);
                   void oprog_get_trigger(CharData * ch, ObjData * obj);
                   void oprog_examine_trigger(CharData * ch, ObjData * obj);
                   void oprog_pull_trigger(CharData * ch, ObjData * obj);
                   void oprog_push_trigger(CharData * ch, ObjData * obj);
/* mud prog defines */
constexpr int  ErrorProg   = -1;
constexpr int  InFileProg         = 0;
constexpr int ActProg      = 0;
constexpr int  SpeechProg        = 1;
constexpr int  RandProg          = 2;
constexpr int  FightProg         = 3;
constexpr int  RfightProg        = 3;
constexpr int  DeathProg        = 4;
constexpr int  RdeathProg       = 4;
constexpr int  HitprcntProg     = 5;
constexpr int  EntryProg        = 6;
constexpr int  EnterProg        = 6;
constexpr int GreetProg         = 7;
constexpr int RgreetProg        = 7;
constexpr int OgreetProg        = 7;
constexpr int AllGreetProg      = 8;
constexpr int GiveProg          = 9;
constexpr int BribeProg         = 10;
constexpr int HourProg          = 11;
constexpr int TimeProg          = 12;
constexpr int WearProg          = 13;
constexpr int RemoveProg        = 14;
constexpr int SacProg           = 15;
constexpr int LookProg          = 16;
constexpr int ExaProg           = 17;
constexpr int ZapProg           = 18;
constexpr int GetProg           = 19;
constexpr int DropProg          = 20;
constexpr int DamageProg        = 21;
constexpr int RepairProg        = 22;
constexpr int RandiwProg        = 23;
constexpr int SpeechiwProg      = 24;
constexpr int PullProg          = 25;
constexpr int PushProg          = 26;
constexpr int SleepProg         = 27;
constexpr int RestProg          = 28;
constexpr int LeaveProg         = 29;
constexpr int ScriptProg        = 30;
constexpr int UseProg           = 31;
                   void rprog_leave_trigger(CharData * ch);
                   void rprog_enter_trigger(CharData * ch);
                   void rprog_sleep_trigger(CharData * ch);
                   void rprog_rest_trigger(CharData * ch);
                   void rprog_rfight_trigger(CharData * ch);
                   void rprog_death_trigger(CharData * killer,
                                            CharData * ch);
                   void rprog_speech_trigger(char *txt, CharData * ch);
                   void rprog_random_trigger(CharData * ch);
                   void rprog_time_trigger(CharData * ch);
                   void rprog_hour_trigger(CharData * ch);
                   char *rprog_type_to_name(int type);
#define OprogActTrigger
#ifdef OprogActTrigger
                   void oprog_act_trigger(char *buf, ObjData * mobj,
                                          CharData * ch, ObjData * obj,
                                          void *vo);
#endif
#define RprogActTrigger
#ifdef RprogActTrigger
                   void rprog_act_trigger(char *buf, RoomIndexData * room,
                                          CharData * ch, ObjData * obj,
                                          void *vo);
#endif
