#pragma once

#include "../../raw_sys/component.h"

class CompCollapseTerrain : public Component {
public:
    struct EraseParam {
        glm::vec2 mTriangle[3];
        uint mBlendSrc;
        uint mBlendDst;
        uint mMask;
    };

public:
    CompCollapseTerrain();
    virtual void OnStart(UIObjectGLCanvas * canvas);
    virtual void OnLeave(UIObjectGLCanvas * canvas);

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
    virtual bool OnModifyProperty(
        const std::any & oldValue, 
        const std::any & newValue, 
        const std::string & title) override;

    void Erase(
        const std::vector<glm::vec2> & points,
        uint blendSrc = GL_ONE,
        uint blendDst = GL_ZERO,
        uint mask = 0);         //  1 ÏÔÊ¾, 0 ²Á³ý

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Init();
    bool Update();
    void HandleErase(UIObjectGLCanvas * canvas);

private:
    Res::Ref    mMap;
    Res::Ref    mJson;
    glm::vec2   mAnchor;
    bool        mReset;

    SharePtr<RawProgram>            mEraseProgram;
    std::deque<EraseParam>          mEraseQueue;
    std::vector<SharePtr<RawMesh>>  mEraseMeshs;

    SharePtr<RawMesh>               mMesh;
    SharePtr<RawProgram>            mProgram;
    std::vector<std::pair<std::string, SharePtr<RawImage>>> mPairImages;
};