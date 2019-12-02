#include "comp_sprite.h"
#include "../../raw_sys/raw.h"
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
    if (!_url.empty())
    {
        Update();

        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mTextures.push_back(std::make_pair("uniform_texture", _texture));
        canvas->Post(command);
    }
}

const std::string & CompSprite::GetName()
{
    static const std::string name = "Sprite";
    return name;
}

void CompSprite::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _url);
    tools::Serialize(os, _size);
    tools::Serialize(os, _anchor);
}

void CompSprite::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _size);
    tools::Deserialize(is, _anchor);
}

bool CompSprite::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Url")
    {
        _update |= kTexture;
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
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kAsset, "Url", &_url);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2, "Size", &_size);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kVector2, "Anchor", &_anchor);
    return std::move(props);
}

void CompSprite::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (_update & kTexture)
        {
            auto first = _texture == nullptr;
            _texture = Global::Ref().mRawSys->Get<GLTexture>(_url);
            if (first)
            {
                _size.x = (float)_texture->GetW();
                _size.y = (float)_texture->GetH();
            }
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

            auto & offset=_texture->GetOffset();
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

