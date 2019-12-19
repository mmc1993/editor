#include "comp_tilemap.h"
#include "../../raw_sys/raw_sys.h"

CompTilemap::CompTilemap()
    : mSize(0.0f, 0.0f)
{
    _trackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh ->Init({}, {}, RawMesh::Vertex::kV | 
                         RawMesh::Vertex::kC |
                         RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_TILEMAP);
}

void CompTilemap::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mMap.Check())
    {
        Update();

        interface::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mTextures   = mTextures;
        command.mTransform  = canvas->GetMatrixStack().GetM();
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
    mMap.EncodeBinary(os, project);
    tools::Serialize(os, mSize);
    tools::Serialize(os, mAnchor);
}

void CompTilemap::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    mMap.DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mAnchor);
}

bool CompTilemap::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

std::vector<Component::Property> CompTilemap::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Map",      &mMap,      std::vector<uint>{ (uint)Res::TypeEnum::kMap });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",     &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompTilemap::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (mMap.Check() && mMap.Modify())
        {
            auto map = mMap.Instance<RawMap>();
            mMesh->Update(map->GetPoints(),{});
            for (auto i = 0; i != map->GetAtlass().size(); ++i)
            {
                const auto & atlas = map->GetAtlass().at(i);
                auto key = SFormat("uniform_texture{0}", i);
                mTextures.emplace_back(key, atlas.mTexture);
                mSize.x = (float)map->GetMap().mPixelW;
                mSize.y = (float)map->GetMap().mPixelH;
            }
            //  ÷ÿΩ®µÿÕº
            _trackPoints.at(0).x = 0;       _trackPoints.at(0).y = 0;
            _trackPoints.at(1).x = mSize.x; _trackPoints.at(1).y = 0;
            _trackPoints.at(2).x = mSize.x; _trackPoints.at(2).y = mSize.y;
            _trackPoints.at(3).x = 0;       _trackPoints.at(3).y = mSize.y;
        }
    }
}
