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

    mEraseProgram = std::create_ptr<RawProgram>();
    mEraseProgram->Init(tools::GL_PROGRAM_SOLID_FILL);

    mPairImages.resize(2);
    mPairImages.at(1).first = "texture0";
    mPairImages.at(1).second = std::create_ptr<RawImage>();

    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompCollapseTerrain::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    auto resetTerrain = Update();

    if (mMap.Check() && mJson.Check())
    {
        //  开始擦除地形
        if (!mEraseQueue.empty())
        {
            RenderPipline::TargetCommand command;
            command.mType = RenderPipline::TargetCommand::kPush;

            command.mRenderTextures[0] = mPairImages.at(1).second;

            command.mClipView.x = 0;
            command.mClipView.y = 0;
            command.mClipView.z = mSize.x;
            command.mClipView.w = mSize.y;
            command.mEnabledFlag = RenderPipline::RenderCommand::kClipView;

            if (resetTerrain)
            {
                command.mEnabledFlag &= ~RenderPipline::RenderCommand::kTargetColor0;
                command.mEnabledFlag &= ~RenderPipline::RenderCommand::kTargetColor1;
            }
            else
            {
                command.mClearColor.r = 0;
                command.mClearColor.g = 0;
                command.mClearColor.b = 0;
                command.mClearColor.a = 0;
                command.mEnabledFlag |= RenderPipline::RenderCommand::kTargetColor0;
                command.mEnabledFlag |= RenderPipline::RenderCommand::kTargetColor1;
            }
            canvas->Post(command);

            //  擦除地形
            for (auto i = 0; i != mEraseQueue.size(); ++i)
            {
                Erase(i, canvas, mEraseQueue.at(i));
            }
            mEraseQueue.clear();

            command.mType = RenderPipline::TargetCommand::kPop;
            canvas->Post(command);
        }

        //RenderPipline::FowardCommand command;
        //command.mMesh       = mMesh;
        //command.mProgram    = mProgram;
        //command.mPairImages = mPairImages;
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
    mJson.EncodeBinary(os, project);
}

void CompCollapseTerrain::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mAnchor);
    mMap.DecodeBinary(is, project);
    mJson.DecodeBinary(is, project);
}

bool CompCollapseTerrain::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompCollapseTerrain::Erase(const Polygon & points, uint blendSrc, uint blendDst, const glm::vec4 & color)
{
    for (auto & convex : tools::StripConvexPoints(points))
    {
        auto & param    = mEraseQueue.emplace_back();
        param.mBlendSrc = blendSrc;
        param.mBlendDst = blendDst;
        for (auto & point : tools::StripTrianglePoints(convex))
        {
            param.mPoints.emplace_back(point, color);
        }
    }
}

std::vector<Component::Property> CompCollapseTerrain::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Map",      &mMap,      std::vector<uint>{ Res::TypeEnum::kObj });
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Json",     &mJson,     std::vector<uint>{ Res::TypeEnum::kJson });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",     &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompCollapseTerrain::Init()
{
    //  初始化MapImage
    auto mapImage = mMap.Instance<GLObject>()->GetComponent<CompRenderTarget>()->GetImage();
    mPairImages.at(0).first = "texture0";
    mPairImages.at(0).second = mapImage;

    //  初始化Polygon
    mPolygons.clear();
    for (auto & area : mJson.Instance<mmc::Json>()->At("List"))
    {
        auto & points = mPolygons.emplace_back();
        for (auto & point : area.mVal)
        {
            points.emplace_back(
                point.mVal->At("x")->ToNumber(),
                point.mVal->At("y")->ToNumber());
        }
        Erase(points, GL_ONE, GL_ZERO, glm::vec4(1, 0, 0, 1));
    }
}

void CompCollapseTerrain::Erase(uint i, UIObjectGLCanvas * canvas, const EraseParam & param)
{
    if (i >= mEraseMeshs.size())
    {
        auto mesh = std::create_ptr<RawMesh>();
        mesh->Init({}, {}, RawMesh::Vertex::kV |
                           RawMesh::Vertex::kC);
        mEraseMeshs.emplace_back(mesh);
    }

    mEraseMeshs.at(i)->Update(param.mPoints, {},
                              GL_DYNAMIC_DRAW, 
                              GL_DYNAMIC_DRAW);

    RenderPipline::FowardCommand command;
    command.mBlendSrc       = param.mBlendSrc;
    command.mBlendDst       = param.mBlendDst;
    command.mMesh           = mEraseMeshs.at(i);
    command.mProgram        = mEraseProgram;
    command.mTransform      = canvas->GetMatrixStack().GetM();
    command.mEnabledFlag    = RenderPipline::RenderCommand::kBlend;
    canvas->Post(command);
}

bool CompCollapseTerrain::Update()
{
    auto ret = false;
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        mTrackPoints.at(0).x = -mSize.x *      mAnchor.x;
        mTrackPoints.at(0).y = -mSize.y *      mAnchor.y;
        mTrackPoints.at(1).x =  mSize.x * (1 - mAnchor.x);
        mTrackPoints.at(1).y = -mSize.y *      mAnchor.y;
        mTrackPoints.at(2).x =  mSize.x * (1 - mAnchor.x);
        mTrackPoints.at(2).y =  mSize.y * (1 - mAnchor.y);
        mTrackPoints.at(3).x = -mSize.x *      mAnchor.x;
        mTrackPoints.at(3).y =  mSize.y * (1 - mAnchor.y);

        ret =   mMap.Check()  && mJson.Check()
            && (mMap.Modify() || mJson.Modify());
        if (ret) { Init(); }
    }
    return ret;
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
