#include "comp_segment.h"
#include "comp_transform.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompSegment::CompSegment()
{
    _trackPoints.resize(1);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({},{}, GLMesh::Vertex::kV | 
                       GLMesh::Vertex::kC);

    _program = std::create_ptr<GLProgram>();
    _program->Init(tools::GL_PROGRAM_SPRITE);

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
    static const std::string name = "Sprite";
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
    return Component::CollectProperty();
}

void CompSegment::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
    }
}

void CompSegment::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
}

void CompSegment::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
}

void CompSegment::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
}

