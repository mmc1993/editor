#include "comp_tilemap.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompTilemap::CompTilemap()
    : _size(0.0f, 0.0f)
    , _anchor(0.5f, 0.5f)
    , _update(kTilemap | kTrackPoint)
{
    _trackPoints.resize(4);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({}, {}, GLMesh::Vertex::kV | 
                        GLMesh::Vertex::kUV);

    _program = std::create_ptr<GLProgram>();
    _program->Init(tools::GL_PROGRAM_SPRITE);
}

void CompTilemap::OnAdd()
{ }

void CompTilemap::OnDel()
{ }

void CompTilemap::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (!_url.empty())
    {
        Update();

        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        //for (auto i = 0; i != _textures.size(); ++i)
        //{
        //    auto & tex = _textures.at(i);
        //    auto   key = SFormat("uniform_texture{0}", i);
        //    command.mTextures.push_back(std::make_pair(key, tex));
        //}
        canvas->Post(command);
    }
}

const std::string & CompTilemap::GetName()
{
    static const std::string name = "Sprite";
    return name;
}

void CompTilemap::EncodeBinary(std::ofstream & os)
{
    tools::Serialize(os, _url);
    tools::Serialize(os, _size);
    tools::Serialize(os, _anchor);
}

void CompTilemap::DecodeBinary(std::ifstream & is)
{
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _size);
    tools::Deserialize(is, _anchor);
}

bool CompTilemap::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "URL") _update |= kTilemap;
    if (title == "Anchor") _update |= kTrackPoint;
    return true;
}

std::vector<Component::Property> CompTilemap::CollectProperty()
{
    return {
        { interface::Serializer::StringValueTypeEnum::kAsset,   "Url",    &_url    },
        { interface::Serializer::StringValueTypeEnum::kVector2, "Size",   &_size   },
        { interface::Serializer::StringValueTypeEnum::kVector2, "Anchor", &_anchor }
    };
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
            _trackPoints.at(0).x = -_size.x *      _anchor.x;
            _trackPoints.at(0).y = -_size.y *      _anchor.y;
            _trackPoints.at(1).x =  _size.x * (1 - _anchor.x);
            _trackPoints.at(1).y = -_size.y *      _anchor.y;
            _trackPoints.at(2).x =  _size.x * (1 - _anchor.x);
            _trackPoints.at(2).y =  _size.y * (1 - _anchor.y);
            _trackPoints.at(3).x = -_size.x *      _anchor.x;
            _trackPoints.at(3).y =  _size.y * (1 - _anchor.y);
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
        UpdateAtlass((uint)value.mVal->At("firstgid")->ToNumber(), 
            folder + value.mVal->At("source")->ToString(), atlass);
    }

    std::vector<uint>           indexs;
    std::vector<GLMesh::Vertex> points;
    auto mapW = (uint)tmx->At("width")->ToNumber();
    auto mapH = (uint)tmx->At("height")->ToNumber();
    auto tileW = (uint)tmx->At("tilewidth")->ToNumber();
    auto tileH = (uint)tmx->At("tileheight")->ToNumber();
    for (auto & layer : tmx->At("layers"))
    {
        UpdateVertexs(mapW, mapH, tileW, tileH, layer.mVal->At("data"), _atlass, indexs, points);
    }

    _mesh->Update(points, indexs);
}

void CompTilemap::UpdateAtlass(uint base, const std::string & url, std::vector<Atlas> & atlass)
{
    auto json = mmc::Json::FromFile(url);
    ASSERT_LOG(json != nullptr, url.c_str());

    Atlas atlas;
    auto image = tools::GetFileFolder(url) + json->At("image")->ToString();
    atlas.mTexture = Global::Ref().mRawSys->Get<GLTexture>(image);
    atlas.mOffset = (uint)json->At("margin")->ToNumber();
    atlas.mSpace = (uint)json->At("spacing")->ToNumber();
    atlas.mIndexBase = base;
    atlass.push_back(atlas);
}

void CompTilemap::UpdateVertexs(uint mapW, uint mapH,
                                uint tileW, uint tileH, 
                                const mmc::Json::Pointer & data, 
                                const std::vector<Atlas> & atlass, 
                                std::vector<uint>           & indexs,
                                std::vector<GLMesh::Vertex> & points)
{
    for (auto i = 0; i != data->GetCount(); ++i)
    {
        auto x = i % mapW;
        auto y = i / mapH;
    }
}

