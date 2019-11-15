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

private:
    void Update();

private:
    std::string                         _url;
    SharePtr<GLMesh>                    _mesh;
    SharePtr<GLProgram>                 _program;
    std::vector<SharePtr<CompPolygon>>  _polygons;
};