#include "comp_sprite_bezier.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompSpriteBezier::CompSpriteBezier()
    : mSize(0.0f, 0.0f)
    , mAnchor(0.5f, 0.5f)
    , mUpdate(kTexture | kTrackPoint)
{
    mTrackPoints.resize(5);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_SPRITE);

    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompSpriteBezier::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mTex.Check())
    {
        Update();

        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mPairImages.emplace_back("texture0", mTex.Instance<RawTexture>()->GetImage());

        command.mBlendSrc = GL_SRC_ALPHA;
        command.mBlendDst = GL_ONE_MINUS_SRC_ALPHA;

        command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;

        canvas->Post(command);
    }
}

const std::string & CompSpriteBezier::GetName()
{
    static const std::string name = "SpriteBezier";
    return name;
}

void CompSpriteBezier::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mSize);
    tools::Serialize(os, mAnchor);
    mTex.EncodeBinary(os, project);
}

void CompSpriteBezier::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mAnchor);
    mTex.DecodeBinary(is, project);
}

bool CompSpriteBezier::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Tex")
    {
        mUpdate |= kTexture 
                | kTrackPoint;
    }
    if (title == "Size" || title == "Anchor")
    {
        mUpdate |= kTrackPoint;
    }
    return true;
}

std::vector<Component::Property> CompSpriteBezier::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Tex",      &mTex,      std::vector<uint>{ (uint)Res::TypeEnum::kImg });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",     &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompSpriteBezier::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (mUpdate & kTexture)
        {
            mSize.x = (float)mTex.Instance<RawTexture>()->GetW();
            mSize.y = (float)mTex.Instance<RawTexture>()->GetH();
        }

        if (mUpdate & kTrackPoint)
        {
            mTrackPoints.at(1).x =  mSize.x * (1 - mAnchor.x);
            mTrackPoints.at(1).y = -mSize.y *      mAnchor.y;
            mTrackPoints.at(2).x =  mSize.x * (1 - mAnchor.x);
            mTrackPoints.at(2).y =  mSize.y * (1 - mAnchor.y);
            mTrackPoints.at(3).x = -mSize.x *      mAnchor.x;
            mTrackPoints.at(3).y =  mSize.y * (1 - mAnchor.y);

            const glm::vec2 min(mTrackPoints.at(3).x, mTrackPoints.at(1).y);
            const glm::vec2 max(mTrackPoints.at(1).x, mTrackPoints.at(3).y);
            mTrackPoints.at(0).y = min.y;
            mTrackPoints.at(0).x = std::clamp(mTrackPoints.at(0).x, min.x, max.x);
            mTrackPoints.at(4).y = std::clamp(mTrackPoints.at(4).y, min.y, max.y);
            mTrackPoints.at(4).x = std::clamp(mTrackPoints.at(4).x, min.x, max.x);


            //  º∆À„Õ¯∏Ò
            auto & offset = mTex.Instance<RawTexture>()->GetOffset();

            const auto Smooth = 128;
            const auto step = 1.0f / (Smooth -1);
            const auto & p0 = mTrackPoints.at(0);
            const auto & p1 = mTrackPoints.at(4);
            const auto & p2 = mTrackPoints.at(3);
            std::vector<RawMesh::Vertex> points;
            std::vector<uint>            indexs;

            auto l = p2.y - p0.y;
            for (auto i = 0; i != Smooth; ++i)
            {
                auto v0 = CalcBezierPoint(p0, p1, p2, step * i);
                auto v1 = glm::vec2(mTrackPoints.at(1).x, v0.y);

                auto d = v0.y - p0.y;
                auto v = glm::lerp(offset.y, offset.w, d / l);
                points.emplace_back(v0, glm::vec2(offset.x, v));
                points.emplace_back(v1, glm::vec2(offset.z, v));

                if (i != Smooth - 1)
                {
                    indexs.emplace_back(points.size());
                    indexs.emplace_back(points.size() - 1);
                    indexs.emplace_back(points.size() - 2);

                    indexs.emplace_back(points.size());
                    indexs.emplace_back(points.size() - 1);
                    indexs.emplace_back(points.size() + 1);
                }
            }

            mMesh->Update(points, indexs);
        }
        mUpdate = 0;
    }
}

glm::vec2 CompSpriteBezier::CalcBezierPoint(const glm::vec2 & p0, const glm::vec2 & p1, const glm::vec2 & p2, float t)
{
    auto r0 = glm::lerp(p0, p1, t);
    auto r1 = glm::lerp(p1, p2, t);
    return glm::lerp(r0, r1, t);
}

void CompSpriteBezier::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    glm::vec2 min(mTrackPoints.at(3).x, mTrackPoints.at(1).y);
    glm::vec2 max(mTrackPoints.at(1).x, mTrackPoints.at(3).y);
    switch (index)
    {
    case 1:
        min.y = std::min(point.y, mTrackPoints.at(3).y);
        max.x = std::max(point.x, mTrackPoints.at(3).x);
        break;
    case 2:
        max.x = std::max(point.x, mTrackPoints.at(3).x);
        max.y = std::max(point.y, mTrackPoints.at(1).y);
        break;
    case 3:
        min.x = std::min(point.x, mTrackPoints.at(1).x);
        max.y = std::max(point.y, mTrackPoints.at(1).y);
        break;
    case 0:
    case 4:
        mTrackPoints.at(index).x = std::clamp(point.x, min.x, max.x);
        mTrackPoints.at(index).y = std::clamp(point.y, min.y, max.y);
        break;
    }

    mSize.x = max.x - min.x;
    mSize.y = max.y - min.y;

    auto coord = GetOwner()->LocalToParent(mSize * mAnchor + min);
    GetOwner()->GetTransform()->Position(coord.x, coord.y);

    mUpdate |= kTrackPoint;

    AddState(StateEnum::kUpdate, true);
}