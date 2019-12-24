#pragma once

#include "../../raw_sys/component.h"
#include "../../raw_sys/raw_sys.h"

class CompTilemap : public Component {
public:
    CompTilemap();
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
    void OnDrawCallback(const RenderPipline::RenderCommand & command, uint texturePos);
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnInsertTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnDeleteTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    using PairImage = std::pair<std::string, SharePtr<RawImage>>;

    Res::Ref        mMap;
    glm::vec2       mSize;
    glm::vec2       mMapWH;
    glm::vec2       mAnchor;

    SharePtr<RawMesh>    mMesh;
    SharePtr<RawProgram> mProgram;
    std::vector<PairImage> mPairImages;
};