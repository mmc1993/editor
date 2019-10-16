#include "component.h"
#include "comp_transform.h"
#include "comp_sprite.h"
#include "../property/ui_property.h"

static const std::map<std::string, SharePtr<Component> (*)()> s_ComponentMap = {
    std::make_pair("Transform", []() { return CastPtr<Component>(std::create_ptr<CompTransform>()); }),
    std::make_pair("Sprite",    []() { return CastPtr<Component>(std::create_ptr<CompSprite>());    })
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
        case interface::Serializer::StringValueTypeEnum::kInt:
            result.push_back(std::create_ptr<UIPropertyInt>(*(int *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kBool:
            result.push_back(std::create_ptr<UIPropertyBool>(*(bool *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kFloat:
            result.push_back(std::create_ptr<UIPropertyFloat>(*(float *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kString:
            result.push_back(std::create_ptr<UIPropertyString>(*(std::string *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kVector2:
            result.push_back(std::create_ptr<UIPropertyVector2>(*(glm::vec2 *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kVector3:
            result.push_back(std::create_ptr<UIPropertyVector3>(*(glm::vec3 *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kVector4:
            result.push_back(std::create_ptr<UIPropertyVector4>(*(glm::vec4 *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kColor4:
            result.push_back(std::create_ptr<UIPropertyColor4>(*(glm::vec4 *)property.mMember, property.mName, modifyfunc)); break;
        case interface::Serializer::StringValueTypeEnum::kAsset:
            result.push_back(std::create_ptr<UIPropertyAsset>(*(std::string *)property.mMember, property.mName, modifyfunc)); break;
        }
    }
    return std::move(result);
}

const std::vector<glm::vec2> & Component::GetTrackPoints()
{
    return _trackPoints;
}

void Component::ModifyTrackPoint(const size_t index, const glm::vec2 & point) 
{ 
    if (_state & kModifyTrackPoint)
    {
        _trackPoints.at(index) = point;
        OnModifyTrackPoint(index, point);
    }
}

void Component::InsertTrackPoint(const size_t index, const glm::vec2 & point) 
{
    if (_state & kInsertTrackPoint)
    {
        _trackPoints.insert(std::next(_trackPoints.begin(), index), point);
        OnInsertTrackPoint(index, point);
    }
}

void Component::DeleteTrackPoint(const size_t index, const glm::vec2 & point) 
{
    if (_state & kDeleteTrackPoint) 
    { 
        _trackPoints.erase(std::next(_trackPoints.begin(), index));
        OnDeleteTrackPoint(index, point);
    }
}
