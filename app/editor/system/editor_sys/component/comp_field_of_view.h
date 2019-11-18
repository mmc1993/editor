#pragma once

#include "component.h"
#include "comp_polygon.h"
#include "comp_layer_render.h"
#include "../../interface/render.h"

class CompFieldOfView 
    : public Component
    , public std::enable_shared_from_this<CompFieldOfView> {
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
    glm::vec2 RayTracking(const std::vector<glm::vec2> & segments, const glm::vec2 & point);
    glm::vec2 RayExtended(const std::vector<glm::vec2> & segments, const glm::vec2 & point);
    void OnDrawCallback(const interface::RenderCommand & command, uint texturePos);

private:
    std::string _clipObjectURL;
    std::string _polyObjectURL;
    glm::vec4   _color;

    SharePtr<GLMesh>                    _mesh;
    SharePtr<GLProgram>                 _program;
    std::vector<glm::vec2>              _segments;
    SharePtr<CompLayerRender>           _layerRender;
    std::vector<SharePtr<CompPolygon>>  _polyObjects;
};