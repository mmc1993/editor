#pragma once

#include "../../raw_sys/component.h"

class CompCollapseTerrain : public Component {
public:
    using Polygon = std::vector<glm::vec2>;

    struct EraseParam {
        glm::vec2 mTriangle[3];
    };

    using PairImage = std::pair<std::string, SharePtr<RawImage>>;

public:
    CompCollapseTerrain();
    virtual void OnUpdate(UIObjectGLCanvas* canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
    virtual bool OnModifyProperty(
        const std::any & oldValue, 
        const std::any & newValue, 
        const std::string & title) override;

    void Erase(const std::vector<glm::vec2> & points);

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Init(UIObjectGLCanvas * canvas);
    bool Update(UIObjectGLCanvas* canvas);
    void ClearErase(UIObjectGLCanvas * canvas);
    void ClearErase(const std::vector<glm::vec2> & points);
    bool ClearErase(std::vector<glm::vec2> & points, 
                    std::vector<Polygon> & polygons0,
                    std::vector<Polygon> & polygons1);
    auto CrossResult(const std::vector<glm::vec2> & points,
                     const std::vector<glm::vec2> & polygon) -> std::tuple<bool, uint, uint, std::vector<glm::vec2>>;
    void BinaryPoints(uint endA, uint endB,
                      const std::vector<glm::vec2> & points,
                      const std::vector<glm::vec2> & clipLine, 
                      std::vector<glm::vec2> * output);

    //  ÍêÉÆºóÉ¾³ý
    //  Debug
    void DebugPostDrawPolygons(UIObjectGLCanvas * canvas);
    void DebugPostDrawPolygon(UIObjectGLCanvas * canvas, const Polygon & polygon);

private:
    Res::Ref    mMap;
    Res::Ref    mJson;
    glm::vec2   mAnchor;

    SharePtr<RawMesh>               mMesh;
    SharePtr<RawImage>              mTexture;
    std::vector<Polygon>            mPolygons;

    SharePtr<RawProgram>            mProgramInit;
    SharePtr<RawProgram>            mProgramDraw;
    SharePtr<RawProgram>            mProgramQuad;
    std::vector<RawMesh::Vertex>    mEraseList;
};