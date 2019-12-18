#include "comp_tilemap.h"
#include "../../raw_sys/raw_sys.h"

CompTilemap::CompTilemap()
    : _size(0.0f, 0.0f)
    , _update(kTilemap | kTrackPoint)
{
    _trackPoints.resize(4);

    _mesh = std::create_ptr<RawMesh>();
    _mesh->Init({}, {}, RawMesh::Vertex::kV | 
                        RawMesh::Vertex::kC |
                        RawMesh::Vertex::kUV);

    _program = std::create_ptr<RawProgram>();
    _program->Init(tools::GL_PROGRAM_TILEMAP);
}

void CompTilemap::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (!_url.empty())
    {
        Update();

        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        for (auto i = 0; i != _textures.size(); ++i)
        {
            auto & tex = _textures.at(i);
            auto   key = SFormat("uniform_texture{0}", i);
            command.mTextures.push_back(std::make_pair(key, tex));
        }
        canvas->Post(command);
    }
}

const std::string & CompTilemap::GetName()
{
    static const std::string name = "Tilemap";
    return name;
}

void CompTilemap::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, _url);
    tools::Serialize(os, _size);
}

void CompTilemap::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _size);
}

bool CompTilemap::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    _update |=(kTilemap | kTrackPoint);
    return true;
}

std::vector<Component::Property> CompTilemap::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Url", &_url);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size", &_size);
    return std::move(props);
}

void CompTilemap::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (_update & kTilemap)
        {
            UpdateTilemap();
        }

        if (_update & kTrackPoint)
        {
            _trackPoints.at(0).x = 0;       _trackPoints.at(0).y = 0;
            _trackPoints.at(1).x = _size.x; _trackPoints.at(1).y = 0;
            _trackPoints.at(2).x = _size.x; _trackPoints.at(2).y = _size.y;
            _trackPoints.at(3).x = 0;       _trackPoints.at(3).y = _size.y;
        }
        _update = 0;
    }
}

void CompTilemap::UpdateTilemap()
{ 
    auto tmx = mmc::Json::FromFile(_url);
    ASSERT_LOG(tmx != nullptr, _url.c_str());

    std::vector<Atlas> atlass;
    auto folder = tools::GetFileFolder(_url);
    for (auto & value : tmx->At("tilesets"))
    {
        auto atlasURL = folder + value.mVal->At("source")->ToString();
        auto baseIndex = (uint)value.mVal->At("firstgid")->ToNumber();
        auto atlasJson = mmc::Json::FromFile(atlasURL);
        ASSERT_LOG(atlasJson != nullptr, atlasURL.c_str());

        Atlas atlas;
        auto image = tools::GetFileFolder(atlasURL) + atlasJson->At("image")->ToString();
        atlas.mTexture  = Global::Ref().mRawSys->Get<RawTexture>(image);
        atlas.mOffset   = (uint)atlasJson->At(  "margin"      )->ToNumber();
        atlas.mSpace    = (uint)atlasJson->At(  "spacing"     )->ToNumber();
        atlas.mCol      = (uint)atlasJson->At(  "columns"     )->ToNumber();
        atlas.mRow      = (uint)atlasJson->At(  "tilecount"   )->ToNumber() / atlas.mCol;
        atlas.mBase = baseIndex;
        atlass.push_back(atlas);
    }

    std::vector<uint>           indexs;
    std::vector<RawMesh::Vertex> points;
    auto mapW   = (uint)tmx->At(    "width"         )->ToNumber();
    auto mapH   = (uint)tmx->At(    "height"        )->ToNumber();
    auto tileW  = (uint)tmx->At(    "tilewidth"     )->ToNumber();
    auto tileH  = (uint)tmx->At(    "tileheight"    )->ToNumber();
    for (auto & layer : tmx->At("layers"))
    {
        UpdateVertexs(mapW, mapH, tileW, tileH, layer.mVal->At("data"), atlass, indexs, points);
    }

    for (auto & atlas : atlass)
    {
        _textures.push_back(atlas.mTexture);
    }
    _size.x = (float)mapW * (float)tileW;
    _size.y = (float)mapH * (float)tileH;
    _mesh->Update(points, indexs);
}

void CompTilemap::UpdateVertexs(
    uint mapW,  uint mapH, 
    uint tileW, uint tileH, 
    const mmc::Json::Pointer & data, 
    const std::vector<Atlas> & atlass, 
    std::vector<uint>        & indexs, 
    std::vector<RawMesh::Vertex> & points)
{
    for (auto i = 0; i != data->GetCount(); ++i)
    {
        if (auto index = (uint)data->At(i)->ToNumber(); index!= 0)
        {
            uint atlasIndex = 0;
            auto uv = GetTileQuad(index, tileW, tileH, atlass, atlasIndex);

            glm::vec4 quad;
            quad.x =                      (float)(i % mapW * tileW);
            quad.y = (mapH - 1) * tileH - (float)(i / mapW * tileH);
            quad.z = quad.x + tileW + 0.5f;
            quad.w = quad.y + tileH + 0.5f;

            points.emplace_back(glm::vec2(quad.x, quad.y), glm::vec4((float)atlasIndex), glm::vec2(uv.x, uv.y));
            points.emplace_back(glm::vec2(quad.z, quad.y), glm::vec4((float)atlasIndex), glm::vec2(uv.z, uv.y));
            points.emplace_back(glm::vec2(quad.z, quad.w), glm::vec4((float)atlasIndex), glm::vec2(uv.z, uv.w));
            points.emplace_back(glm::vec2(quad.x, quad.y), glm::vec4((float)atlasIndex), glm::vec2(uv.x, uv.y));
            points.emplace_back(glm::vec2(quad.z, quad.w), glm::vec4((float)atlasIndex), glm::vec2(uv.z, uv.w));
            points.emplace_back(glm::vec2(quad.x, quad.w), glm::vec4((float)atlasIndex), glm::vec2(uv.x, uv.w));
            indexs.emplace_back(indexs.size());
            indexs.emplace_back(indexs.size());
            indexs.emplace_back(indexs.size());
            indexs.emplace_back(indexs.size());
            indexs.emplace_back(indexs.size());
            indexs.emplace_back(indexs.size());
        }
    }
}

glm::vec4 CompTilemap::GetTileQuad(
    uint idx, uint tileW, uint tileH, 
    const std::vector<Atlas> & atlass, 
    uint & atlasIndex)
{
    for (auto i = 0; i != atlass.size(); atlasIndex = i++)
    {
        if (idx < atlass.at(i).mBase) { break; }
    }
    glm::vec4 quad;
    auto & atlas = atlass.at(atlasIndex);
    auto x =                  (idx - atlas.mBase) % atlas.mCol;
    auto y = atlas.mRow - 1 - (idx - atlas.mBase) / atlas.mCol;
    quad.x = (float)(x * tileW + x * atlas.mSpace + atlas.mOffset) / atlas.mTexture->GetW();
    quad.y = (float)(y * tileH + y * atlas.mSpace + atlas.mOffset) / atlas.mTexture->GetH();
    quad.z = quad.x + (float)tileW                                 / atlas.mTexture->GetW();
    quad.w = quad.y + (float)tileH                                 / atlas.mTexture->GetH();
    return quad;
}

