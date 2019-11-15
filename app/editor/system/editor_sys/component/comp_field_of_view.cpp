#include "comp_field_of_view.h"
#include "../editor_sys.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"

CompFieldOfView::CompFieldOfView()
{
    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({}, {}, GLMesh::Vertex::kV | 
                        GLMesh::Vertex::kC);

    _program = Global::Ref().mRawSys->Get<GLProgram>(tools::GL_PROGRAM_SOLID_FILL);
}

const std::string & CompFieldOfView::GetName()
{
    return "FieldOfView";
}

void CompFieldOfView::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _url);
}

void CompFieldOfView::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _url);
}

bool CompFieldOfView::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    return false;
}

void CompFieldOfView::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    Update();
}

void CompFieldOfView::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        auto track = Global::Ref().mEditorSys->GetProject()->GetRoot();
        for (auto & name : tools::Split(_url, "/"))
        {
            track = track->GetObject(name);
            ASSERT_LOG(track != nullptr, name.c_str());
        }
        _polygons = track->GetComponentsInChildren<CompPolygon>();
    }
}
