#include "comp_segment.h"
#include "comp_transform.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompSegment::CompSegment()
    : _smooth(0)
    , _width(1)
    , _color(1)
{
    _trackPoints.emplace_back(0, 0);

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
    tools::Serialize(os, _trackPoints);
}

void CompSegment::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _trackPoints);
}

bool CompSegment::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

std::vector<Component::Property> CompSegment::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Width", &_width);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Smooth", &_smooth);
    return std::move(props);
}

void CompSegment::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        std::vector<GLMesh::Vertex> points;
        for (auto i = 0; i != _trackPoints.size() - 1; ++i)
        {
            auto & a = _trackPoints.at(i    );
            auto & b = _trackPoints.at(i + 1);
            auto ab  = glm::normalize(b - a);
            glm::vec2 rAxis(ab.y * _width * 0.5f,
                           -ab.x * _width * 0.5f);
            glm::vec2 ps[] = {  a - rAxis, a + rAxis,
                                b + rAxis, b - rAxis, };
            points.emplace_back(ps[0], _color);
            points.emplace_back(ps[1], _color);
            points.emplace_back(ps[2], _color);
            points.emplace_back(ps[0], _color);
            points.emplace_back(ps[2], _color);
            points.emplace_back(ps[3], _color);
        }
        _mesh->Update(points, {});
    }
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

