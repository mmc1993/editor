#pragma once

#include "component.h"
#include "../../raw_sys/raw_sys.h"
#include "../../interface/render.h"

class CompTilemap : public Component {
private:
    enum UpdateEnum { 
        kTilemap    = 1,
        kTrackPoint = 2,
    };

    struct Atlas {
        uint mBase;
        uint mSpace;
        uint mOffset;
        uint mRow, mCol;
        SharePtr<GLTexture> mTexture;
    };

public:
    CompTilemap();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
    virtual bool OnModifyProperty(const std::any & oldValue, 
                                  const std::any & newValue, 
                                  const std::string & title) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Update();
    void UpdateTilemap();
    void UpdateVertexs(
        uint mapW, uint mapH,
        uint tileW, uint tileH,
        const mmc::Json::Pointer & data,
        const std::vector<Atlas> & atlass,
        std::vector<uint>        & indexs,
        std::vector<GLMesh::Vertex> & points);
    glm::vec4 GetTileQuad(
        uint idx, uint tileW, uint tileH, 
        const std::vector<Atlas> & atlass,
        uint & atlasIndex);

private:
    std::string     _url;
    glm::vec2       _size;
    uint            _update;

    SharePtr<GLMesh>                    _mesh;
    SharePtr<GLProgram>                 _program;
    std::vector<SharePtr<GLTexture>>    _textures;
};