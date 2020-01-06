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
        //if (!mEraseQueue.empty())
        //{
        //    RenderPipline::TargetCommand command;
        //    command.mType   = RenderPipline::TargetCommand::kPush;
        //    command.mRenderTextures[0] = mPairImages.at(0).second;
        //    command.mClipView.x = 0;
        //    command.mClipView.y = 0;
        //    command.mClipView.z = (float)mMap.Instance<RawTexture>()->GetW();
        //    command.mClipView.w = (float)mMap.Instance<RawTexture>()->GetH();
        //    command.mEnabledFlag = RenderPipline::RenderCommand::kClipView;

        //    if (mReset)
        //    {
        //        mReset = false;
        //        command.mClearColor.r = 0;
        //        command.mClearColor.g = 0;
        //        command.mClearColor.b = 0;
        //        command.mClearColor.a = 0;
        //        command.mEnabledFlag |= RenderPipline::RenderCommand::kTargetColor0;
        //        command.mEnabledFlag |= RenderPipline::RenderCommand::kTargetColor1;
        //    }
        //    else
        //    {
        //        command.mEnabledFlag &= ~RenderPipline::RenderCommand::kTargetColor0;
        //        command.mEnabledFlag &= ~RenderPipline::RenderCommand::kTargetColor1;
        //    }
        //    canvas->Post(command);

        //    //  擦除地形
        //    HandleErase(canvas);
        //    mEraseQueue.clear();

        //    command.mType = RenderPipline::TargetCommand::kPop;
        //    canvas->Post(command);
        //}

        RenderPipline::FowardCommand command;
        command.mMesh = mMesh;command.mProgram = mProgramQuad;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mPairImages.emplace_back("texture0",mTexture);
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

void CompCollapseTerrain::Erase(const std::vector<glm::vec2> & points, uint blendSrc, uint blendDst, uint mask)
{
    for (auto & convex : tools::StripConvexPoints(points))
    {
        auto points = tools::StripTrianglePoints(convex);
        for (auto i = 0; i != points.size(); i += 3)
        {
            auto & param = mEraseQueue.emplace_back();
            param.mTriangle[0] = points.at(i    );
            param.mTriangle[1] = points.at(i + 1);
            param.mTriangle[2] = points.at(i + 2);
            param.mBlendSrc = blendSrc;
            param.mBlendDst = blendDst;
            param.mMask = mask;
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
    mEraseQueue.clear();

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
    fowardCommand.mViewMat = glm::lookAt(
        glm::vec3(
            (float)map->GetMap().mPixelW * 0.5f,
            (float)map->GetMap().mPixelH * 0.5f,  0),
        glm::vec3(
            (float)map->GetMap().mPixelW * 0.5f,
            (float)map->GetMap().mPixelH * 0.5f, -1),
        glm::vec3(0, 1, 0));
    fowardCommand.mProjMat = glm::ortho(
        (float)map->GetMap().mPixelW * -0.5f, (float)map->GetMap().mPixelW * 0.5f,
        (float)map->GetMap().mPixelH * -0.5f, (float)map->GetMap().mPixelH * 0.5f);
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

bool CompCollapseTerrain::Update(UIObjectGLCanvas* canvas)
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

void CompCollapseTerrain::HandleErase(UIObjectGLCanvas * canvas)
{
    //if (!mEraseQueue.empty())
    //{
    //    uint commandCount = 1;
    //    if (mEraseMeshs.empty())
    //    {
    //        mEraseMeshs.emplace_back(std::create_ptr<RawMesh>())
    //            ->Init({}, {}, RawMesh::Vertex::kV 
    //                         | RawMesh::Vertex::kC);
    //    }
    //    RenderPipline::FowardCommand command;
    //    command.mMesh = mEraseMeshs.front();
    //    command.mProgram = mEraseProgram;
    //    command.mBlendSrc = mEraseQueue.front().mBlendSrc;
    //    command.mBlendDst = mEraseQueue.front().mBlendDst;
    //    command.mTransform = canvas->GetMatrixStack().GetM();
    //    command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;

    //    std::vector<RawMesh::Vertex> points;
    //    for (const auto & erase : mEraseQueue)
    //    {
    //        if (erase.mBlendSrc != command.mBlendSrc ||
    //            erase.mBlendDst != command.mBlendDst ||
    //            !points.empty() && points.back().c.r != erase.mMask)
    //        {
    //            command.mMesh->Update(points, {}, GL_DYNAMIC_DRAW,
    //                                              GL_STATIC_DRAW);
    //            canvas->Post(command);

    //            if (mEraseMeshs.size() <= commandCount)
    //            {
    //                mEraseMeshs.emplace_back(std::create_ptr<RawMesh>())
    //                    ->Init({}, {}, RawMesh::Vertex::kV 
    //                                 | RawMesh::Vertex::kC);
    //            }
    //            points.clear();
    //            command.mBlendSrc = erase.mBlendSrc;
    //            command.mBlendDst = erase.mBlendDst;
    //            command.mMesh = mEraseMeshs.at(commandCount++);
    //        }
    //        points.emplace_back(erase.mTriangle[0], glm::vec4(erase.mMask, 0, 0, 0));
    //        points.emplace_back(erase.mTriangle[1], glm::vec4(erase.mMask, 0, 0, 0));
    //        points.emplace_back(erase.mTriangle[2], glm::vec4(erase.mMask, 0, 0, 0));
    //    }
    //    command.mMesh->Update(points, {}, GL_DYNAMIC_DRAW,
    //                                      GL_STATIC_DRAW);
    //    canvas->Post(command);
    //}
}
