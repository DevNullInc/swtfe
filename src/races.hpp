

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
 *                Data for Races and their attributes                                    *
 *****************************************************************************************/
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#include <cstdio>
#include <cstdint>
#include "mud.hpp"
#include "olc.hpp"

constexpr std::string_view FileRaceList = "race.lst";
constexpr std::string_view RacesDir = "../races/";

enum class RaceAttr : int
{
        Strength,
        Intelligence,
        Wisdom,
        Dexterity,
        Constitution,
        Charisma,
        Force,
        Luck,
        MaxAttr
};

class RaceData
{
private:
        ExtBV body_parts_;
        std::shared_ptr<LanguageData> language_;
        std::int32_t affected_{0};
        std::int16_t con_plus_{0};
        std::int16_t cha_plus_{0};
        std::int16_t dex_plus_{0};
        std::int16_t frc_plus_{0};
        std::int32_t hit_{0};
        std::int16_t int_plus_{0};
        std::int16_t lang_bonus_{0};
        std::int16_t lck_plus_{0};
        std::int32_t endurance_{0};
        std::string name_;
        std::int16_t str_plus_{0};
        std::int16_t wis_plus_{0};
        std::array<std::int16_t, MaxAbility> class_modifier_{};
        std::array<std::int16_t, static_cast<int>(RaceAttr::MaxAttr)> attr_mod_{};
        std::int32_t home_{0};
        std::int32_t death_age_{0};
        std::int16_t hunger_mod_{0};
        std::int16_t thirst_mod_{0};
        std::int32_t rpneeded_{0};
        std::int32_t start_age_{0};
        std::int32_t class_restriction_{0};

public:
        RaceData() = default;
        ~RaceData() = default;

        /**
         * @brief Get the language associated with the race.
         */
        std::shared_ptr<LanguageData> language() const { return language_; }
        void set_language(std::shared_ptr<LanguageData> p) { language_ = std::move(p); }

        ExtBV &body_parts() { return body_parts_; }
        void set_body_parts(std::size_t bit, bool set);
        void toggle_body_parts(std::size_t bit);

        const std::string &name() const { return name_; }
        void set_name(const std::string &newname) { name_ = newname; }

        std::int32_t class_restriction() const { return class_restriction_; }
        void set_class_restriction(std::int32_t p) { class_restriction_ = p; }

        std::int32_t start_age() const { return start_age_; }
        void set_start_age(std::int32_t p) { start_age_ = p; }

        std::int16_t thirst_mod() const { return thirst_mod_; }
        void set_thirst_mod(std::int16_t p) { thirst_mod_ = p; }

        std::int16_t hunger_mod() const { return hunger_mod_; }
        void set_hunger_mod(std::int16_t p) { hunger_mod_ = p; }

        std::int32_t death_age() const { return death_age_; }
        void set_death_age(std::int32_t p) { death_age_ = p; }

        std::int32_t endurance() const { return endurance_; }
        void set_endurance(std::int32_t p) { endurance_ = p; }

        std::int32_t affected() const { return affected_; }
        void set_affected(std::int32_t p) { affected_ = p; }

        std::int16_t lang_bonus() const { return lang_bonus_; }
        void set_lang_bonus(std::int16_t p) { lang_bonus_ = p; }

        std::int32_t hit() const { return hit_; }
        void set_hit(std::int32_t p) { hit_ = p; }

        std::int32_t home() const { return home_; }
        void set_home(std::int32_t p) { home_ = p; }

        std::int32_t rpneeded() const { return rpneeded_; }
        void set_rpneeded(std::int32_t p) { rpneeded_ = p; }

        std::int16_t class_modifier(std::size_t ability) const { return class_modifier_.at(ability); }
        void set_class_modifier(std::size_t ability, std::int16_t p) { class_modifier_.at(ability) = p; }

        std::int16_t attr_modifier(std::size_t attr) const { return attr_mod_.at(attr); }
        void set_attr_modifier(std::size_t attr, std::int16_t p) { attr_mod_.at(attr) = p; }

        void save() const;
        void load(FILE *fp);
        static bool load_race_file(const std::string &racefile);
        static void fwrite_race_list();
        static void load_races();
};

using RaceList = std::vector<std::shared_ptr<RaceData>>;
extern RaceList races;
