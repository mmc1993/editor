#include "comp_text.h"
#include "../editor_sys.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompText::CompText()
    : _text("M")
    , _color(1.0f)
    , _anchor(0.0f)
    , _size(100.0f)
    , _outDelta(0)
    , _outColor(0)
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

void CompText::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    _fnt.EncodeBinary(os, project);

    tools::Serialize(os, _text);
    tools::Serialize(os, _size);
    tools::Serialize(os, _anchor);

    tools::Serialize(os, _color);
    tools::Serialize(os, _outColor);
    tools::Serialize(os, _outDelta);
}

void CompText::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    _fnt.DecodeBinary(is, project);
    
    tools::Deserialize(is, _text);
    tools::Deserialize(is, _size);
    tools::Deserialize(is, _anchor);

    tools::Deserialize(is, _color);
    tools::Deserialize(is, _outColor);
    tools::Deserialize(is, _outDelta);
}

bool CompText::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompText::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        if (_fnt.Check()) { UpdateMesh(); }
    }

    if (_fnt.Check())
    {
        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mTextures.emplace_back("uniform_texture",
                _fnt.Instance<GLFont>()->RefTexture());
        command.mCallback = std::bind(
            &CompText::OnDrawCallback, this,
            std::placeholders::_1,
            std::placeholders::_2);
        canvas->Post(command);
    }
}
std::vector<Component::Property> CompText::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Font",      &_fnt, (uint)(Res::TypeEnum::kFnt));
    props.emplace_back(UIParser::StringValueTypeEnum::kString,  "Text",      &_text);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",      &_size);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",    &_anchor);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4,  "Color",     &_color);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4,  "OutColor",  &_outColor);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat,   "OutDelta",  &_outDelta);

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

    AddState(StateEnum::kUpdate, true);
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

    std::vector<GLMesh::Vertex> points;

    auto codes = _fnt.Instance<GLFont>()->RefWord(_text);
    auto texW  = _fnt.Instance<GLFont>()->RefTexture()->GetW();
    auto texH  = _fnt.Instance<GLFont>()->RefTexture()->GetH();
    auto posX  = 0.0f;
    auto posY  = 0.0f;
    auto lineH = 0.0f;
    for (auto i = 0; i != codes.size(); ++i)
    {
        const auto & word = _fnt.Instance<GLFont>()->RefWord(codes.at(i));
        auto wordW = (word.mUV.z - word.mUV.x) * texW;
        auto wordH = (word.mUV.w - word.mUV.y) * texH;
        //  ½Ø¶Ï¿í¶È
        if (posX + wordW  > _size.x)
        {
            posX = 0.0f; posY += lineH;
        }
        //  ½Ø¶Ï¸ß¶È
        if (posX + wordW > _size.x ||
            posY + wordH > _size.y)
        {
            break;
        }
        lineH = std::max(lineH, wordH);

        auto x = _trackPoints.at(3).x + posX;
        auto y = _trackPoints.at(3).y - posY;
        glm::vec2 p0(x,         y);
        glm::vec2 p1(x + wordW, y);
        glm::vec2 p2(x + wordW, y - wordH);
        glm::vec2 p3(x,         y - wordH);

        points.emplace_back(p0, _color, glm::vec2(word.mUV.x, word.mUV.w));
        points.emplace_back(p1, _color, glm::vec2(word.mUV.z, word.mUV.w));
        points.emplace_back(p2, _color, glm::vec2(word.mUV.z, word.mUV.y));

        points.emplace_back(p0, _color, glm::vec2(word.mUV.x, word.mUV.w));
        points.emplace_back(p2, _color, glm::vec2(word.mUV.z, word.mUV.y));
        points.emplace_back(p3, _color, glm::vec2(word.mUV.x, word.mUV.y));

        posX += wordW;
    }

    _mesh->Update(points, { });
}

void CompText::OnDrawCallback(const interface::RenderCommand & command, uint pos)
{ 
    auto forward = (const interface::FowardCommand &)(command);
    forward.mProgram->BindUniformNumber("out_delta_", _outDelta);
    forward.mProgram->BindUniformVector("out_color_", _outColor);
}
