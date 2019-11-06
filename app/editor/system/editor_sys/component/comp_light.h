#pragma once

#include "component.h"
#include "../../interface/render.h"

class CompLight : public Component {
private:
    enum UpdateEnum {
        kTrackPoint = 1,
        kBorder     = 2,
    };

public:
    CompLight();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
    virtual bool OnModifyProperty(const std::any & oldValue, 
                                  const std::any & newValue, 
                                  const std::string & title) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Update();
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnInsertTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnDeleteTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    uint                _update;
    float               _border;
    glm::vec4           _color;
    SharePtr<GLProgram> _program;
    SharePtr<GLMesh>    _mesh;
};