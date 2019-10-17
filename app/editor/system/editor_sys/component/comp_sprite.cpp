#include "comp_sprite.h"
#include "../../interface/render.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompSprite::CompSprite()
    : _size(0.0f, 0.0f)
    , _anchor(0.5f, 0.5f)
{
    _trackPoints.resize(4);

    //_command = std::create_ptr<interface::FowardCommand>();

    //_command->mMesh = std::create_ptr<GLMesh>();
    //_command->mMesh->Init({}, {}, GLMesh::Vertex::kV);

    //_command->mProgram = std::create_ptr<GLProgram>();
    //_command->mProgram->Init(tools::GL_PROGRAM_SPRITE);

    //_command->mCallback = std::bind(&CompSprite::OnRenderCallback, this, std::placeholders::_1);
}

void CompSprite::OnAdd()
{ }

void CompSprite::OnDel()
{ }

void CompSprite::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    //_command->mTransform = canvas->GetMatrixStack().GetM();
    //_command->mCallback = std::bind(&CompSprite::OnRenderCallback, this, std::placeholders::_1);
    //canvas->Post(_command);
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

    if (!_url.empty())
    {
        //_material = CastPtr<GLMaterial>(Global::Ref().mRawSys->Import(_url));
        //ASSERT_LOG(!_material->GetTextures().empty(), _url.c_str());
        //UpdateTrackPoints();
    }
}

bool CompSprite::OnModifyProperty(const std::any & value, const std::any & backup, const std::string & title)
{
    //if (title == "Url")
    //{
    //    auto & url = std::any_cast<const std::string &>(backup);
    //    auto ptr = CastPtr<GLMaterial>(Global::Ref().mRawSys->Import(url));
    //    ASSERT_LOG(!ptr->GetTextures().empty(), url.c_str());
    //    if (_material == nullptr)
    //    {
    //        _size.x = (float)ptr->GetTextures().at(0).mTex->GetW();
    //        _size.y = (float)ptr->GetTextures().at(0).mTex->GetH();
    //    }
    //    _material = ptr;
    //}
    //UpdateTrackPoints();
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

void CompSprite::UpdateTrackPoints()
{
    _trackPoints.at(0).x = -_size.x * _anchor.x;
    _trackPoints.at(0).y = -_size.y * _anchor.y;

    _trackPoints.at(1).x = -_size.x *      _anchor.x;
    _trackPoints.at(1).y =  _size.y * (1 - _anchor.y);

    _trackPoints.at(2).x =  _size.x * (1 - _anchor.x);
    _trackPoints.at(2).y =  _size.y * (1 - _anchor.y);

    _trackPoints.at(3).x =  _size.x * (1 - _anchor.x);
    _trackPoints.at(3).y = -_size.y *      _anchor.y;
}

void CompSprite::OnRenderCallback(const interface::RenderCommand & command)
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

