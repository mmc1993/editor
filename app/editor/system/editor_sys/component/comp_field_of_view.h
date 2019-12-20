#pragma once

#include "../../raw_sys/component.h"
#include "comp_render_target.h"
#include "comp_polygon.h"

class CompFieldOfView : public Component {
public:
    CompFieldOfView();

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
    virtual bool OnModifyProperty(const std::any & oldValue,
                                  const std::any & newValue,
                                  const std::string & title) override;
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Update();
    void GenView();
    void GenMesh();
    glm::vec2 RayTracking(const std::vector<glm::vec2> & segments, const glm::vec2 & point);
    glm::vec2 RayExtended(const std::vector<glm::vec2> & segments, const glm::vec2 & point);
    void OnDrawCallback(const interface::RenderCommand & command, uint texturePos);

private:
    glm::vec4   _color;
    Res::Ref    _clipObject;
    Res::Ref    _polyObject;

    SharePtr<RawMesh>      _mesh;
    SharePtr<RawProgram>   _program;

    std::vector<glm::vec2> _rayPoints;
};