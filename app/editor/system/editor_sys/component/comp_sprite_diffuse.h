#pragma once

#include "../../raw_sys/component.h"

class CompSpriteDiffuse : public Component {
private:
    enum UpdateEnum {
        kTexture = 1,
        kTrackPoint = 2,
    };

public:
    CompSpriteDiffuse();
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
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    Res::Ref            mImage;
    Res::Ref            mNoise;
    uint                mUpdate;
    glm::vec2           mAnchor;
    glm::vec2           mSize;
    glm::vec4           mThreshold;

    SharePtr<RawMesh>    mMesh;
    SharePtr<RawProgram> mProgram;
};