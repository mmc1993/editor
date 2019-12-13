#pragma once

#include "component.h"

class CompTransform : public Component {
public:
    CompTransform();
    ~CompTransform();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;

    void Position(float x, float y);
    void Scale(float x, float y);
    void Angle(float r);

    CompTransform & AddPosition(float x, float y);
    CompTransform & AddScale(float x, float y);
    CompTransform & AddAngle(float r);

    const glm::vec2 & GetPosition() const;
    const glm::vec2 & GetScale() const;
    float GetAngle() const;

    const glm::mat4 & GetMatrix();
    glm::mat4 GetMatrixFromRoot();
    glm::mat4 GetRotateFromRoot();
    glm::vec2 GetWorldPosition();
    glm::mat4 GetAngleMatrix();

    virtual const std::string & GetName() override;
    
    virtual bool OnModifyProperty(const std::any & oldValue, 
                                  const std::any & newValue,
                                  const std::string & title) override;

    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void UpdateMatrix();

private:
    float     _angle;
    glm::vec2 _scale;
    glm::mat4 _matrix;
    glm::vec2 _position;
};