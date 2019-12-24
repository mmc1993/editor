#include "comp_collapse_terrain.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"
#include "comp_render_target.h"

CompCollapseTerrain::CompCollapseTerrain()
    : mSize(0.0f)
    , mAnchor(0.0f)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_COLLAPSE_TERRAIN);

    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompCollapseTerrain::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mMap.Check())
    {
        Update();

        //RenderPipline::FowardCommand command;
        //command.mMesh       = mMesh;
        //command.mProgram    = mProgram;
        //command.mTransform  = canvas->GetMatrixStack().GetM();
        //canvas->Post(command);
    }
}

const std::string & CompCollapseTerrain::GetName()
{
    static const std::string name = "CollapseTerrain";
    return name;
}

void CompCollapseTerrain::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mSize);
    tools::Serialize(os, mAnchor);
    mMap.EncodeBinary(os, project);
}

void CompCollapseTerrain::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mAnchor);
    mMap.DecodeBinary(is, project);
}

bool CompCollapseTerrain::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompCollapseTerrain::Collapse(const Polygon & polygon)
{
}

std::vector<Component::Property> CompCollapseTerrain::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Map",      &mMap,  std::vector<uint>{ Res::TypeEnum::kObj });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",     &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompCollapseTerrain::Init()
{
    CollapseInfo info;
}

void CompCollapseTerrain::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (mMap.Modify())
        {
            Init();
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

void CompCollapseTerrain::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    glm::vec2 min(0);
    glm::vec2 max(0);
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
