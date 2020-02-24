#include "component.h"
#include "comp_transform.h"
#include "../editor_sys/component/comp_collapse_terrain.h"
#include "../editor_sys/component/comp_sprite_diffuse.h"
#include "../editor_sys/component/comp_field_of_view.h"
#include "../editor_sys/component/comp_render_target.h"
#include "../editor_sys/component/comp_sprite_shader.h"
#include "../editor_sys/component/comp_sprite_bezier.h"
#include "../editor_sys/component/comp_noise_perlin.h"
#include "../editor_sys/component/comp_lightning.h"
#include "../editor_sys/component/comp_tilemap.h"
#include "../editor_sys/component/comp_segment.h"
#include "../editor_sys/component/comp_polygon.h"
#include "../editor_sys/component/comp_sprite.h"
#include "../editor_sys/component/comp_light.h"
#include "../editor_sys/component/comp_text.h"
#include "../editor_sys/property/ui_property.h"

#define COMP_MAP_FUN(Key, Type) std::make_pair(Key, []() { return CastPtr<Component>(std::create_ptr<Type>()); }),

static const std::map<std::string, SharePtr<Component> (*)()> s_ComponentMap = {
    COMP_MAP_FUN("CollapseTerrain", CompCollapseTerrain )
    COMP_MAP_FUN("SpriteDiffuse",   CompSpriteDiffuse   )
    COMP_MAP_FUN("RenderTarget",    CompRenderTarget    )
    COMP_MAP_FUN("SpriteShader",    CompSpriteShader    )
    COMP_MAP_FUN("SpriteBezier",    CompSpriteBezier    )
    COMP_MAP_FUN("NoisePerlin",     CompNoisePerlin     )
    COMP_MAP_FUN("FieldOfView",     CompFieldOfView     )
    COMP_MAP_FUN("Transform",       CompTransform       )
    COMP_MAP_FUN("Lightning",       CompLightning       )
    COMP_MAP_FUN("Polygon",         CompPolygon         )
    COMP_MAP_FUN("Tilemap",         CompTilemap         )
    COMP_MAP_FUN("Segment",         CompSegment         )
    COMP_MAP_FUN("Sprite",          CompSprite          )
    COMP_MAP_FUN("Light",           CompLight           )
    COMP_MAP_FUN("Text",            CompText            )
};

SharePtr<Component> Component::Create(const std::string & name)
{
    auto it = s_ComponentMap.find(name);
    ASSERT_LOG(it != s_ComponentMap.end(), name.c_str());
    return it->second();
}

void Component::OnAppend()
{ 
    AddState(StateEnum::kUpdate, true);
}

void Component::OnDelete()
{ }

void Component::OnStart(UIObjectGLCanvas * canvas)
{ }

void Component::OnLeave(UIObjectGLCanvas * canvas)
{ }

void Component::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{ }

void Component::EncodeBinary(std::ostream & os, Project * project)
{ 
    tools::Serialize(os, mState);
}

void Component::DecodeBinary(std::istream & is, Project * project)
{ 
    tools::Deserialize(is, mState);
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
    return mTrackPoints;
}

std::vector<Component::Property> Component::CollectProperty()
{
    return {
        {UIParser::StringValueTypeEnum::kFlag, "Active", &mState, (uint)StateEnum::kActive},
    };
}

