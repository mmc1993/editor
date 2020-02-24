#pragma once

#include "../../raw_sys/component.h"

class CompNoisePerlin : public Component {
public:
    CompNoisePerlin();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
    virtual bool OnModifyProperty(
        const std::any & oldValue,
        const std::any & newValue,
        const std::string & title) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Update();
    void OnDrawCallback(const RenderPipline::RenderCommand & command, uint texturePos);

private:
    std::vector<glm::vec2> mGrads;

    glm::vec2           mAnchor;
    glm::vec2           mCellWH;
    glm::vec2           mSize;

    SharePtr<RawMesh>    mMesh;
    SharePtr<RawProgram> mProgram;
};