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
}

void CompSprite::DecodeBinary(std::ifstream & is)
{
}

bool CompSprite::OnModifyProperty(const std::any & value, const std::any & backup, const std::string & title)
{
    return false;
}

std::vector<Component::Property> CompSprite::CollectProperty()
{
    return std::vector<Property>();
}
