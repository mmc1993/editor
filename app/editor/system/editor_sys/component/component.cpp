#include "component.h"
#include "comp_transform.h"
#include "../property/ui_property.h"

static const std::map<std::string, SharePtr<Component> (*)()> s_ComponentMap = {
    std::make_pair("Transform", []() { return CastPtr<Component>(std::create_ptr<CompTransform>()); })
};

SharePtr<Component> Component::Create(const std::string & name)
{
    auto it = s_ComponentMap.find(name);
    ASSERT_LOG(it != s_ComponentMap.end(), name.c_str());
    return it->second();
}

std::vector<SharePtr<UIObject>> Component::CreateUIPropertys()
{
    auto modifyfunc = std::bind(
        &Component::OnModifyProperty, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    std::vector<SharePtr<UIObject>> result;
    for (const auto & property : CollectProperty())
    {
        switch (property.mType)
        {
        case Interface::Serializer::StringValueTypeEnum::kINT:
            result.push_back(std::create_ptr<UIPropertyInt>(*(int *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kBOOL:
            result.push_back(std::create_ptr<UIPropertyBool>(*(bool *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kFLOAT:
            result.push_back(std::create_ptr<UIPropertyFloat>(*(float *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kSTRING:
            result.push_back(std::create_ptr<UIPropertyString>(*(std::string *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kVEC2:
            result.push_back(std::create_ptr<UIPropertyVector2>(*(glm::vec2 *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kVEC3:
            result.push_back(std::create_ptr<UIPropertyVector3>(*(glm::vec3 *)property.mMember, property.mName, modifyfunc)); break;
        case Interface::Serializer::StringValueTypeEnum::kVEC4:
            result.push_back(std::create_ptr<UIPropertyVector4>(*(glm::vec4 *)property.mMember, property.mName, modifyfunc)); break;
        }
    }
    return std::move(result);
}

const std::vector<glm::vec2> & Component::GetControlPoints()
{
    return _controlPoints;
}

void Component::ModifyControlPoint(const size_t index, const glm::vec2 & point) 
{ 
    if (_state & kModifyCP)
    {
        _controlPoints.at(index) = point;
        OnModifyControlPoint(index, point);
    }
}

void Component::InsertControlPoint(const size_t index, const glm::vec2 & point) 
{
    if (_state & kInsertCP)
    {
        _controlPoints.insert(_controlPoints.begin() + index, point);
        OnInsertControlPoint(index, point);
    }
}

void Component::DeleteControlPoint(const size_t index, const glm::vec2 & point) 
{
    if (_state & kDeleteCP) 
    { 
        _controlPoints.erase(_controlPoints.begin() + index);
        OnDeleteControlPoint(index, point);
    }
}
