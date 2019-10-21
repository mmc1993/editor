#include "comp_sprite.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompSprite::CompSprite()
    : _size(0.0f, 0.0f)
    , _anchor(0.5f, 0.5f)
{
    _trackPoints.resize(4);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({}, {}, GLMesh::Vertex::kV | GLMesh::Vertex::kUV);

    _program = std::create_ptr<GLProgram>();
    _program->Init(tools::GL_PROGRAM_SPRITE);

    AddState(StateEnum::kUpdate, true);
}

void CompSprite::OnAdd()
{ }

void CompSprite::OnDel()
{ }

void CompSprite::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (!_url.empty())
    {
        Update();

        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mCallback   = std::bind(&CompSprite::OnRenderCallback, this, 
                                        std::placeholders::_1, 
                                        std::placeholders::_2);
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
    tools::Serialize(os, _url);
    tools::Serialize(os, _size);
    tools::Serialize(os, _anchor);
}

void CompSprite::DecodeBinary(std::ifstream & is)
{
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _size);
    tools::Deserialize(is, _anchor);
}

bool CompSprite::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    if (title == "Url" || title == "Size" || title == "Anchor")
    {
        AddState(StateEnum::kUpdate, true);
    }
    return true;
}

std::vector<Component::Property> CompSprite::CollectProperty()
{
    return {
        { interface::Serializer::StringValueTypeEnum::kAsset,   "Url",    &_url    },
        { interface::Serializer::StringValueTypeEnum::kVector2, "Size",   &_size   },
        { interface::Serializer::StringValueTypeEnum::kVector2, "Anchor", &_anchor }
    };
}

void CompSprite::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        _texture = Global::Ref().mRawSys->Get<GLTexture>(_url);
        _size.x = (float)_texture->GetW();
        _size.y = (float)_texture->GetH();
        _trackPoints.at(0).x = -_size.x *      _anchor.x;
        _trackPoints.at(0).y = -_size.y *      _anchor.y;
        _trackPoints.at(1).x =  _size.x * (1 - _anchor.x);
        _trackPoints.at(1).y = -_size.y *      _anchor.y;
        _trackPoints.at(2).x =  _size.x * (1 - _anchor.x);
        _trackPoints.at(2).y =  _size.y * (1 - _anchor.y);
        _trackPoints.at(3).x = -_size.x *      _anchor.x;
        _trackPoints.at(3).y =  _size.y * (1 - _anchor.y);

        auto & offset = _texture->GetOffset();

        std::vector<GLMesh::Vertex> vertexs;
        vertexs.emplace_back(_trackPoints.at(0), glm::vec2(offset.x, offset.y));
        vertexs.emplace_back(_trackPoints.at(1), glm::vec2(offset.z, offset.y));
        vertexs.emplace_back(_trackPoints.at(2), glm::vec2(offset.z, offset.w));
        vertexs.emplace_back(_trackPoints.at(3), glm::vec2(offset.x, offset.w));
        _mesh->Update(vertexs, {0, 1, 2, 0, 2, 3});
    }
}

void CompSprite::OnRenderCallback(const interface::RenderCommand & command, uint * pos)
{
    auto & forward = (const interface::FowardCommand &)(command);
    forward.mProgram->BindUniformVector("uniform_size",   _size);
    forward.mProgram->BindUniformVector("uniform_anchor", _anchor);
}

void CompSprite::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
}

void CompSprite::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
}

void CompSprite::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
}

