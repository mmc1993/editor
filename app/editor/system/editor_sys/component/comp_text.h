#pragma once

#include "../../raw_sys/component.h"

class CompText 
    : public Component
    , public std::enable_shared_from_this<CompText> {

public:
    CompText();

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
    virtual bool OnModifyProperty(const std::any & oldValue,
                                  const std::any & newValue,
                                  const std::string & title) override;
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;


private:
    void UpdateMesh();
    void OnDrawCallback(const interface::RenderCommand & command, uint pos);

private:
    Res::Ref    _fnt;
    std::string _text;
    glm::vec2   _size;
    glm::vec2   _anchor;

    float       _outDelta; //  ÎÄ×Ö±ßÔµÆ½»¬·¶Î§
    glm::vec4   _outColor;
    glm::vec4   _color;

    SharePtr<RawMesh>    _mesh;
    SharePtr<RawProgram> _program;
};