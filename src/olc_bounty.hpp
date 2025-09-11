
#pragma once

#include <vector>
#include <memory>
#include <array>
#include <string_view>
#include "mud.hpp"


enum class OlcBountyType : int {
        Alive = 0,
        Dead = 1,
        MaxType = 2
};

constexpr std::array<std::string_view, 3> olc_bounty_types = {
        "Alive", "Dead", "MAX_TYPE"
};

constexpr std::string_view OLC_BOUNTY_FILE = SYSTEM_DIR "olcbounty.dat";


class OLC_BOUNTY_DATA {
private:
        int _owner{0};
        int _vnum{0};
        int _corpse{0};
        OlcBountyType _type{OlcBountyType::Alive};
        int _amount{0};
        int _exp{0};

public:
        OLC_BOUNTY_DATA() = default;
        explicit OLC_BOUNTY_DATA(int vnum);
        ~OLC_BOUNTY_DATA() = default;

        OlcBountyType type() const { return _type; }
        bool set_type(OlcBountyType t) {
                if (t == OlcBountyType::MaxType) return false;
                _type = t;
                return true;
        }

        int owner() const { return _owner; }
        bool set_owner(int vnum);

        int vnum() const { return _vnum; }
        bool set_vnum(int vnum);

        int corpse() const { return _corpse; }
        bool set_corpse(int vnum);

        int amount() const { return _amount; }
        void set_amount(int credits) { _amount = credits; }

        int experience() const { return _exp; }
        void set_experience(int experience) { _exp = experience; }

        void save() const;
        void load(FILE* fp);
        static void load_olc_bounties();
};



using OLC_BOUNTY_LIST = std::vector<std::shared_ptr<OLC_BOUNTY_DATA>>;
extern OLC_BOUNTY_LIST olc_bounties;

std::shared_ptr<OLC_BOUNTY_DATA> has_olc_bounty(const CharData* victim);
void mset_bounty(CharData* ch, CharData* mob, std::string_view argument);
void print_olc_bounties_mob(CharData* ch, CharData* mob);
void load_olc_bounties();
bool check_given_bounty(CharData* ch, CharData* hunter, const ObjData* obj);
int print_olc_bounties(CharData* ch);
bool check_olc_bounties(const RoomIndexData* room);


