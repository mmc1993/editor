#include "comp_lightning.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompLightning::CompLightning()
    : mColor(1)
    , mScale(1)
    , mWidth(1)
    , mUpdate(kTexture | kSegment | kMesh)
{
    mTrackPoints.emplace_back(0, 0  );
    mTrackPoints.emplace_back(0, 100);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kC |
                       RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_LIGHTNING);

    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
}

void CompLightning::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mTex.Check())
    {
        Update();

        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mPairImages.emplace_back("texture0", mTex.Instance<RawTexture>()->GetRefImage());
        canvas->Post(command);
    }
}

const std::string & CompLightning::GetName()
{
    static const std::string name = "Lightning";
    return name;
}

void CompLightning::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    mTex.EncodeBinary(os, project);
    tools::Serialize(os, mScale);
    tools::Serialize(os, mWidth);
    tools::Serialize(os, mColor);
    tools::Serialize(os, mTrackPoints);
}

void CompLightning::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    mTex.DecodeBinary(is, project);
    tools::Deserialize(is, mScale);
    tools::Deserialize(is, mWidth);
    tools::Deserialize(is, mColor);
    tools::Deserialize(is, mTrackPoints);
}

bool CompLightning::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Tex")
    {
        mUpdate |= kTexture;
    }
    else if (title == "Width")
    {
        mUpdate |= kSegment;
    }
    else if (title == "Scale" 
          || title == "Color")
    {
        mUpdate |= kMesh;
    }
    return true;
}

std::vector<Component::Property> CompLightning::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Tex",    &mTex,      std::vector<uint>{ Res::TypeEnum::kImg });
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Scale",  &mScale);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Width",  &mWidth);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4, "Color", &mColor);
    return std::move(props);
}

void CompLightning::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (mTex.Modify())
        {
            mTex.Instance<RawTexture>()->GetRefImage()->SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            mTex.Instance<RawTexture>()->GetRefImage()->SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        if (mUpdate & kSegment)
        {
            std::queue<Segment> input;
            for (auto i = 0; i != mTrackPoints.size() - 1; ++i)
            {
                auto & a = mTrackPoints.at(i    );
                auto & b = mTrackPoints.at(i + 1);
                input.emplace(a, b, 0);
            }
            GenSegm(mWidth, input, mSegments);
        }

        if (mUpdate & (kSegment | kMesh))
        {
            GenMesh(mScale, mSegments);
        }

        mUpdate = 0;
    }
}

void CompLightning::GenSegm(float width, std::queue<Segment> & input, std::vector<Segment> & output)
{
    output.clear();
    while (!input.empty())
    {
        auto & segment = input.front();
        if (auto len = glm::length(segment.mEnded - segment.mStart); len >= width)
        {
            auto mid = glm::lerp(segment.mStart, segment.mEnded, 0.5f);
            auto dir = glm::normalize(segment.mEnded - segment.mStart);
            auto offset = glm::vec2(dir.y, -dir.x) * len;
            offset *= tools::Random(-1.0f, +1.0f) * 0.3f;
            input.emplace(segment.mStart, mid + offset, segment.mBranch);
            input.emplace(mid + offset, segment.mEnded, segment.mBranch);
        }
        else
        {
            if (tools::Random(0.0f, 1.0f) < std::pow(0.06f, segment.mBranch + 1) - 0.01f)
            {
                //  产生分支
                auto ended = glm::lerp(segment.mStart, segment.mEnded, 5.0f);
                auto length = glm::length(ended - segment.mStart);
                auto offset = glm::vec2(tools::Random(-length, length),
                                        tools::Random(-length, length));
                input.emplace(segment.mStart, ended + offset, segment.mBranch + 1);
            }
            output.emplace_back(segment);
        }
        input.pop();
    }
}

void CompLightning::GenMesh(float scale, const std::vector<Segment> & segments)
{
    std::vector<RawMesh::Vertex> points;
    for (auto & segment : segments)
    {
        auto mid = glm::lerp(segment.mStart, segment.mEnded, 0.5f);
        auto dir = glm::vec2(+(segment.mEnded - segment.mStart).y, 
                             -(segment.mEnded - segment.mStart).x);
        auto width = glm::length(segment.mEnded - segment.mStart);
        dir = glm::normalize(dir) * width * 0.5f;

        auto p0 = mid + (segment.mStart + dir - mid) * scale;
        auto p1 = mid + (segment.mEnded + dir - mid) * scale;
        auto p2 = mid + (segment.mEnded - dir - mid) * scale;
        auto p3 = mid + (segment.mStart - dir - mid) * scale;

        points.emplace_back(p0, mColor, glm::vec2(0, 0));
        points.emplace_back(p1, mColor, glm::vec2(1, 0));
        points.emplace_back(p2, mColor, glm::vec2(1, 1));

        points.emplace_back(p0, mColor, glm::vec2(0, 0));
        points.emplace_back(p2, mColor, glm::vec2(1, 1));
        points.emplace_back(p3, mColor, glm::vec2(0, 1));
    }
    mMesh->Update(points, { });
}

void CompLightning::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    mUpdate |= UpdateEnum::kSegment;
    mTrackPoints.at(index) = point;
}

void CompLightning::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    mUpdate |= UpdateEnum::kSegment;
    mTrackPoints.insert(std::next(mTrackPoints.begin(), index), point);
}

void CompLightning::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    if (mTrackPoints.size() > 1)
    {
        mUpdate |= UpdateEnum::kSegment;
        mTrackPoints.erase(std::next(mTrackPoints.begin(), index));
    }
}

