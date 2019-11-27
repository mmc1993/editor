#pragma once

#include "component.h"
#include "comp_polygon.h"
#include "comp_render_target.h"
#include "../../interface/render.h"

class CompText 
    : public Component
    , public std::enable_shared_from_this<CompText> {
public:
    enum UpdateEnum {
        kFont = 0x1,
        kMesh = 0x2,
    };

public:
    CompText();

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
    virtual bool OnModifyProperty(const std::any & oldValue,
                                  const std::any & newValue,
                                  const std::string & title) override;
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;


private:
    void UpdateFont();
    void UpdateMesh();
    void OnDrawCallback(const interface::RenderCommand & command, uint pos);

private:
    uint _update;

    std::string _url;
    std::string _text;
    glm::vec2   _size;
    glm::vec2   _anchor;

    float       _outDelta; //  ÎÄ×Ö±ßÔµÆ½»¬·¶Î§
    glm::vec4   _outColor;
    glm::vec4   _color;

    SharePtr<GLFont>    _font;
    SharePtr<GLMesh>    _mesh;
    SharePtr<GLProgram> _program;
};