#pragma once

#include "../../raw_sys/component.h"

class CompCollapseTerrain : public Component {
public:
    using Polygon = std::vector<glm::vec2>;

    struct EraseParam {
        glm::vec4 mColor;
        Polygon mPolygon;
        uint mBlendSrc;
        uint mBlendDst;
    };

public:
    CompCollapseTerrain();
    virtual void OnStart(UIObjectGLCanvas * canvas) override;
    virtual void OnLeave(UIObjectGLCanvas * canvas) override;
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
    virtual bool OnModifyProperty(const std::any & oldValue, 
                                  const std::any & newValue, 
                                  const std::string & title) override;

    void Collapse(const Polygon & polygon);

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Init();
    void Update();
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    //  绑定Map
    //  生成初始Mask
    //  生成初始多边形

    Res::Ref    mMap;
    Res::Ref    mTerrain;
    glm::vec2   mSize;
    glm::vec2   mAnchor;

    SharePtr<RawMesh> mMesh;
    SharePtr<RawImage>   mMaskBuff;
    std::vector<Polygon> mPolygons;
    SharePtr<RawProgram> mProgram;
    std::deque<EraseParam> mEraseQueue;
    std::vector<SharePtr<RawMesh>> mMeshPool;
    std::vector<std::pair<std::string, SharePtr<RawImage>>> mPairImages;
};