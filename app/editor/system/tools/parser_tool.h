#pragma once

#include "../../include.h"

namespace tools {
    class ValueParser {
    public:
        enum TypeEnum {
            kINT,
            kBOOL,
            kFLOAT,
            kSTRING,
            kMENU,
            kVEC2,
            kVEC3,
            kVEC4,
            kCOLOR,
            kINT_LIST,
            kFLOAT_LIST,
            kSTRING_LIST,
        };

        static bool Parse(TypeEnum type, const std::string & val, void * out)
        {
            switch (type)
            {
            case ValueParser::kINT:
                *((int *)out) = std::stoi(val);
                return true;
            case ValueParser::kBOOL:
                *((bool *)out) = val.at(0) == 'o' && val.at(1) == 'k';
                return true;
            case ValueParser::kFLOAT:
                *((float *)out) = std::stof(val);
                return true;
            case ValueParser::kSTRING:
                *((std::string *)out) = val;
                return true;
            case ValueParser::kMENU:
                {
                    auto split = tools::Split(val, ",");
                    auto value = (std::vector<std::string> *)out;
                    *value = std::move(split);
                }
                return true;
            case ValueParser::kVEC2:
                {
                    auto split = tools::Split(val, " ");
                    auto value = (glm::vec2 *)out;
                    value->x = std::stof(split.at(0));
                    value->y = std::stof(split.at(1));
                }
                return true;
            case ValueParser::kVEC3:
                {
                    auto split = tools::Split(val, " ");
                    auto value = (glm::vec3 *)out;
                    value->x = std::stof(split.at(0));
                    value->y = std::stof(split.at(1));
                    value->z = std::stof(split.at(2));
                }
                return true;
            case ValueParser::kVEC4:
                {
                    auto split = tools::Split(val, " ");
                    auto value = (glm::vec4 *)out;
                    value->x = std::stof(split.at(0));
                    value->y = std::stof(split.at(1));
                    value->z = std::stof(split.at(2));
                    value->w = std::stof(split.at(3));
                }
                return true;
            case TypeEnum::kCOLOR:
                {
                    auto split = tools::Split(val, " ");
                    auto value = (glm::vec4 *)out;
                    value->x = std::stof(split.at(0));
                    value->y = std::stof(split.at(1));
                    value->z = std::stof(split.at(2));
                    if (split.size() == 4)
                    {
                        value->w = std::stof(split.at(3));
                    }
                }
                return true;
            case ValueParser::kINT_LIST:
                {
                    auto split = tools::Split(val, " ");
                    auto value = (std::vector<int> *)out;
                    std::transform(
                        split.begin(), split.end(),
                        std::back_inserter(*value),
                        [](const std::string & val)
                        { return std::stoi(val); });
                }
                return true;
            case ValueParser::kFLOAT_LIST:
                {
                    auto split = tools::Split(val, " ");
                    auto value = (std::vector<float> *)out;
                    std::transform(
                        split.begin(), split.end(),
                        std::back_inserter(*value),
                        [](const std::string & val)
                        { return std::stof(val); });
                }
                return true;
            case ValueParser::kSTRING_LIST:
                {
                    auto split = tools::Split(val, " ");
                    auto value = (std::vector<std::string> *)out;
                    *value = std::move(split);
                }
                return true;
            }
            return false;
        }
    };
}

#ifndef PARSER_REG_MEMBER
#define PARSER_REG_MEMBER(type, key, val, K, out)  if (key == #K) { tools::ValueParser::Parse(type, val, &out); return ; }
#endif