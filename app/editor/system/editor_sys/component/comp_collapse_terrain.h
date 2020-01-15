#pragma once

#include "../../raw_sys/component.h"

class CompCollapseTerrain : public Component {
public:
    using PairImage = std::pair<std::string, SharePtr<RawImage>>;

    using Clip = std::vector<glm::vec2>;
    using Area = std::vector<glm::vec2>;

    struct ClipPoint {
        uint mAreaIdx0;
        uint mAreaIdx1;
        float mAreaCross;
        uint mClipIdx0;
        uint mClipIdx1;
        float mClipCross;

        ClipPoint(
            uint    areaIdx0    = 0,
            uint    areaIdx1    = 0,
            float   areaCross   = 0,
            uint    clipIdx0    = 0,
            uint    clipIdx1    = 0,
            float   clipCross   = 0)
            : mAreaIdx0(areaIdx0)
            , mAreaIdx1(areaIdx1)
            , mAreaCross(areaCross)
            , mClipIdx0(clipIdx0)
            , mClipIdx1(clipIdx1)
            , mClipCross(clipCross)
        { }
    };

    struct ClipLine {
        uint mAreaEnd0;
        uint mAreaEnd1;
        std::vector<glm::vec2> mLine;
    };

    using ClipNums = std::vector<uint>;

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
    
    auto GenClipNums(const Clip & clip)->ClipNums;

    auto GenClipLine(
        const Clip & clip,
        const ClipPoint & cp0,
        const ClipPoint & cp1)->ClipLine;

    auto GenClipPoint(
        const Area & area,
        const Clip & clip,
        bool onlyone = false)->std::vector<ClipPoint>;

    void BinaryPoints(
        const Area & area,
        const Clip & clip,
        const ClipLine & clipLine,
        std::vector<glm::vec2> * output);

    bool UpdateClip(const Area & area, Clip & clip);

    void HandleClip(
        const Clip & clip,
        const std::vector<Area> & input, 
              std::vector<Area> & output);

    void HandleClip(const Clip & clip);

    bool IsContains(const std::vector<glm::vec2> & points0, 
                    const std::vector<glm::vec2> & points1);

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