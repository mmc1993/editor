#include "comp_transform.h"

CompTransform::CompTransform()
    : _isChange(true)
    , _angle(0)
    , _scale(1, 1)
    , _position(0, 0)
{ }

CompTransform::~CompTransform()
{ }

void CompTransform::OnAdd()
{ }

void CompTransform::OnDel()
{ }

void CompTransform::OnUpdate(float dt)
{
    UpdateMatrix();
}

void CompTransform::EncodeBinary(std::ofstream & os)
{
    tools::Serialize(os, _angle);
    tools::Serialize(os, _scale);
    tools::Serialize(os, _position);
}

void CompTransform::DecodeBinary(std::ifstream & is)
{
    tools::Deserialize(is, _angle);
    tools::Deserialize(is, _scale);
    tools::Deserialize(is, _position);
}

void CompTransform::Position(float x, float y)
{
    _position.x = x;
    _position.y = y;
    _isChange = true;
}

void CompTransform::Angle(float r)
{
    _angle = r;
    _isChange = true;
}

void CompTransform::Scale(float x, float y)
{
    _scale.x = x;
    _scale.y = y;
    _isChange = true;
}

CompTransform & CompTransform::AddPosition(float x, float y)
{
    Position(_position.x + x, _position.y + y);
    return *this;
}

CompTransform & CompTransform::AddScale(float x, float y)
{
    Scale(_scale.x + x, _scale.y + y);
    return *this;
}

CompTransform & CompTransform::AddAngle(float r)
{
    Angle(_angle + r);
    return *this;
}

const glm::vec2 & CompTransform::GetPosition() const
{
    return _position;
}

const glm::vec2 & CompTransform::GetScale() const
{
    return _scale;
}

float CompTransform::GetAngle() const
{
    return _angle;
}

const glm::mat3 & CompTransform::GetMatrix()
{
    UpdateMatrix();
    return _matrix;
}

glm::mat3 CompTransform::GetMatrixFromRoot()
{
    auto matrix = GetMatrix();
    auto parent = GetOwner()->GetParent();
    while (parent != nullptr)
    {
        matrix = parent->GetTransform()->GetMatrix() * matrix;
        parent = parent->GetParent();
    }
    return matrix;
}

glm::mat3 CompTransform::GetRotateFromRoot()
{
    auto matrix = GetAngleMatrix();
    auto parent = GetOwner()->GetParent();
    while (parent != nullptr)
    {
        matrix = parent->GetTransform()->GetAngleMatrix() * matrix;
        parent = parent->GetParent();
    }
    return (glm::mat3)matrix;
}

glm::vec2 CompTransform::GetWorldPosition()
{
    return glm::vec3(GetMatrixFromRoot() * glm::vec3(0, 0, 1));
}

glm::mat4 CompTransform::GetAngleMatrix()
{
    return glm::rotate(glm::mat4(1), _angle, glm::vec3(0, 0, 1));
}

const std::string & CompTransform::GetName()
{
    static const std::string name = "Transform";
    return name;
}

std::vector<Component::Property> CompTransform::CollectProperty()
{
    return {
        {   Interface::Serializer::StringValueTypeEnum::kVEC2,  "Poosition", &_position  },
        {   Interface::Serializer::StringValueTypeEnum::kVEC2,  "Scale",     &_scale     },
        {   Interface::Serializer::StringValueTypeEnum::kFLOAT, "Angle",     &_angle     }
    };
}

bool CompTransform::OnModifyProperty(const std::any & value, const std::any & backup, const std::string & title)
{
    std::cout << "Title " << title << std::endl;
    return true;
}

void CompTransform::UpdateMatrix()
{
    if (_isChange)
    {
        auto t = glm::translate(glm::mat4(1), glm::vec3(_position, 0));
        auto s = glm::scale(glm::mat4(1), glm::vec3(_scale, 1));
        auto r = GetAngleMatrix();
        _matrix = t * r * s;
        _isChange = false;
    }
}
