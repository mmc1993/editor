#include "comp_segment.h"
#include "comp_transform.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompSegment::CompSegment()
    : _color(1)
    , _width(1)
    , _smooth(1.0f)
    , _update(kSegment | kMesh)
{
    _trackPoints.emplace_back(0, 0 );
    _trackPoints.emplace_back(0, 100);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({},{}, GLMesh::Vertex::kV | 
                       GLMesh::Vertex::kC);

    _program = std::create_ptr<GLProgram>();
    _program->Init(tools::GL_PROGRAM_SEGMENT);

    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
}

void CompSegment::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    Update();

    interface::FowardCommand command;
    command.mMesh       = _mesh;
    command.mProgram    = _program;
    command.mTransform  = canvas->GetMatrixStack().GetM();
    canvas->Post(command);
}

const std::string & CompSegment::GetName()
{
    static const std::string name = "Segment";
    return name;
}

void CompSegment::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _width);
    tools::Serialize(os, _color);
    tools::Serialize(os, _smooth);
    tools::Serialize(os, _trackPoints);
}

void CompSegment::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _width);
    tools::Deserialize(is, _color);
    tools::Deserialize(is, _smooth);
    tools::Deserialize(is, _trackPoints);
}

bool CompSegment::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Width" || title == "Color")
    {
        _update |= kMesh;
    }
    else if (title == "Smooth")
    {
        auto value = std::any_cast<float>(newValue);
        _smooth = std::clamp(value, 0.01f, 1.0f);
        _update |= kSegment;
        return false;
    }
    return true;
}

std::vector<Component::Property> CompSegment::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Width", &_width);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kColor4, "Color", &_color);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Smooth", &_smooth);
    return std::move(props);
}

void CompSegment::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        if (_update & kSegment)
        {
            GenSegm();
        }
        if (_update & (kSegment | kMesh))
        {
            GenMesh();
        }
    }
}

void CompSegment::GenSegm()
{
    //  贝塞尔插值
    std::vector<glm::vec2> ctrlPoints{ _trackPoints.front() };
    if (_trackPoints.size() > 2)
    {
        for (auto i = 0; i != _trackPoints.size() - 2; ++i)
        {
            auto & a = _trackPoints.at(i    );
            auto & b = _trackPoints.at(i + 1);
            auto & c = _trackPoints.at(i + 2);

            auto mid0 = glm::lerp(a, b, 0.5f);
            auto mid1 = glm::lerp(b, c, 0.5f);

            auto abLen = glm::length(b - a);
            auto bcLen = glm::length(c - b);
            auto sunLen = abLen + bcLen;

            auto offset = b - glm::lerp(mid0, mid1, abLen / sunLen);
            ctrlPoints.emplace_back(mid0 + offset);
            ctrlPoints.emplace_back(mid1 + offset);
        }
    }
    ctrlPoints.emplace_back(_trackPoints.back());

    _segments.clear();
    if (ctrlPoints.size() == 2)
    {
        //  1次贝塞尔曲线
        _segments.emplace_back(ctrlPoints.at(0));
        _segments.emplace_back(ctrlPoints.at(1));
    }
    else
    {
        //  N次贝塞尔曲线
        auto count = iint(1.0f / _smooth);
        for (auto s = 0; s != count; ++s)
        {
            auto t = s * _smooth;
            auto beg = glm::lerp(ctrlPoints.at(0), ctrlPoints.at(1), t);
            auto end = glm::lerp(ctrlPoints.at(1), ctrlPoints.at(2), t);
            for (auto i = 2; i != ctrlPoints.size() - 1; ++i)
            {
                auto & a = ctrlPoints.at(i    );
                auto & b = ctrlPoints.at(i + 1);
                beg = glm::lerp(beg, end, t);
                end = glm::lerp(a, b, t);
            }
            _segments.emplace_back(glm::lerp(beg, end, t));
        }
        _segments.emplace_back(ctrlPoints.back());
    }


    for (auto & point : ctrlPoints)
    {
        _segments.emplace_back(point);
    }
    

    //  拉卡朗日插值
    //if (_smooth != 1.0f)
    //{
    //    _segments.clear();
    //    auto points = _trackPoints;
    //    std::sort(points.begin(), points.end(), 
    //        [](const auto & a, const auto & b) { return a.x < b.x; });
    //    auto iter = std::unique(points.begin(), points.end(), 
    //        [](const auto & a, const auto & b) { return a.x == b.x; });
    //    points.erase(iter,  points.end());
    //    if (points.size() < 2) { return; }

    //    auto beg = points.front().x;
    //    auto end = points.back().x;
    //    auto distance  = end - beg;
    //    auto step  = distance * _smooth;
    //    auto count = int(1.0f / _smooth);
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
    //        _segments.emplace_back(x, y);
    //    }
    //    _segments.emplace_back(points.back());
    //}
    //else
    //{
    //    _segments = _trackPoints;
    //}
}

void CompSegment::GenMesh()
{
    std::vector<GLMesh::Vertex> points;
    std::vector<uint>           indexs;
    for (auto i = 0; i != _segments.size() - 1; ++i)
    {
        auto & a = _segments.at(i    );
        auto & b = _segments.at(i + 1);
        auto dir = glm::vec2(+(b -a).y, -(b -a).x);
        auto offset = glm::normalize(dir) * _width;

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

        points.emplace_back(a + offset, _color);
        points.emplace_back(b + offset, _color);
        points.emplace_back(b - offset, _color);
        points.emplace_back(a - offset, _color);
    }
    _mesh->Update(points, indexs);
}

void CompSegment::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    _update |= kSegment;
    AddState(StateEnum::kUpdate, true);
    _trackPoints.at(index) = point;
}

void CompSegment::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    _update |= kSegment;
    AddState(StateEnum::kUpdate, true);
    _trackPoints.insert(std::next(_trackPoints.begin(), index), point);
}

void CompSegment::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    if (_trackPoints.size() > 2)
    {
        _update |= kSegment;
        AddState(StateEnum::kUpdate, true);
        _trackPoints.erase(std::next(_trackPoints.begin(), index));
    }
}

