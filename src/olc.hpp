#pragma once

#include "mud.hpp"
#include <string>
#include <vector>
#include <memory>
#include <string_view>

constexpr std::string_view ClassesDir = "../classes/";
constexpr std::string_view ProtoshipDir = "../protoships/";
constexpr std::string_view ListDir = "../list/";
constexpr std::string_view IllnessDir = "../illness/";
constexpr std::string_view ProtoshipList = "protoship.lst";
constexpr std::string_view ClassList = "class.lst";
constexpr std::string_view ListList = "list.lst";
constexpr std::string_view LanguageList = "language.lst";
constexpr std::string_view IllnessList = "illness.lst";

class ClassData {
public:
        std::string class_name;
        int value{0};
        std::vector<std::shared_ptr<ClassData>> foils;

        ClassData() = default;
        explicit ClassData(std::string name, int val = 0) : class_name(std::move(name)), value(val) {}
        ~ClassData() = default;
};

class LanguageData {
public:
        std::string name;
        int min_intelligence{0};

        LanguageData() = default;
        explicit LanguageData(std::string language_name, int min_int = 0) : name(std::move(language_name)), min_intelligence(min_int) {}
        ~LanguageData() = default;
};

class ProtoshipData {
public:
        std::string name;
        std::string description;
        std::string shipclass;
        int mingroundspeed{0}, maxgroundspeed{0};
        int minrooms{0}, maxrooms{0};
        int mincomm{0}, maxcomm{0};
        int minsensor{0}, maxsensor{0};
        int minastro_array{0}, maxastro_array{0};
        int minhyperspeed{0}, maxhyperspeed{0};
        int minspeed{0}, maxspeed{0};
        int minmissiles{0}, maxmissiles{0};
        int mintorpedos{0}, maxtorpedos{0};
        int minrockets{0}, maxrockets{0};
        int minlasers{0}, maxlasers{0};
        int mintractorbeam{0}, maxtractorbeam{0};
        int minions{0}, maxions{0};
        int minmanuever{0}, maxmanuever{0};
        int maxcargo{0}, mincargo{0};
        int maxenergy{0}, minenergy{0};
        int minshield{0}, maxshield{0};
        int minhull{0}, maxhull{0};
        int minchaff{0}, maxchaff{0};
        int minbattalions{0}, maxbattalions{0};

        ProtoshipData() = default;
        ~ProtoshipData() = default;
};

class ListData {
public:
        std::string name;
        std::string filename;
        std::string value1, value2, value3, value4, value5;

        ListData() = default;
        ~ListData() = default;
};

class IllnessData {
public:
        std::string name;
        std::string social1, social2, social3;
        std::string message1, message2, message3;
        int hploss{0}, mvloss{0}, hunger{0}, thirst{0};

        IllnessData() = default;
        ~IllnessData() = default;
};

using ClassList = std::vector<std::shared_ptr<ClassData>>;
using LanguageList = std::vector<std::shared_ptr<LanguageData>>;
using ProtoshipList = std::vector<std::shared_ptr<ProtoshipData>>;
using ListList = std::vector<std::shared_ptr<ListData>>;
using IllnessList = std::vector<std::shared_ptr<IllnessData>>;

extern ClassList classes;
extern LanguageList languages;
extern ProtoshipList protoships;
extern ListList lists;
extern IllnessList illnesses;

std::shared_ptr<ProtoshipData> get_protoship(std::string_view name);
