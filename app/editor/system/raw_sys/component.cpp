#include "component.h"
#include "comp_transform.h"
#include "../editor_sys/component/comp_field_of_view.h"
#include "../editor_sys/component/comp_render_target.h"
#include "../editor_sys/component/comp_lightning.h"
#include "../editor_sys/component/comp_tilemap.h"
#include "../editor_sys/component/comp_segment.h"
#include "../editor_sys/component/comp_polygon.h"
#include "../editor_sys/component/comp_sprite.h"
#include "../editor_sys/component/comp_light.h"
#include "../editor_sys/component/comp_text.h"
#include "../editor_sys/property/ui_property.h"

static const std::map<std::string, SharePtr<Component> (*)()> s_ComponentMap = {
    std::make_pair("RenderTarget",  []() { return CastPtr<Component>(std::create_ptr<CompRenderTarget>());  }),
    std::make_pair("FieldOfView",   []() { return CastPtr<Component>(std::create_ptr<CompFieldOfView>());   }),
    std::make_pair("Transform",     []() { return CastPtr<Component>(std::create_ptr<CompTransform>());     }),
    std::make_pair("Lightning",     []() { return CastPtr<Component>(std::create_ptr<CompLightning>());     }),
    std::make_pair("Polygon",       []() { return CastPtr<Component>(std::create_ptr<CompPolygon>());       }),
    std::make_pair("Tilemap",       []() { return CastPtr<Component>(std::create_ptr<CompTilemap>());       }),
    std::make_pair("Segment",       []() { return CastPtr<Component>(std::create_ptr<CompSegment>());       }),
    std::make_pair("Sprite",        []() { return CastPtr<Component>(std::create_ptr<CompSprite>());        }),
    std::make_pair("Light",         []() { return CastPtr<Component>(std::create_ptr<CompLight>());         }),
    std::make_pair("Text",          []() { return CastPtr<Component>(std::create_ptr<CompText>());          }),
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

void Component::EncodeBinary(std::ostream & os, Project * project)
{ 
    tools::Serialize(os, _state);
}

void Component::DecodeBinary(std::istream & is, Project * project)
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
        case UIParser::StringValueTypeEnum::kInt:
            result.push_back(std::create_ptr<UIPropertyInt>(*(int *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kBool:
            result.push_back(std::create_ptr<UIPropertyBool>(*(bool *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kFlag:
            result.push_back(std::create_ptr<UIPropertyFlag>(*(uint *)property.mMember, property.mName, modifyfunc, property.mExtend)); break;
        case UIParser::StringValueTypeEnum::kFloat:
            result.push_back(std::create_ptr<UIPropertyFloat>(*(float *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kString:
            result.push_back(std::create_ptr<UIPropertyString>(*(std::string *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kVector2:
            result.push_back(std::create_ptr<UIPropertyVector2>(*(glm::vec2 *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kVector3:
            result.push_back(std::create_ptr<UIPropertyVector3>(*(glm::vec3 *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kVector4:
            result.push_back(std::create_ptr<UIPropertyVector4>(*(glm::vec4 *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kColor4:
            result.push_back(std::create_ptr<UIPropertyColor4>(*(glm::vec4 *)property.mMember, property.mName, modifyfunc)); break;
        case UIParser::StringValueTypeEnum::kAsset:
            result.push_back(std::create_ptr<UIPropertyAsset>(*(Res::Ref *)property.mMember, property.mName, modifyfunc, property.mExtend)); break;
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
        {UIParser::StringValueTypeEnum::kFlag, "Active", &_state, (uint)StateEnum::kActive},
    };
}

