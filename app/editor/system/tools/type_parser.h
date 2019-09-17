#pragma once

class TypeParser {
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

    virtual bool Parse(TypeEnum type, const std::string & buffer, void * output)
    {
        switch (type)
        {
        case TypeParser::kINT:
            *((int *)output) = std::stoi(buffer);
            return true;
        case TypeParser::kBOOL:
            *((bool *)output) = std::stoi(buffer) != 0;
            return true;
        case TypeParser::kFLOAT:
            *((float *)output) = std::stof(buffer);
            return true;
        case TypeParser::kSTRING:
            *((std::string *)output) = buffer;
            return true;
        case TypeParser::kMENU:
            {
                auto split = tools::Split(buffer, ",");
                auto value = (std::vector<std::string> *)output;
                *value = std::move(split);
            }
            return true;
        case TypeParser::kVEC2:
            {
                auto split = tools::Split(buffer, " ");
                auto value = (glm::vec2 *)output;
                value->x = std::stof(split.at(0));
                value->y = std::stof(split.at(1));
            }
            return true;
        case TypeParser::kVEC3:
            {
                auto split = tools::Split(buffer, " ");
                auto value = (glm::vec3 *)output;
                value->x = std::stof(split.at(0));
                value->y = std::stof(split.at(1));
                value->z = std::stof(split.at(2));
            }
            return true;
        case TypeParser::kVEC4:
            {
                auto split = tools::Split(buffer, " ");
                auto value = (glm::vec4 *)output;
                value->x = std::stof(split.at(0));
                value->y = std::stof(split.at(1));
                value->z = std::stof(split.at(2));
                value->w = std::stof(split.at(3));
            }
            return true;
        case TypeEnum::kCOLOR:
            {
                auto split = tools::Split(buffer, " ");
                auto value = (glm::vec4 *)output;
                value->x = std::stof(split.at(0));
                value->y = std::stof(split.at(1));
                value->z = std::stof(split.at(2));
                if (split.size() == 4)
                {
                    value->w = std::stof(split.at(3));
                }
            }
            return true;
        case TypeParser::kINT_LIST:
            {
                auto split = tools::Split(buffer, " ");
                auto value = (std::vector<int> *)output;
                std::transform(
                    split.begin(), split.end(), 
                    std::back_inserter(*value), 
                    [] (const std::string & val)
                    { return std::stoi(val); });
            }
            return true;
        case TypeParser::kFLOAT_LIST:
            {
                auto split = tools::Split(buffer, " ");
                auto value = (std::vector<float> *)output;
                std::transform(
                    split.begin(), split.end(), 
                    std::back_inserter(*value), 
                    [] (const std::string & val)
                    { return std::stof(val); });
            }
            return true;
        case TypeParser::kSTRING_LIST:
            {
                auto split = tools::Split(buffer, " ");
                auto value = (std::vector<std::string> *)output;
                *value = std::move(split);
            }
            return true;
        }
        return false;
    }
};