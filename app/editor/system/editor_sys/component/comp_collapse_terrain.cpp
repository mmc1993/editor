#include "comp_collapse_terrain.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompCollapseTerrain::CompCollapseTerrain()
    : mAnchor(0.5f, 0.5f)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kUV);

    mTexture = std::create_ptr<RawImage>();
    mTexture->InitNull(GL_RGBA);

    mProgramInit = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_COLLAPSE_TERRAIN_INIT);
    mProgramDraw = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_COLLAPSE_TERRAIN_DRAW);
    mProgramQuad = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_SPRITE);
}

void CompCollapseTerrain::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (Update(canvas))
    {
        if (!mEraseList.empty())
        {
            ClearErase(canvas);
            mEraseList.clear();
        }

        RenderPipline::FowardCommand command;
        command.mMesh = mMesh;command.mProgram = mProgramQuad;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mPairImages.emplace_back("texture0",mTexture);
        command.mBlendSrc = GL_SRC_ALPHA;
        command.mBlendDst = GL_ONE_MINUS_SRC_ALPHA;
        command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;
        canvas->Post(command);
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
    tools::Serialize(os, mAnchor);
    mMap.EncodeBinary(os, project);
    mJson.EncodeBinary(os, project);
}

void CompCollapseTerrain::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mAnchor);
    mMap.DecodeBinary(is, project);
    mJson.DecodeBinary(is, project);
}

bool CompCollapseTerrain::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompCollapseTerrain::Erase(const std::vector<glm::vec2> & points)
{
    ASSERT_LOG(mMap.Check(), "");
    const glm::vec4 zeroColor(0, 0, 0, 0.0f);
    const glm::vec4 edgeColor(0, 0, 0, 0.5f);
    const glm::vec4 normColor(0, 0, 0, 1.0f);

    // points 世界坐标集
    const auto & map = mMap.Instance<RawMap>()->GetMap();
    glm::vec2 offset(map.mPixelW * mAnchor.x,
                     map.mPixelH * mAnchor.y);
    for (auto & convex : tools::StripConvexPoints(points))
    {
        for (auto & point : tools::StripTrianglePoints(convex))
        {
            mEraseList.emplace_back(GetOwner()->WorldToLocal(point) + offset, zeroColor);
        }
    }

    //  边缘处理
    auto order = tools::CalePointsOrder(points);
    ASSERT_LOG(order != 0, "");
    order = order >= 0.0f ? 1.0f : -1.0f;
    std::vector<RawMesh::Vertex> vertexs;
    auto count = points.size();
    for (auto i = 0; i != count; ++i)
    {
        auto & a = points.at(i);
        auto & b = points.at((i + 1) % count);
        auto & c = points.at((i + 2) % count);
        auto ab = b - a;
        auto bc = c - b;
        auto abr = glm::vec2(ab.y, -ab.x);
        auto bcr = glm::vec2(bc.y, -bc.x);
        abr = glm::normalize(abr) * 5.0f * order;
        bcr = glm::normalize(bcr) * 5.0f * order;

        auto ab0 = a, ab1 = a + abr;
        auto ab2 = b, ab3 = b + abr;
        auto bc0 = b, bc1 = b + bcr;

        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab0) + offset, edgeColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab3) + offset, normColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab1) + offset, normColor);

        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab2) + offset, edgeColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab3) + offset, normColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab0) + offset, edgeColor);

        auto ord = glm::cross(ab, bc);
        if (glm::cross(ab, bc) * order >= 0)
        {
            mEraseList.emplace_back(GetOwner()->WorldToLocal(ab2) + offset, edgeColor);
            mEraseList.emplace_back(GetOwner()->WorldToLocal(ab3) + offset, normColor);
            mEraseList.emplace_back(GetOwner()->WorldToLocal(bc1) + offset, normColor);
        }
    }
}

std::vector<Component::Property> CompCollapseTerrain::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Map",      &mMap,      std::vector<uint>{ Res::TypeEnum::kMap  });
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Json",     &mJson,     std::vector<uint>{ Res::TypeEnum::kJson });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompCollapseTerrain::Init(UIObjectGLCanvas* canvas)
{
    mEraseList.clear();

    //  初始化地图.开始
    const auto & map = mMap.Instance<RawMap>();
    ASSERT_LOG(map->GetAtlass().size() == 1,"");

    RenderPipline::TargetCommand targetCommand;
    targetCommand.mRenderTextures[0] = mTexture;
    targetCommand.mClearColor = glm::vec4(0.0f);
    targetCommand.mClipView.x = 0;
    targetCommand.mClipView.y = 0;
    targetCommand.mClipView.z = (float)map->GetMap().mPixelW;
    targetCommand.mClipView.w = (float)map->GetMap().mPixelH;
    targetCommand.mType = RenderPipline::TargetCommand::kPush;
    targetCommand.mEnabledFlag = RenderPipline::RenderCommand::kTargetColor0
                               | RenderPipline::RenderCommand::kTargetColor1
                               | RenderPipline::RenderCommand::kClipView;
    canvas->Post(targetCommand);

    //  填充地图
    RenderPipline::FowardCommand fowardCommand;
    fowardCommand.mMesh = std::create_ptr<RawMesh>();
    fowardCommand.mMesh->Init(map->GetPoints(), { },
        RawMesh::Vertex::kV | RawMesh::Vertex::kUV);

    fowardCommand.mPairImages.emplace_back( "texture0",
        map->GetAtlass().front().mTexture->GetImage());

    fowardCommand.mProgram = mProgramInit;
    fowardCommand.mViewMat = glm::lookAt(glm::vec3(0, 0, 0.0f), glm::vec3(0, 0, -1.0f), glm::vec3(0, 1, 0.0f));
    fowardCommand.mProjMat = glm::ortho(0.0f, (float)map->GetMap().mPixelW, 0.0f, (float)map->GetMap().mPixelH);
    fowardCommand.mBlendSrc = GL_SRC_ALPHA;
    fowardCommand.mBlendDst = GL_ONE_MINUS_SRC_ALPHA;
    fowardCommand.mEnabledFlag = RenderPipline::RenderCommand::kProjMat
                               | RenderPipline::RenderCommand::kViewMat
                               | RenderPipline::RenderCommand::kBlend;
    canvas->Post(fowardCommand);

    //  初始化地图.结束
    targetCommand.mType = RenderPipline::TargetCommand::kPop;
    canvas->Post(targetCommand);

    //  初始化碰撞边框
    for (auto & area : mJson.Instance<mmc::Json>()->At("List"))
    {
        auto & points = mPolygons.emplace_back();
        for (auto & point : area.mVal)
        {
            points.emplace_back(
                point.mVal->At("x")->ToNumber(),
                point.mVal->At("y")->ToNumber());
        }
        mPolygons.emplace_back(points);
    }
}

bool CompCollapseTerrain::Update(UIObjectGLCanvas * canvas)
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        
        if ( mMap.Check()  && mJson.Check() &&
            (mMap.Modify() || mJson.Modify()))
        {
            Init(canvas);
        }

        if (mMap.Check())
        {
            auto w = (iint)mMap.Instance<RawMap>()->GetMap().mPixelW;
            auto h = (iint)mMap.Instance<RawMap>()->GetMap().mPixelH;
            mTrackPoints.at(0).x = -w *      mAnchor.x;
            mTrackPoints.at(0).y = -h *      mAnchor.y;
            mTrackPoints.at(1).x =  w * (1 - mAnchor.x);
            mTrackPoints.at(1).y = -h *      mAnchor.y;
            mTrackPoints.at(2).x =  w * (1 - mAnchor.x);
            mTrackPoints.at(2).y =  h * (1 - mAnchor.y);
            mTrackPoints.at(3).x = -w *      mAnchor.x;
            mTrackPoints.at(3).y =  h * (1 - mAnchor.y);
        }

        mMesh->Update({
            { mTrackPoints.at(0), glm::vec2(0, 0) },
            { mTrackPoints.at(1), glm::vec2(1, 0) },
            { mTrackPoints.at(2), glm::vec2(1, 1) },

            { mTrackPoints.at(0), glm::vec2(0, 0) },
            { mTrackPoints.at(2), glm::vec2(1, 1) },
            { mTrackPoints.at(3), glm::vec2(0, 1) },
        }, {}, GL_DYNAMIC_DRAW, GL_STATIC_READ);
    }
    return mMap.Check() && mJson.Check();
}

void CompCollapseTerrain::ClearErase(UIObjectGLCanvas * canvas)
{
    ASSERT_LOG(!mEraseList.empty(), "");
    const auto & map = mMap.Instance<RawMap>();
    RenderPipline::TargetCommand  targetCommand;
    targetCommand.mRenderTextures[0] = mTexture;
    targetCommand.mClipView.x = 0;
    targetCommand.mClipView.y = 0;
    targetCommand.mClipView.z = (float)map->GetMap().mPixelW;
    targetCommand.mClipView.w = (float)map->GetMap().mPixelH;
    targetCommand.mType        = RenderPipline::TargetCommand::kPush;
    targetCommand.mEnabledFlag = RenderPipline::RenderCommand::kClipView;
    canvas->Post(targetCommand);

    RenderPipline::FowardCommand fowardCommand;
    fowardCommand.mMesh = std::create_ptr<RawMesh>();
    fowardCommand.mMesh->Init(mEraseList, {}, RawMesh::Vertex::kV | RawMesh::Vertex::kC);

    fowardCommand.mProgram = mProgramDraw;
    fowardCommand.mViewMat = glm::lookAt(glm::vec3(0, 0, 0.0f), glm::vec3(0, 0, -1.0f), glm::vec3(0, 1, 0.0f));
    fowardCommand.mProjMat = glm::ortho(0.0f, (float)map->GetMap().mPixelW, 0.0f, (float)map->GetMap().mPixelH);

    fowardCommand.mBlendSrc = GL_DST_ALPHA;
    fowardCommand.mBlendDst = GL_SRC_ALPHA;

    fowardCommand.mEnabledFlag = RenderPipline::RenderCommand::kViewMat
                               | RenderPipline::RenderCommand::kProjMat
                               | RenderPipline::RenderCommand::kBlend;
    canvas->Post(fowardCommand);

    targetCommand.mType = RenderPipline::TargetCommand::kPop;
    canvas->Post(targetCommand);
}
