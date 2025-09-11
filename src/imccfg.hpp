/* Codebase macros - Change as you might need.
 * Yes, Rogel, you can gloat all you want. You win, this is cleaner, though not by a whole lot.
 */


#pragma once
#include <string>


#if !defined(IMCCIRCLE) && !defined(IMCSTANDALONE)
// Modernized: prefer inline functions for type safety
inline auto CH_IMCDATA(auto ch) { return ch->pcdata->imcchardata; }
inline auto CH_IMCLEVEL(auto ch) { return ch->top_level; }
inline std::string CH_IMCNAME(auto ch) { return ch->name; }
inline auto CH_IMCSEX(auto ch) { return ch->sex; }
inline std::string CH_IMCTITLE(auto ch) { return ch->pcdata->title; }
#endif


#if defined(IMCSMAUG) || defined(IMCCHRONICLES)
#define SMAUGSOCIAL
using SOCIAL_DATA = SocialType;
inline auto CH_IMCRANK(auto ch) { return ch->pcdata->rank; }
#endif


#if defined(IMCROM)
inline auto first_descriptor() { return descriptor_list; }
inline std::string CH_IMCRANK(auto ch) { return title_table[ch->class][ch->level][ch->sex == SEX_FEMALE ? 1 : 0]; }
#endif


#if defined(IMCMERC)
inline auto first_descriptor() { return descriptor_list; }
inline std::string CH_IMCRANK(auto ch) { return title_table[ch->class][ch->level][ch->sex == SEX_FEMALE ? 1 : 0]; }
#endif


#if defined(IMCACK)
inline auto first_descriptor() { return first_desc; }
inline std::string CH_IMCRANK(auto ch) { return class_table[ch->class].who_name; }
#endif


#if defined(IMCUENVY)
#define SMAUGSOCIAL
using SOCIAL_DATA = SOC_INDEX_DATA;
SOC_INDEX_DATA* find_social(const std::string& command);

inline auto first_descriptor() { return descriptor_list; }
inline std::string CH_IMCRANK(auto ch) { return title_table[ch->class][ch->level][ch->sex == SEX_FEMALE ? 1 : 0]; }
#endif

/* Blasted circle, always gotta do thing the hard way! */

#if defined(IMCCIRCLE)
#define SMAUGSOCIAL

struct social_messg {
        int act_nr = 0;
        int hide = 0;
        int min_victim_position = 0;
        std::string char_no_arg;
        std::string others_no_arg;
        std::string char_found;
        std::string others_found;
        std::string vict_found;
        std::string not_found;
        std::string char_auto;
        std::string others_auto;
};

extern social_messg* soc_mess_list;
social_messg* find_social(const std::string& name);

using SOCIAL_DATA = social_messg;
using CharData = char_data;
using DescriptorData = DescriptorData;

extern const char* class_abbrevs[];

inline std::string title_female(int chclass, int level);
inline std::string title_male(int chclass, int level);

inline auto first_descriptor() { return descriptor_list; }
inline int URANGE(int a, int b, int c) { return (b < a ? a : (b > c ? c : b)); }
inline auto CH_IMCDATA(auto ch) { return ch->player_specials->imcchardata; }
inline auto CH_IMCLEVEL(auto ch) { return GET_LEVEL(ch); }
inline std::string CH_IMCNAME(auto ch) { return GET_NAME(ch); }
inline std::string CH_IMCTITLE(auto ch) { return GET_TITLE(ch); }
inline std::string CH_IMCRANK(auto ch) { return GET_SEX(ch) == SEX_FEMALE ? title_female(GET_CLASS(ch), GET_LEVEL(ch)) : title_male(GET_CLASS(ch), GET_LEVEL(ch)); }
inline auto CH_IMCSEX(auto ch) { return GET_SEX(ch); }
#endif

#if defined(IMCSTANDALONE)

using bool = unsigned char;

#ifndef FALSE
constexpr int FALSE = 0;
#endif

#ifndef TRUE
constexpr int TRUE = 1;
#endif

inline auto CH_IMCDATA(auto ch) { return ch->imcchardata; }
inline auto CH_IMCLEVEL(auto ch) { return ch->top_level; }
inline std::string CH_IMCNAME(auto ch) { return ch->name; }
inline auto CH_IMCSEX(auto ch) { return ch->sex; }
inline std::string CH_IMCTITLE(auto) { return "User"; }
inline std::string CH_IMCRANK(auto) { return "User"; }

enum class genders { SEX_NEUTRAL, SEX_MALE, SEX_FEMALE };

constexpr int ConPlaying = 1;
inline char LOWER(char c) { return (c >= 'A' && c <= 'Z' ? c + 'a' - 'A' : c); }

struct user_data {
        imcchar_data* imcchardata = nullptr;
        std::string name;
        int level = 0;
        short sex = 0;
};

struct conn_data {
        conn_data* next = nullptr;
        conn_data* prev = nullptr;
        user_data* original = nullptr;
        user_data* character = nullptr;
        short connected = 0;
};

conn_data* first_descriptor = nullptr;
conn_data* last_descriptor = nullptr;

#endif
