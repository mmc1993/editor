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
    , _update(kFont | kMesh)
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

void CompText::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _url);
    tools::Serialize(os, _text);
    tools::Serialize(os, _size);
    tools::Serialize(os, _anchor);

    tools::Serialize(os, _color);
    tools::Serialize(os, _outColor);
    tools::Serialize(os, _outDelta);
}

void CompText::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _url);
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
    if (title == "URL")
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
        command.mTextures.emplace_back("uniform_texture", _font->RefTexture());
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
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kAsset,   "URL",       &_url);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kString,  "Text",      &_text);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2, "Size",      &_size);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2, "Anchor",    &_anchor);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kColor4,  "Color",     &_color);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kColor4,  "OutColor",  &_outColor);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat,   "OutDelta",  &_outDelta);

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

    std::vector<GLMesh::Vertex> points;

    auto codes = _font->RefWord(_text);
    auto texW  = _font->RefTexture()->GetW();
    auto texH  = _font->RefTexture()->GetH();
    auto posX  = 0.0f;
    auto posY  = 0.0f;
    auto lineH = 0.0f;
    for (auto i = 0; i != codes.size(); ++i)
    {
        const auto & word = _font->RefWord(codes.at(i));
        auto wordW = (word.mUV.z - word.mUV.x) * texW;
        auto wordH = (word.mUV.w - word.mUV.y) * texH;
        //  ½Ø¶Ï¿í¶È
        if (posX + wordW  > _size.x)
        {
            posX  = 0.0f;
            posY += lineH;
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
