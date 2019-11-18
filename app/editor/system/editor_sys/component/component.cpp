#include "component.h"
#include "comp_field_of_view.h"
#include "comp_layer_render.h"
#include "comp_transform.h"
#include "comp_lightning.h"
#include "comp_tilemap.h"
#include "comp_segment.h"
#include "comp_polygon.h"
#include "comp_sprite.h"
#include "comp_light.h"
#include "../property/ui_property.h"

static const std::map<std::string, SharePtr<Component> (*)()> s_ComponentMap = {
    std::make_pair("FieldOfView",   []() { return CastPtr<Component>(std::create_ptr<CompFieldOfView>());   }),
    std::make_pair("LayerRender",   []() { return CastPtr<Component>(std::create_ptr<CompLayerRender>());   }),
    std::make_pair("Transform",     []() { return CastPtr<Component>(std::create_ptr<CompTransform>());     }),
    std::make_pair("Lightning",     []() { return CastPtr<Component>(std::create_ptr<CompLightning>());     }),
    std::make_pair("Polygon",       []() { return CastPtr<Component>(std::create_ptr<CompPolygon>());       }),
    std::make_pair("Tilemap",       []() { return CastPtr<Component>(std::create_ptr<CompTilemap>());       }),
    std::make_pair("Segment",       []() { return CastPtr<Component>(std::create_ptr<CompSegment>());       }),
    std::make_pair("Sprite",        []() { return CastPtr<Component>(std::create_ptr<CompSprite>());        }),
    std::make_pair("Light",         []() { return CastPtr<Component>(std::create_ptr<CompLight>());         }),

};

SharePtr<Component> Component::Create(const std::string & name)
{
    auto it = s_ComponentMap.find(name);
    ASSERT_LOG(it != s_ComponentMap.end(), name.c_str());
    return it->second();
}

void Component::OnAdd()
{ 
    AddState(StateEnum::kUpdate, true);
}

void Component::OnDel()
{ }

void Component::OnStart(UIObjectGLCanvas * canvas)
{ }

void Component::OnLeave(UIObjectGLCanvas * canvas)
{ }

void Component::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{ }

void Component::EncodeBinary(std::ofstream & os)
{ 
    tools::Serialize(os, _state);
}

void Component::DecodeBinary(std::ifstream & is)
{ 
    tools::Deserialize(is, _state);
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
        case interface::Serializer::StringValueTypeEnum::kFlag:
            result.push_back(std::create_ptr<UIPropertyFlag>(*(uint *)property.mMember, property.mName, modifyfunc, std::any_cast<uint>(property.mExtend))); break;
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

std::vector<Component::Property> Component::CollectProperty()
{
    return {
        {interface::Serializer::StringValueTypeEnum::kFlag, "Active", &_state, (uint)StateEnum::kActive},
    };
}

