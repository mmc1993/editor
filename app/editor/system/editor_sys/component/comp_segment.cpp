#include "comp_segment.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompSegment::CompSegment()
    : mColor(1)
    , mWidth(1)
    , mSmooth(1.0f)
    , mUpdate(kSegment | kMesh)
{
    mTrackPoints.emplace_back(-50,  0);
    mTrackPoints.emplace_back( 50, 50);
    mTrackPoints.emplace_back( 50,  0);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kC);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_SEGMENT);

    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
}

void CompSegment::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    Update();

    RenderPipline::FowardCommand command;
    command.mMesh       = mMesh;
    command.mProgram    = mProgram;
    command.mTransform  = canvas->GetMatrixStack().GetM();

    command.mBlendSrc = GL_SRC_COLOR;
    command.mBlendDst = GL_ZERO;

    command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;

    canvas->Post(command);
}

const std::string & CompSegment::GetName()
{
    static const std::string name = "Segment";
    return name;
}

void CompSegment::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mWidth);
    tools::Serialize(os, mColor);
    tools::Serialize(os, mSmooth);
    tools::Serialize(os, mTrackPoints);
}

void CompSegment::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mWidth);
    tools::Deserialize(is, mColor);
    tools::Deserialize(is, mSmooth);
    tools::Deserialize(is, mTrackPoints);
}

bool CompSegment::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Width" || title == "Color")
    {
        mUpdate |= kMesh;
    }
    else if (title == "Smooth")
    {
        auto value = std::any_cast<float>(newValue);
        mSmooth = std::clamp(value, 0.01f, 1.0f);
        mUpdate |= kSegment;
        return false;
    }
    return true;
}

std::vector<Component::Property> CompSegment::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Width", &mWidth);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4, "Color", &mColor);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Smooth", &mSmooth);
    return std::move(props);
}

void CompSegment::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        if (mUpdate & kSegment)
        {
            GenSegm();
        }
        if (mUpdate & (kSegment | kMesh))
        {
            GenMesh();
        }
    }
}

void CompSegment::GenSegm()
{
    mSegments.clear();
    if (mTrackPoints.size() == 2)
    {
        mSegments.emplace_back(mTrackPoints.at(0));
        mSegments.emplace_back(mTrackPoints.at(1));
    }
    else
    {
        glm::vec2 p;
        auto size = mTrackPoints.size();
        for (auto i = 1; i != size; ++i)
        {
            if (i == 1)
            {
                auto & a = mTrackPoints.at(i - 1);
                auto & b = mTrackPoints.at(i    );
                auto & c = mTrackPoints.at(i + 1);
                auto mab = glm::lerp(a, b, 0.5f);
                auto mbc = glm::lerp(b, c, 0.5f);
                auto m = glm::lerp(mab, mbc, 0.5f);
                auto diff=b-m; mab+=diff;mbc+=diff;
                DrawBezier(a, mab, b); p = mbc;
            }
            else if (i == size - 1)
            {
                auto & a = mTrackPoints.at(i - 1);
                auto & b = mTrackPoints.at(i    );
                DrawBezier(a, p, b);
            }
            else
            {
                auto & a = mTrackPoints.at(i - 1);
                auto & b = mTrackPoints.at(i    );
                auto & c = mTrackPoints.at(i + 1);
                auto mab = glm::lerp(a, b, 0.5f);
                auto mbc = glm::lerp(b, c, 0.5f);
                auto m = glm::lerp(mab, mbc, 0.5f);
                auto diff=b-m; mab+=diff;mbc+=diff;
                DrawBezier(a, p, mab, b); p = mbc;
            }
        }
    }

    //  拉卡朗日插值
    //if (mSmooth != 1.0f)
    //{
    //    mSegments.clear();
    //    auto points = mTrackPoints;
    //    std::sort(points.begin(), points.end(), 
    //        [](const auto & a, const auto & b) { return a.x < b.x; });
    //    auto iter = std::unique(points.begin(), points.end(), 
    //        [](const auto & a, const auto & b) { return a.x == b.x; });
    //    points.erase(iter,  points.end());
    //    if (points.size() < 2) { return; }

    //    auto beg = points.front().x;
    //    auto end = points.back().x;
    //    auto distance  = end - beg;
    //    auto step  = distance * mSmooth;
    //    auto count = int(1.0f / mSmooth);
    //    for (auto i = 0; i != count; ++i)
    //    {
    //        auto y = 0.0f;
    //        auto x = beg + i * step;
    //        for (auto i0 = 0; i0 != points.size(); ++i0)
    //        {
    //            auto selfY = points.at(i0).y;
    //            auto m0 = 1.0f; //  分子
    //            auto m1 = 1.0f; //  分母
    //            for (auto i1 = 0; i1 != points.size(); ++i1)
    //            {
    //                if (i0 != i1)
    //                {
    //                    m0 *= (x -               points.at(i1).x);
    //                    m1 *= (points.at(i0).x - points.at(i1).x);
    //                }
    //            }
    //            y += m0 / m1 * selfY;
    //        }
    //        mSegments.emplace_back(x, y);
    //    }
    //    mSegments.emplace_back(points.back());
    //}
    //else
    //{
    //    mSegments = mTrackPoints;
    //}
}

void CompSegment::GenMesh()
{
    if (mSegments.empty()) { return; }
    std::vector<RawMesh::Vertex> points;
    std::vector<uint>            indexs;
    for (auto i = 0; i != mSegments.size() - 1; ++i)
    {
        auto & a = mSegments.at(i    );
        auto & b = mSegments.at(i + 1);
        auto dir = glm::vec2(+(b -a).y, -(b -a).x);
        auto offset = glm::normalize(dir) * mWidth;

        if (i != 0)
        {
            indexs.emplace_back(points.size() - 3);
            indexs.emplace_back(points.size());
            indexs.emplace_back(points.size() + 3);

            indexs.emplace_back(points.size() - 3);
            indexs.emplace_back(points.size() + 3);
            indexs.emplace_back(points.size() - 2);
        }

        indexs.emplace_back(points.size());
        indexs.emplace_back(points.size() + 1);
        indexs.emplace_back(points.size() + 2);

        indexs.emplace_back(points.size());
        indexs.emplace_back(points.size() + 2);
        indexs.emplace_back(points.size() + 3);

        points.emplace_back(a + offset, mColor);
        points.emplace_back(b + offset, mColor);
        points.emplace_back(b - offset, mColor);
        points.emplace_back(a - offset, mColor);
    }
    mMesh->Update(points, indexs);
}

void CompSegment::DrawBezier(const glm::vec2 & a, const glm::vec2 & b, const glm::vec2 & c)
{
    const auto max = 1.0f + mSmooth;
    for (auto t = 0.0f; t < max; t += mSmooth)
    {
        auto p0 = glm::lerp(a, b, t);
        auto p1 = glm::lerp(b, c, t);
        mSegments.emplace_back(glm::lerp(p0, p1, t));
    }
}

void CompSegment::DrawBezier(const glm::vec2 & a, const glm::vec2 & b, const glm::vec2 & c, const glm::vec2 & d)
{
    const auto max = 1.0f + mSmooth;
    for (auto t = 0.0f; t < max; t += mSmooth)
    {
        auto p0 = glm::lerp(a, b, t);
        auto p1 = glm::lerp(b, c, t);
        auto p2 = glm::lerp(c, d, t);
        auto p3 = glm::lerp(p0, p1, t);
        auto p4 = glm::lerp(p1, p2, t);
        mSegments.emplace_back(glm::lerp(p3, p4, t));
    }
}

void CompSegment::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    mUpdate |= kSegment;
    AddState(StateEnum::kUpdate, true);
    mTrackPoints.at(index) = point;
}

void CompSegment::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    mUpdate |= kSegment;
    AddState(StateEnum::kUpdate, true);
    mTrackPoints.insert(std::next(mTrackPoints.begin(), index), point);
}

void CompSegment::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    if (mTrackPoints.size() > 2)
    {
        mUpdate |= kSegment;
        AddState(StateEnum::kUpdate, true);
        mTrackPoints.erase(std::next(mTrackPoints.begin(), index));
    }
}

