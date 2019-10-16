#include "serializer.h"

bool interface::Serializer::FromStringParse(StringValueTypeEnum type, const std::string & val, const std::string & sep, void * out)
{
    switch (type)
    {
    case interface::Serializer::StringValueTypeEnum::kInt:
        {
            *((int *)out) = std::stoi(val);
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kBool:
        {
            *((bool *)out) = val.at(0) == 'o' && val.at(1) == 'k';
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kFloat:
        {
            *((float *)out) = std::stof(val);
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kString:
        {
            *((std::string *)out) = val;
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kVector2:
        {
            auto value = (glm::vec2 *)out;
            auto array = tools::Split(val, sep);
            value->x = std::stof(array.at(0));
            value->y = std::stof(array.at(1));
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kVector3:
        {
            auto value = (glm::vec3 *)out;
            auto array = tools::Split(val, sep);
            value->x = std::stof(array.at(0));
            value->y = std::stof(array.at(1));
            value->z = std::stof(array.at(2));
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kVector4:
        {
            auto value = (glm::vec4 *)out;
            auto array = tools::Split(val, sep);
            value->x = std::stof(array.at(0));
            value->y = std::stof(array.at(1));
            value->z = std::stof(array.at(2));
            value->w = std::stof(array.at(3));
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kIntList:
        {
            auto value = (std::vector<int>*)out;
            auto array = tools::Split(val, sep);
            std::transform(
                array.begin(), array.end(),
                std::back_inserter(*value),
                [](const std::string & val)
                { return std::stoi(val); });
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kFloatList:
        {
            auto value = (std::vector<float> *)out;
            auto array = tools::Split(val, sep);
            std::transform(
                array.begin(), array.end(),
                std::back_inserter(*value),
                [](const std::string & val)
                { return std::stof(val); });
        }
        return true;
    case interface::Serializer::StringValueTypeEnum::kStringList:
        {
            auto value = (std::vector<std::string> *)out;
            auto array = tools::Split(val, sep);
            *value = std::move(array);
        }
        return true;
    }
    return false;
}
