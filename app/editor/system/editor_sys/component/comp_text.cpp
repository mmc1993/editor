#include "comp_text.h"
#include "../editor_sys.h"
#include "comp_transform.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompText::CompText()
    : _text("M")
    , _color(1.0f)
    , _anchor(0.0f)
    , _size(100.0f)
    , _update(kFont | kMesh)
{
    _trackPoints.resize(4);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({},{}, GLMesh::Vertex::kV | 
                       GLMesh::Vertex::kC | 
                       GLMesh::Vertex::kUV);
    _program = Global::Ref().mRawSys->Get<GLProgram>(tools::GL_PROGRAM_FONT_SDF);

    AddState(StateEnum::kModifyTrackPoint, true);
}

const std::string & CompText::GetName()
{
    static const std::string name = "Text";
    return name;
}

void CompText::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _url);
    tools::Serialize(os, _text);
    tools::Serialize(os, _size);
    tools::Serialize(os, _color);
    tools::Serialize(os, _anchor);
}

void CompText::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _text);
    tools::Deserialize(is, _size);
    tools::Deserialize(is, _color);
    tools::Deserialize(is, _anchor);
}

bool CompText::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Font")
    {
        _update |= kFont;
    }
    _update |= kMesh;
    return true;
}

void CompText::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        if (!_url.empty())
        {
            if (_update & kFont) { UpdateFont(); }
            if (_update & kMesh) { UpdateMesh(); }
        }
        _update = 0;
    }

    if (!_url.empty())
    {
        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mTextures.emplace_back("uniform_texture", _font->RefImage());
        canvas->Post(command);
    }
}

std::vector<Component::Property> CompText::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kAsset,     "Font",          &_url);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kString,     "Text",         &_text);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2,    "Size",         &_size);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kColor4,     "Color",        &_color);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2,    "Anchor",       &_anchor);
    return std::move(props);
}

void CompText::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{ 
    glm::vec2 min, max;
    switch (index)
    {
    case 0:
        min.x = std::min(point.x, _trackPoints.at(2).x);
        min.y = std::min(point.y, _trackPoints.at(2).y);
        max.x = _trackPoints.at(2).x;
        max.y = _trackPoints.at(2).y;
        break;
    case 1:
        min.x = _trackPoints.at(0).x;
        min.y = std::min(point.y, _trackPoints.at(3).y);
        max.x = std::max(point.x, _trackPoints.at(3).x);
        max.y = _trackPoints.at(2).y;
        break;
    case 2:
        min.x = _trackPoints.at(0).x;
        min.y = _trackPoints.at(0).y;
        max.x = std::max(point.x, _trackPoints.at(0).x);
        max.y = std::max(point.y, _trackPoints.at(0).y);
        break;
    case 3:
        min.x = std::min(point.x, _trackPoints.at(1).x);
        min.y = _trackPoints.at(0).y;
        max.x = _trackPoints.at(2).x;
        max.y = std::max(point.y, _trackPoints.at(1).y);
        break;
    }

    _size.x = max.x - min.x;
    _size.y = max.y - min.y;

    auto coord = GetOwner()->LocalToParent(_size * _anchor + min);
    GetOwner()->GetTransform()->Position(coord.x, coord.y);

    _update |= kMesh;

    AddState(StateEnum::kUpdate, true);
}

void CompText::UpdateFont()
{
    _font = Global::Ref().mRawSys->Get<GLFont>(_url);
}

void CompText::UpdateMesh()
{
    _trackPoints.at(0).x = -_size.x *      _anchor.x;
    _trackPoints.at(0).y = -_size.y *      _anchor.y;
    _trackPoints.at(1).x =  _size.x * (1 - _anchor.x);
    _trackPoints.at(1).y = -_size.y *      _anchor.y;
    _trackPoints.at(2).x =  _size.x * (1 - _anchor.x);
    _trackPoints.at(2).y =  _size.y * (1 - _anchor.y);
    _trackPoints.at(3).x = -_size.x *      _anchor.x;
    _trackPoints.at(3).y =  _size.y * (1 - _anchor.y);

    auto codes = _font->RefWord("Hello World");
    int a = 0;
}
