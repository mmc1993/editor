#include "component.h"
#include "comp_transform.h"
#include "../property/ui_property.h"

static const std::map<std::string, Component *(*)()> ComponentMap = {
    std::make_pair("Transform", &std::create<Component, CompTransform>)
};

Component * Component::Create(const std::string & name)
{
    auto it = ComponentMap.find(name);
    ASSERT_LOG(it != ComponentMap.end(), name.c_str());
    return it->second();
}

std::vector<UIObject *> Component::BuildUIPropertys(Component * component)
{
    auto modifyfunc = std::bind(
        &Component::OnModifyProperty, component,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    std::vector<UIObject *> result;
    for (const auto & property : component->CollectProperty())
    {
        switch (property.mType)
        {
        case Interface::Serializer::StringValueTypeEnum::kINT:
            result.push_back(new UIPropertyInt(*(int *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kBOOL:
            result.push_back(new UIPropertyBool(*(bool *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kFLOAT:
            result.push_back(new UIPropertyFloat(*(float *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kSTRING:
            result.push_back(new UIPropertyString(*(std::string *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kVEC2:
            result.push_back(new UIPropertyVector2(*(glm::vec2 *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kVEC3:
            result.push_back(new UIPropertyVector3(*(glm::vec3 *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kVEC4:
            result.push_back(new UIPropertyVector4(*(glm::vec4 *)property.mMember, property.mName, modifyfunc)); break;
        }
    }
    return std::move(result);
}