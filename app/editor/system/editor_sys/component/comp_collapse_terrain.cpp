#include "comp_collapse_terrain.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"
#include "comp_render_target.h"

CompCollapseTerrain::CompCollapseTerrain()
    : mSize(0.0f)
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
            command.mRenderTextures[0] = mPairImages.at(1).second;
            command.mClipView.x = 0;
            command.mClipView.y = 0;
            command.mClipView.z = mSize.x;
            command.mClipView.w = mSize.y;
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
    tools::Serialize(os, mSize);
    mJson.EncodeBinary(os, project);
}

void CompCollapseTerrain::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
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
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Json",     &mJson,     std::vector<uint>{ Res::TypeEnum::kJson });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",     &mSize);
    return std::move(props);
}

void CompCollapseTerrain::Init()
{
    if (mMapTarget && mJson.Check())
    {
        mReset       = true;
        mEraseQueue.clear();

        //  初始化MapImage
        auto mapImage = mMapTarget->GetImage();
        mPairImages.at(0).first  = "texture0";
        mPairImages.at(0).second = mapImage;

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
}

bool CompCollapseTerrain::Update()
{
    auto target = GetOwner()->GetComponent<CompRenderTarget>();
    if (target != mMapTarget) { mMapTarget = target; Init(); }
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        
        if (mJson.Check() && mJson.Modify()) { Init(); }

        mTrackPoints.at(0).x = 0;       mTrackPoints.at(0).y = 0;
        mTrackPoints.at(1).x = mSize.x; mTrackPoints.at(1).y = 0;
        mTrackPoints.at(2).x = mSize.x; mTrackPoints.at(2).y = mSize.y;
        mTrackPoints.at(3).x = 0;       mTrackPoints.at(3).y = mSize.y;

        mMesh->Update({ mTrackPoints.at(0),
                        mTrackPoints.at(0),
                        mTrackPoints.at(0),
                        mTrackPoints.at(0) }, {}, GL_DYNAMIC_DRAW, GL_STATIC_READ);
    }
    return mMapTarget && mJson.Check();
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
                erase.mMask != points.back().c.r)
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

    const auto & coord = GetOwner()->LocalToParent(min);
    GetOwner()->GetTransform()->Position(coord.x, coord.y);

    AddState(StateEnum::kUpdate, true);
}
