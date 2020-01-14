#pragma once

#include "../../raw_sys/component.h"

class CompCollapseTerrain : public Component {
public:
    using PairImage = std::pair<std::string, SharePtr<RawImage>>;

    using Clip = std::vector<glm::vec2>;
    using Area = std::vector<glm::vec2>;

    struct ClipCount {
        uint mAreaIdx;
        uint mAreaNum;
    };

    struct ClipPoint {
        uint mAreaIdx0;
        uint mAreaIdx1;
        float mAreaCross;
        uint mClipIdx0;
        uint mClipIdx1;
        float mClipCross;
    };

    struct ClipLine {
        uint mAreaEnd0;
        uint mAreaEnd1;
        std::vector<glm::vec2> mLine;
    };

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
    
    auto GenClipCount(const Clip & clip)->std::vector<ClipCount>;

    auto GenClipPoint(
        const Area & area,
        const Clip & clip,
        bool onlyone)->std::vector<ClipPoint>;

    auto GenClipLine(
        const Clip & clip,
        const ClipPoint & cp0,
        const ClipPoint & cp1)->ClipLine;

    void BinaryPoints(
        const Area & area,
        const Clip & clip,
        const ClipLine & clipLine,
        std::vector<glm::vec2> * output);

    void HandleClip(const Clip & clip);





    void ClearErase(const std::vector<glm::vec2> & points);
    bool ClearErase(std::vector<glm::vec2> & points, 
                    std::vector<Area> & polygons0,
                    std::vector<Area> & polygons1);
    auto CrossResult(const std::vector<glm::vec2> & points,
                     const std::vector<glm::vec2> & polygon) -> std::tuple<bool, uint, uint, std::vector<glm::vec2>>;
    void BinaryPoints(uint endA, uint endB,
                      const std::vector<glm::vec2> & points,
                      const std::vector<glm::vec2> & clipLine, 
                      std::vector<glm::vec2> * output);
    bool IsContains(const std::vector<glm::vec2> & points0, 
                    const std::vector<glm::vec2> & points1);
    void Optimize(std::vector<glm::vec2> & polygon);

    //  ÍêÉÆºóÉ¾³ý
    //  Debug
    void DebugPostDrawPolygons(UIObjectGLCanvas * canvas);
    void DebugPostDrawPolygon(UIObjectGLCanvas * canvas, const Area & polygon);

private:
    Res::Ref    mMap;
    Res::Ref    mJson;
    glm::vec2   mAnchor;

    std::vector<Area>               mAreas;

    SharePtr<RawMesh>               mMesh;
    SharePtr<RawImage>              mTexture;

    SharePtr<RawProgram>            mProgramInit;
    SharePtr<RawProgram>            mProgramDraw;
    SharePtr<RawProgram>            mProgramQuad;
    std::vector<RawMesh::Vertex>    mEraseList;
};