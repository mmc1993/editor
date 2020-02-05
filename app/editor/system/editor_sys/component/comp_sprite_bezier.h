#pragma once

#include "../../raw_sys/component.h"

class CompSpriteBezier : public Component {
private:
    enum UpdateEnum {
        kTexture    = 1,
        kTrackPoint = 2,
    };

public:
    CompSpriteBezier();
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
    glm::vec2 CalcBezierPoint(
        const glm::vec2 & p0, 
        const glm::vec2 & p1, 
        const glm::vec2 & p2, float t);
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    Res::Ref            mTex;
    glm::vec2           mSize;
    glm::vec2           mAnchor;
    uint                mUpdate;

    SharePtr<RawMesh>    mMesh;
    SharePtr<RawProgram> mProgram;
};