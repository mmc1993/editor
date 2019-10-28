#include "comp_transform.h"

CompTransform::CompTransform()
    : _angle(0      )
    , _scale(1, 1   )
    , _position(0, 0)
{
    _trackPoints.push_back(glm::vec2( 10,  10));
    _trackPoints.push_back(glm::vec2(-10,  10));
    _trackPoints.push_back(glm::vec2(-10, -10));
    _trackPoints.push_back(glm::vec2( 10, -10));
    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kUpdate,           true);
}

CompTransform::~CompTransform()
{ }

void CompTransform::OnAdd()
{ }

void CompTransform::OnDel()
{ }

void CompTransform::OnUpdate(UIObjectGLCanvas * canvas, float dt)
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
    AddState(StateEnum::kUpdate, true);
}

void CompTransform::Angle(float r)
{
    _angle = r;
    AddState(StateEnum::kUpdate, true);
}

void CompTransform::Scale(float x, float y)
{
    _scale.x = x;
    _scale.y = y;
    AddState(StateEnum::kUpdate, true);
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

const glm::mat4 & CompTransform::GetMatrix()
{
    UpdateMatrix();
    return _matrix;
}

glm::mat4 CompTransform::GetMatrixFromRoot()
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

glm::mat4 CompTransform::GetRotateFromRoot()
{
    auto matrix = GetAngleMatrix();
    auto parent = GetOwner()->GetParent();
    while (parent != nullptr)
    {
        matrix = parent->GetTransform()->GetAngleMatrix() * matrix;
        parent = parent->GetParent();
    }
    return matrix;
}

glm::vec2 CompTransform::GetWorldPosition()
{
    return glm::vec3(GetMatrixFromRoot() * glm::vec4(0, 0, 0, 1));
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
    auto props = Component::CollectProperty();
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2, "Poosition", &_position);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2, "Scale"    , &_scale);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat  , "Angle"    , &_angle);
    return std::move(props);
}

bool CompTransform::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    std::cout << "Title " << title << std::endl;
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompTransform::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{ 
    if (index == 0)
    {
        _position.x = point.x + 10;
        _position.y = point.y + 10;
        AddState(StateEnum::kUpdate, true);
    }
}

void CompTransform::UpdateMatrix()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        auto t = glm::translate(glm::mat4(1), glm::vec3(_position, 0));
        auto s = glm::scale(glm::mat4(1), glm::vec3(_scale, 1));
        auto r = glm::rotate(glm::mat4(1), _angle, glm::vec3(0, 0, 1));
        _matrix = t * r * s;
    }
}
