#include "comp_segment.h"
#include "comp_transform.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompSegment::CompSegment()
    : _color(1)
    , _width(1)
    , _stage(1)
    , _smooth(0)
{
    _trackPoints.emplace_back(0, 0 );
    _trackPoints.emplace_back(0, 500);

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
    tools::Serialize(os, _stage);
    tools::Serialize(os, _color);
    tools::Serialize(os, _smooth);
    tools::Serialize(os, _trackPoints);
}

void CompSegment::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _width);
    tools::Deserialize(is, _stage);
    tools::Deserialize(is, _color);
    tools::Deserialize(is, _smooth);
    tools::Deserialize(is, _trackPoints);
}

bool CompSegment::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Width")
    {
        _width = std::max(1.0f, std::any_cast<float>(newValue));
        return false;
    }
    else if (title == "Stage")
    {
        if (std::any_cast<float>(newValue) > 0)
        {
            _stage = std::any_cast<float>(newValue);
        }
        return false;
    }
    return true;
}

std::vector<Component::Property> CompSegment::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Width", &_width);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Stage", &_stage);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kColor4, "Color", &_color);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Smooth", &_smooth);
    return std::move(props);
}

void CompSegment::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        std::vector<glm::vec2> segments;
        GenSegm(_trackPoints, segments);
        GenMesh(segments);
    }
}

void CompSegment::GenSegm(const std::vector<glm::vec2> & segments, std::vector<glm::vec2> & output)
{
    auto & a    = segments.at(0);
    auto & b    = segments.at(1);
    auto sAixs  = glm::normalize(b - a);
    output.emplace_back(a);

    for (auto i = 0; i != segments.size() - 1; ++i)
    {
        auto & a = segments.at(i    );
        auto & b = segments.at(i + 1);

        auto stepLen = glm::length(b - a) / _stage;
        auto stepDir = glm::normalize(b - a);
        auto stepDirect = stepDir  * stepLen;

        for (auto s = 0; s != _stage; ++s)
        {
            auto dir = glm::normalize(b - output.back());
            sAixs    = glm::normalize(sAixs + dir);
            auto len = glm::dot(stepDirect, sAixs);
            output.emplace_back(sAixs * len + output.back());
        }
    }
}

void CompSegment::GenMesh(const std::vector<glm::vec2> & segments)
{
    std::vector<GLMesh::Vertex> points;
    std::vector<uint>           indexs;
    for (auto i = 0; i != segments.size() - 1; ++i)
    {
        auto & a = segments.at(i    );
        auto & b = segments.at(i + 1);
        auto dir = glm::vec2(+(b - a).y, -(b - a).x);
        auto offset = glm::normalize(dir) * _width * 0.5f;

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
    AddState(StateEnum::kUpdate, true);
    _trackPoints.at(index) = point;
}

void CompSegment::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _trackPoints.insert(std::next(_trackPoints.begin(), index), point);
}

void CompSegment::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    if (_trackPoints.size() > 1)
    {
        _trackPoints.erase(std::next(_trackPoints.begin(), index));
    }
}

