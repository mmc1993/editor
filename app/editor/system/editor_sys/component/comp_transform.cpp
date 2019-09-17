#include "comp_transform.h"

CompTransform::CompTransform()
    : _isChange(true)
    , _angle(0)
    , _scale(1, 1)
    , _translate(0, 0)
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

void CompTransform::Translate(float x, float y)
{
    _translate.x = x;
    _translate.y = y;
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

CompTransform & CompTransform::AddTranslate(float x, float y)
{
    Translate(_translate.x + x, _translate.y + y);
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
    return _translate;
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
    auto matrix = GetAngleQuat();
    auto parent = GetOwner()->GetParent();
    while (parent != nullptr)
    {
        matrix = parent->GetTransform()->GetAngleQuat() * matrix;
        parent = parent->GetParent();
    }
    return (glm::mat3)matrix;
}

glm::vec2 CompTransform::GetWorldPosition()
{
    return glm::vec3(GetMatrixFromRoot() * glm::vec3(0, 0, 1));
}

glm::quat CompTransform::GetAngleQuat()
{
    return glm::angleAxis(_angle, glm::vec3(0, 0, 1));
}

void CompTransform::UpdateMatrix()
{
    if (_isChange)
    {
        auto t = glm::translate(glm::mat4(1), glm::vec3(_translate, 0));
        auto s = glm::scale(glm::mat4(1), glm::vec3(_scale, 1));
        auto r = glm::angleAxis(_angle, glm::vec3(0, 0, 1));
        _matrix = t * (glm::mat4)r * s;
        _isChange = false;
    }
}
