#include "comp_transform.h"
#include "../../tools/parser_tool.h"

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

bool CompTransform::ParseProperty(const std::string & key, const std::string & val)
{
    PARSER_REG_MEMBER(tools::ValueParser::kFLOAT, key, val, Angle,  _angle);
    PARSER_REG_MEMBER(tools::ValueParser::kVEC2,  key, val, Scale,  _scale);
    PARSER_REG_MEMBER(tools::ValueParser::kVEC2,  key, val, Position, _position);
    return false;
}

std::vector<Component::Property> CompTransform::CollectProperty()
{
    return {
        { "Poosition", tools::ValueParser::kVEC2, &_position },
        { "Scale",     tools::ValueParser::kVEC2,  &_scale },
        { "Angle",     tools::ValueParser::kFLOAT, &_angle }
    };
}

bool CompTransform::OnModifyProperty(const std::any & value, const std::string & title, const std::any & backup)
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
