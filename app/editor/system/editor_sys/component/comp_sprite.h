#pragma once

#include "../../raw_sys/component.h"

class CompSprite : public Component {
private:
    enum UpdateEnum {
        kTexture    = 1,
        kTrackPoint = 2,
    };

public:
    CompSprite();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
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
    Res::Ref            _tex;
    glm::vec2           _size;
    glm::vec2           _anchor;
    uint                _update;

    SharePtr<GLMesh>    _mesh;
    SharePtr<GLProgram> _program;
};