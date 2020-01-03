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

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_COLLAPSE_TERRAIN);

    mEraseProgram = std::create_ptr<RawProgram>();
    mEraseProgram->Init(tools::GL_PROGRAM_SOLID_FILL);

    mPairImages.resize(2);
    mPairImages.at(0).first = "texture0";
    mPairImages.at(0).second = std::create_ptr<RawImage>();
    mPairImages.at(0).second->InitNull(GL_RED);
}

void CompCollapseTerrain::OnStart(UIObjectGLCanvas * canvas)
{
}

void CompCollapseTerrain::OnLeave(UIObjectGLCanvas * canvas)
{
    if (Update())
    {
        if (!mEraseQueue.empty())
        {
            RenderPipline::TargetCommand command;
            command.mType   = RenderPipline::TargetCommand::kPush;
            command.mRenderTextures[0] = mPairImages.at(0).second;
            command.mClipView.x = 0;
            command.mClipView.y = 0;
            command.mClipView.z = (float)mMap.Instance<RawTexture>()->GetW();
            command.mClipView.w = (float)mMap.Instance<RawTexture>()->GetH();
            command.mEnabledFlag = RenderPipline::RenderCommand::kClipView;

            if (mReset)
            {
                mReset = false;
                command.mClearColor.r = 0;
                command.mClearColor.g = 0;
                command.mClearColor.b = 0;
                command.mClearColor.a = 0;
                command.mEnabledFlag |= RenderPipline::RenderCommand::kTargetColor0;
                command.mEnabledFlag |= RenderPipline::RenderCommand::kTargetColor1;
            }
            else
            {
                command.mEnabledFlag &= ~RenderPipline::RenderCommand::kTargetColor0;
                command.mEnabledFlag &= ~RenderPipline::RenderCommand::kTargetColor1;
            }
            canvas->Post(command);

            //  擦除地形
            HandleErase(canvas);
            mEraseQueue.clear();

            command.mType = RenderPipline::TargetCommand::kPop;
            canvas->Post(command);
        }

        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mPairImages = mPairImages;
        command.mTransform  = canvas->GetMatrixStack().GetM();
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
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Map",      &mMap,      std::vector<uint>{ Res::TypeEnum::kImg  });
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Json",     &mJson,     std::vector<uint>{ Res::TypeEnum::kJson });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompCollapseTerrain::Init()
{
    mReset       = true;
    mEraseQueue.clear();

    //  初始化MapImage
    auto mapImage = mMap.Instance<RawTexture>();
    mPairImages.at(1).first  = "texture1";
    mPairImages.at(1).second = mapImage->GetImage();

    for (auto & area : mJson.Instance<mmc::Json>()->At("List"))
    {
        std::vector<glm::vec2> points;
        for (auto & point : area.mVal)
        {
            points.emplace_back(
                point.mVal->At("x")->ToNumber(),
                point.mVal->At("y")->ToNumber());
        }
        Erase(points, GL_ONE, GL_ZERO, 1);
    }
}

bool CompCollapseTerrain::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        
        if ( mMap.Check()  && mJson.Check() && 
            (mMap.Modify() || mJson.Modify()))
        {
            Init();
        }

        if (mMap.Check())
        {
            auto w = (iint)mMap.Instance<RawTexture>()->GetW();
            auto h = (iint)mMap.Instance<RawTexture>()->GetH();
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
    if (!mEraseQueue.empty())
    {
        uint commandCount = 1;
        if (mEraseMeshs.empty())
        {
            mEraseMeshs.emplace_back(std::create_ptr<RawMesh>())
                ->Init({}, {}, RawMesh::Vertex::kV 
                             | RawMesh::Vertex::kC);
        }
        RenderPipline::FowardCommand command;
        command.mMesh = mEraseMeshs.front();
        command.mProgram = mEraseProgram;
        command.mBlendSrc = mEraseQueue.front().mBlendSrc;
        command.mBlendDst = mEraseQueue.front().mBlendDst;
        command.mTransform = canvas->GetMatrixStack().GetM();
        command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;

        std::vector<RawMesh::Vertex> points;
        for (const auto & erase : mEraseQueue)
        {
            if (erase.mBlendSrc != command.mBlendSrc ||
                erase.mBlendDst != command.mBlendDst ||
                !points.empty() && points.back().c.r != erase.mMask)
            {
                command.mMesh->Update(points, {}, GL_DYNAMIC_DRAW,
                                                  GL_STATIC_DRAW);
                canvas->Post(command);

                if (mEraseMeshs.size() <= commandCount)
                {
                    mEraseMeshs.emplace_back(std::create_ptr<RawMesh>())
                        ->Init({}, {}, RawMesh::Vertex::kV 
                                     | RawMesh::Vertex::kC);
                }
                points.clear();
                command.mBlendSrc = erase.mBlendSrc;
                command.mBlendDst = erase.mBlendDst;
                command.mMesh = mEraseMeshs.at(commandCount++);
            }
            points.emplace_back(erase.mTriangle[0], glm::vec4(erase.mMask, 0, 0, 0));
            points.emplace_back(erase.mTriangle[1], glm::vec4(erase.mMask, 0, 0, 0));
            points.emplace_back(erase.mTriangle[2], glm::vec4(erase.mMask, 0, 0, 0));
        }
        command.mMesh->Update(points, {}, GL_DYNAMIC_DRAW,
                                          GL_STATIC_DRAW);
        canvas->Post(command);
    }
}
