#include "comp_tilemap.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompTilemap::CompTilemap()
    : mSize(0.0f, 0.0f)
    , mAnchor(0.5f, 0.5f)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh ->Init({}, {}, RawMesh::Vertex::kV | 
                         RawMesh::Vertex::kC |
                         RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_TILEMAP);

    AddState(StateEnum::kModifyTrackPoint, true);
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

        //  °ó¶¨äÖÈ¾»Øµ÷
        command.mCallback = std::bind(
            &CompTilemap::OnDrawCallback,
            CastPtr<CompTilemap>(shared_from_this()),
            std::placeholders::_1, std::placeholders::_2);

        //  ÉèÖÃ²Ã¼ô¾ØÐÎ
        auto min = command.mTransform * glm::vec4(mTrackPoints.at(0), 0, 1);
        auto max = command.mTransform * glm::vec4(mTrackPoints.at(2), 0, 1);
        command.mClipview.x = min.x;
        command.mClipview.y = min.y;
        command.mClipview.z = max.x;
        command.mClipview.w = max.y;

        //  ÆôÓÃ²Ã¼ô
        command.mEnabled = interface::FowardCommand::kClipView;

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
    tools::Serialize(os, mMapWH);
    tools::Serialize(os, mAnchor);
}

void CompTilemap::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    mMap.DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mMapWH);
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
            mMapWH.x = (float)map->GetMap().mPixelW;
            mMapWH.y = (float)map->GetMap().mPixelH;
            for (auto i = 0; i != map->GetAtlass().size(); ++i)
            {
                const auto & atlas = map->GetAtlass().at(i);
                auto key = SFormat("uniform_texture{0}", i);
                mTextures.emplace_back(key, atlas.mTexture);
                mSize.x = (float)map->GetMap().mPixelW;
                mSize.y = (float)map->GetMap().mPixelH;
            }
            mTrackPoints.at(0).x = 0;       mTrackPoints.at(0).y = 0;
            mTrackPoints.at(1).x = mSize.x; mTrackPoints.at(1).y = 0;
            mTrackPoints.at(2).x = mSize.x; mTrackPoints.at(2).y = mSize.y;
            mTrackPoints.at(3).x = 0;       mTrackPoints.at(3).y = mSize.y;
        }

        mTrackPoints.at(0).x = -mSize.x *      mAnchor.x;
        mTrackPoints.at(0).y = -mSize.y *      mAnchor.y;
        mTrackPoints.at(1).x =  mSize.x * (1 - mAnchor.x);
        mTrackPoints.at(1).y = -mSize.y *      mAnchor.y;
        mTrackPoints.at(2).x =  mSize.x * (1 - mAnchor.x);
        mTrackPoints.at(2).y =  mSize.y * (1 - mAnchor.y);
        mTrackPoints.at(3).x = -mSize.x *      mAnchor.x;
        mTrackPoints.at(3).y =  mSize.y * (1 - mAnchor.y);
    }
}

void CompTilemap::OnDrawCallback(const interface::RenderCommand & command, uint texturePos)
{ 
    auto & forward = (const interface::FowardCommand &)(command);
    forward.mProgram->BindUniformVector("anchor_", mAnchor);
    forward.mProgram->BindUniformVector("map_wh_",  mMapWH);
}

void CompTilemap::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    glm::vec2 min, max;
    switch (index)
    {
    case 0:
        min.x = std::min(point.x, mTrackPoints.at(2).x);
        min.y = std::min(point.y, mTrackPoints.at(2).y);
        max.x = mTrackPoints.at(2).x;
        max.y = mTrackPoints.at(2).y;
        break;
    case 1:
        min.x = mTrackPoints.at(0).x;
        min.y = std::min(point.y, mTrackPoints.at(3).y);
        max.x = std::max(point.x, mTrackPoints.at(3).x);
        max.y = mTrackPoints.at(2).y;
        break;
    case 2:
        min.x = mTrackPoints.at(0).x;
        min.y = mTrackPoints.at(0).y;
        max.x = std::max(point.x, mTrackPoints.at(0).x);
        max.y = std::max(point.y, mTrackPoints.at(0).y);
        break;
    case 3:
        min.x = std::min(point.x, mTrackPoints.at(1).x);
        min.y = mTrackPoints.at(0).y;
        max.x = mTrackPoints.at(2).x;
        max.y = std::max(point.y, mTrackPoints.at(1).y);
        break;
    }

    mSize.x = max.x - min.x;
    mSize.y = max.y - min.y;

    auto coord = GetOwner()->LocalToParent(mSize * mAnchor + min);
    GetOwner()->GetTransform()->Position(coord.x, coord.y);

    AddState(StateEnum::kUpdate, true);
}

void CompTilemap::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{ }

void CompTilemap::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{ }