#include "comp_sprite.h"

void CompSprite::OnAdd()
{ }

void CompSprite::OnDel()
{ }

void CompSprite::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{ }

const std::string & CompSprite::GetName()
{
    static const std::string name = "Sprite";
    return name;
}

void CompSprite::EncodeBinary(std::ofstream & os)
{
    tools::Serialize(os, _url);
    tools::Serialize(os, _size);
    tools::Serialize(os, _anchor);
}

void CompSprite::DecodeBinary(std::ifstream & is)
{
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _size);
    tools::Deserialize(is, _anchor);
}

bool CompSprite::OnModifyProperty(const std::any & value, const std::any & backup, const std::string & title)
{
    return true;
}

std::vector<Component::Property> CompSprite::CollectProperty()
{
    return {
        { interface::Serializer::StringValueTypeEnum::kVEC2, "Url",    &_url    },
        { interface::Serializer::StringValueTypeEnum::kVEC2, "Size",   &_size   },
        { interface::Serializer::StringValueTypeEnum::kVEC2, "Anchor", &_anchor }
    };
}
