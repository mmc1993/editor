#include "comp_transform.h"

CompTransform::CompTransform()
    : mAngle(0      )
    , mScale(1, 1   )
    , mPosition(0, 0)
{
    mTrackPoints.push_back(glm::vec2(-10, -10));
    mTrackPoints.push_back(glm::vec2( 10, -10));
    mTrackPoints.push_back(glm::vec2( 10,  10));
    mTrackPoints.push_back(glm::vec2(-10,  10));
    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kUpdate,           true);
}

CompTransform::~CompTransform()
{ }

void CompTransform::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    UpdateMatrix();
}

void CompTransform::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mAngle);
    tools::Serialize(os, mScale);
    tools::Serialize(os, mPosition);
}

void CompTransform::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mAngle);
    tools::Deserialize(is, mScale);
    tools::Deserialize(is, mPosition);
}

void CompTransform::Position(float x, float y)
{
    mPosition.x = x;
    mPosition.y = y;
    AddState(StateEnum::kUpdate, true);
}

void CompTransform::Angle(float r)
{
    mAngle = r;
    AddState(StateEnum::kUpdate, true);
}

void CompTransform::Scale(float x, float y)
{
    mScale.x = x;
    mScale.y = y;
    AddState(StateEnum::kUpdate, true);
}

CompTransform & CompTransform::AddPosition(float x, float y)
{
    Position(mPosition.x + x, mPosition.y + y);
    return *this;
}

CompTransform & CompTransform::AddScale(float x, float y)
{
    Scale(mScale.x + x, mScale.y + y);
    return *this;
}

CompTransform & CompTransform::AddAngle(float r)
{
    Angle(mAngle + r);
    return *this;
}

const glm::vec2 & CompTransform::GetPosition() const
{
    return mPosition;
}

const glm::vec2 & CompTransform::GetScale() const
{
    return mScale;
}

float CompTransform::GetAngle() const
{
    return mAngle;
}

const glm::mat4 & CompTransform::GetMatrix()
{
    UpdateMatrix();
    return mMatrix;
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
    return glm::rotate(glm::mat4(1), glm::radians(mAngle), glm::vec3(0, 0, 1));
}

const std::string & CompTransform::GetName()
{
    static const std::string name = "Transform";
    return name;
}

std::vector<Component::Property> CompTransform::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Poosition", &mPosition);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Scale"    , &mScale);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat  , "Angle"    , &mAngle);
    return std::move(props);
}

bool CompTransform::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompTransform::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{ }

void CompTransform::UpdateMatrix()
{
    static const auto ZAxis = glm::vec3(0, 0, 1);
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        mMatrix = glm::translate(glm::mat4(1), glm::vec3(mPosition, 0));
        mMatrix = glm::rotate(mMatrix, glm::radians(mAngle), ZAxis);
        mMatrix = glm::scale(mMatrix, glm::vec3(mScale, 1));
    }
}
