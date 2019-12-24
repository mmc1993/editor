#pragma once

#include "../../raw_sys/component.h"

class CompCollapseTerrain : public Component {
public:
    using Polygon = std::vector<glm::vec2>;

    struct CollapseInfo {
        Polygon mPolygon;
        SharePtr<RawMesh> mMesh;
    };

public:
    CompCollapseTerrain();
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
    void Update();
    void PostInit();
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    //  绑定Map
    //  生成初始Mask
    //  生成初始多边形


    Res::Ref    mMap;
    glm::vec2   mSize;
    glm::vec2   mAnchor;

    SharePtr<RawMesh> mMesh;
    SharePtr<RawProgram> mProgram;
    SharePtr<RawTexture> mMaskBuff;
    std::vector<Polygon> mPolygons;
    std::vector<CollapseInfo> mCollapseQueue;
    std::vector<SharePtr<RawMesh>> mMeshPool;
    std::vector<std::pair<std::string, SharePtr<RawTexture>>> mTextures;
};