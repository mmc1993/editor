#pragma once

#include "../../raw_sys/component.h"

class CompCollapseTerrain : public Component {
public:
    using Polygon = std::vector<glm::vec2>;

    struct EraseParam {
        std::vector<RawMesh::Vertex> mPoints;
        uint mBlendSrc;
        uint mBlendDst;
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

    void Erase(
        const Polygon & points, 
        uint blendSrc = GL_ONE,
        uint blendDst = GL_ZERO,
        const glm::vec4 & color = glm::vec4(0, 0, 0, 0));

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Init();
    void Erase(
        uint i,
        UIObjectGLCanvas * canvas,
        const EraseParam & param);
    bool Update();      //  返回是否重置地形
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    Res::Ref    mMap;
    Res::Ref    mJson;
    glm::vec2   mSize;
    glm::vec2   mAnchor;

    SharePtr<RawProgram>            mEraseProgram;
    std::deque<EraseParam>          mEraseQueue;
    std::vector<SharePtr<RawMesh>>  mEraseMeshs;

    SharePtr<RawMesh>       mMesh;
    SharePtr<RawProgram>    mProgram;
    std::vector<Polygon>    mPolygons;
    std::vector<std::pair<std::string, SharePtr<RawImage>>> mPairImages;
};