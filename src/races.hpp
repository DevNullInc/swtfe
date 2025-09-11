

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

constexpr std::string_view FILE_RACE_LIST = "race.lst";
constexpr std::string_view RACES_DIR = "../races/";

enum class RaceAttr : int {
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


class RaceData {
private:
        ExtBV body_parts_;
        std::shared_ptr<LANGUAGE_DATA> language_;
        int affected_{0};
        int con_plus_{0};
        int cha_plus_{0};
        int dex_plus_{0};
        int frc_plus_{0};
        int hit_{0};
        int int_plus_{0};
        int lang_bonus_{0};
        int lck_plus_{0};
        int endurance_{0};
        std::string name_;
        int str_plus_{0};
        int wis_plus_{0};
        std::array<int, MaxAbility> class_modifier_{};
        std::array<int, static_cast<int>(RaceAttr::MaxAttr)> attr_mod_{};
        int home_{0};
        int death_age_{0};
        int hunger_mod_{0};
        int thirst_mod_{0};
        int rpneeded_{0};
        int start_age_{0};
        int class_restriction_{0};

public:
        RaceData() = default;
        ~RaceData() = default;

        std::shared_ptr<LANGUAGE_DATA> language() const { return language_; }
        void set_language(std::shared_ptr<LANGUAGE_DATA> p) { language_ = std::move(p); }

        ExtBV& body_parts() { return body_parts_; }
        void set_body_parts(int bit, bool set);
        void toggle_body_parts(int bit);

        const std::string& name() const { return name_; }
        void set_name(const std::string& newname) { name_ = newname; }

        int class_restriction() const { return class_restriction_; }
        void set_class_restriction(int p) { class_restriction_ = p; }

        int start_age() const { return start_age_; }
        void set_start_age(int p) { start_age_ = p; }

        int thirst_mod() const { return thirst_mod_; }
        void set_thirst_mod(int p) { thirst_mod_ = p; }

        int hunger_mod() const { return hunger_mod_; }
        void set_hunger_mod(int p) { hunger_mod_ = p; }

        int death_age() const { return death_age_; }
        void set_death_age(int p) { death_age_ = p; }

        int endurance() const { return endurance_; }
        void set_endurance(int p) { endurance_ = p; }

        int affected() const { return affected_; }
        void set_affected(int p) { affected_ = p; }

        int lang_bonus() const { return lang_bonus_; }
        void set_lang_bonus(int p) { lang_bonus_ = p; }

        int hit() const { return hit_; }
        void set_hit(int p) { hit_ = p; }

        int home() const { return home_; }
        void set_home(int p) { home_ = p; }

        int rpneeded() const { return rpneeded_; }
        void set_rpneeded(int p) { rpneeded_ = p; }

        int class_modifier(int ability) const { return class_modifier_.at(ability); }
        void set_class_modifier(int ability, int p) { class_modifier_.at(ability) = p; }

        int attr_modifier(int attr) const { return attr_mod_.at(attr); }
        void set_attr_modifier(int attr, int p) { attr_mod_.at(attr) = p; }

        void save() const;
        void load(FILE* fp);
        static bool load_race_file(const std::string& racefile);
        static void fwrite_race_list();
        static void load_races();
};

using RACE_LIST = std::vector<std::shared_ptr<RaceData>>;
extern RACE_LIST races;


