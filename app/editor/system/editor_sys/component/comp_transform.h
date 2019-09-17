#pragma once

#include "component.h"

class CompTransform : public Component {
public:
    CompTransform();
    ~CompTransform();
    virtual void OnAdd() override;
    virtual void OnDel() override;
    virtual void OnUpdate(float dt) override;

    void Translate(float x, float y);
    void Scale(float x, float y);
    void Angle(float r);

    CompTransform & AddTranslate(float x, float y);
    CompTransform & AddScale(float x, float y);
    CompTransform & AddAngle(float r);

    const glm::vec2 & GetPosition() const;
    const glm::vec2 & GetScale() const;
    float GetAngle() const;

    const glm::mat3 & GetMatrix();
    glm::mat3 GetMatrixFromRoot();
    glm::mat3 GetRotateFromRoot();
    glm::vec2 GetWorldPosition();
    glm::quat GetAngleQuat();

private:
    void UpdateMatrix();

private:
    float     _angle;
    glm::vec2 _scale;
    glm::vec2 _translate;
    glm::mat3 _matrix;
    bool _isChange;
};