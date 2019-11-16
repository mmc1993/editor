#pragma once

#include "component.h"
#include "comp_polygon.h"
#include "../../interface/render.h"

class CompFieldOfView : public Component {
public:
    CompFieldOfView();

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
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
    glm::vec2 RayPoint(const std::vector<glm::vec2> & segments, const glm::vec2 & point, glm::vec2 * next);
    glm::vec2 RayPoint(const std::vector<glm::vec2> & segments, const glm::vec2 & point);

private:
    std::string                         _url;
    glm::vec4                           _color;
    float                               _sceneW;
    float                               _sceneH;

    SharePtr<GLMesh>                    _mesh;
    SharePtr<GLProgram>                 _program;
    std::vector<glm::vec2>              _segments;
    std::vector<SharePtr<CompPolygon>>  _polygons;
};