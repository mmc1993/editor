#include "comp_sprite.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompSprite::CompSprite()
    : _size(0.0f, 0.0f)
    , _anchor(0.5f, 0.5f)
    , _update(kTexture | kTrackPoint)
{
    _trackPoints.resize(4);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({},{}, GLMesh::Vertex::kV | 
                       GLMesh::Vertex::kUV);

    _program = std::create_ptr<GLProgram>();
    _program->Init(tools::GL_PROGRAM_SPRITE);

    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompSprite::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (_tex.Check())
    {
        Update();

        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mTextures.push_back(std::make_pair("uniform_texture", _tex.Instance<GLTexture>()));
        canvas->Post(command);
    }
}

const std::string & CompSprite::GetName()
{
    static const std::string name = "Sprite";
    return name;
}

void CompSprite::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, _size);
    tools::Serialize(os, _anchor);
    _tex.EncodeBinary(os, project);
}

void CompSprite::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, _size);
    tools::Deserialize(is, _anchor);
    _tex.DecodeBinary(is, project);
}

bool CompSprite::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Tex")
    {
        _update |= kTexture 
                | kTrackPoint;
    }
    if (title == "Size" || title == "Anchor")
    {
        _update |= kTrackPoint;
    }
    return true;
}

std::vector<Component::Property> CompSprite::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Tex", &_tex, (uint)Res::TypeEnum::kImg);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size", &_size);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor", &_anchor);
    return std::move(props);
}

void CompSprite::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (_update & kTexture)
        {
            _size.x = (float)_tex.Instance<GLTexture>()->GetW();
            _size.y = (float)_tex.Instance<GLTexture>()->GetH();
        }

        if (_update & kTrackPoint)
        {
            _trackPoints.at(0).x = -_size.x *      _anchor.x;
            _trackPoints.at(0).y = -_size.y *      _anchor.y;
            _trackPoints.at(1).x =  _size.x * (1 - _anchor.x);
            _trackPoints.at(1).y = -_size.y *      _anchor.y;
            _trackPoints.at(2).x =  _size.x * (1 - _anchor.x);
            _trackPoints.at(2).y =  _size.y * (1 - _anchor.y);
            _trackPoints.at(3).x = -_size.x *      _anchor.x;
            _trackPoints.at(3).y =  _size.y * (1 - _anchor.y);

            auto & offset = _tex.Instance<GLTexture>()->GetOffset();
            std::vector<GLMesh::Vertex> vertexs;
            vertexs.emplace_back(_trackPoints.at(0), glm::vec2(offset.x, offset.y));
            vertexs.emplace_back(_trackPoints.at(1), glm::vec2(offset.z, offset.y));
            vertexs.emplace_back(_trackPoints.at(2), glm::vec2(offset.z, offset.w));
            vertexs.emplace_back(_trackPoints.at(3), glm::vec2(offset.x, offset.w));
            _mesh->Update(vertexs, { 0, 1, 2, 0, 2, 3 });
        }
        _update = 0;
    }
}

void CompSprite::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
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

    _update |= kTrackPoint;

    AddState(StateEnum::kUpdate, true);
}

void CompSprite::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
}

void CompSprite::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
}

