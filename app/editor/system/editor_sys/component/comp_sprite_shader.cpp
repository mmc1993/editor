#include "comp_sprite_shader.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

static std::vector<glm::vec2> sPoints;

CompSpriteShader::CompSpriteShader()
    : mSize(0.0f, 0.0f)
    , mAnchor(0.5f, 0.5f)
    , mUpdate(kTexture | kTrackPoint)
{
    mTrackPoints.resize(5);
    mTrackPoints.back().x = 0;
    mTrackPoints.back().y = 0;

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_SPRITE_SHADER0);

    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompSpriteShader::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mTex.Check())
    {
        Update();

        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mPairImages.emplace_back("texture0", mTex.Instance<RawTexture>()->GetImage());

        command.mBlendSrc = GL_SRC_ALPHA;
        command.mBlendDst = GL_ONE_MINUS_SRC_ALPHA;

        command.mCallback = std::bind(
            &CompSpriteShader::OnDrawCallback, 
            CastPtr<CompSpriteShader>(shared_from_this()),
            std::placeholders::_1, std::placeholders::_2);

        command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;

        canvas->Post(command);
    }
}

const std::string & CompSpriteShader::GetName()
{
    static const std::string name = "SpriteShader";
    return name;
}

void CompSpriteShader::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mSize);
    tools::Serialize(os, mAnchor);
    tools::Serialize(os, mTrackPoints);
    mTex.EncodeBinary(os, project);
}

void CompSpriteShader::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mAnchor);
    tools::Deserialize(is, mTrackPoints);
    mTex.DecodeBinary(is, project);
}

bool CompSpriteShader::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Tex")
    {
        mUpdate |= kTexture 
                |  kTrackPoint;
    }
    if (title == "Size" || title == "Anchor")
    {
        mUpdate |= kTrackPoint;
    }
    return true;
}

std::vector<Component::Property> CompSpriteShader::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Tex",      &mTex,      std::vector<uint>{ (uint)Res::TypeEnum::kImg });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",     &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4,  "Radian",   &mRadian);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompSpriteShader::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (mUpdate & kTexture)
        {
            mSize.x = (float)mTex.Instance<RawTexture>()->GetW();
            mSize.y = (float)mTex.Instance<RawTexture>()->GetH();
        }

        if (mUpdate & kTrackPoint)
        {
            mTrackPoints.at(0).x = -mSize.x *      mAnchor.x;
            mTrackPoints.at(0).y = -mSize.y *      mAnchor.y;
            mTrackPoints.at(1).x =  mSize.x * (1 - mAnchor.x);
            mTrackPoints.at(1).y = -mSize.y *      mAnchor.y;
            mTrackPoints.at(2).x =  mSize.x * (1 - mAnchor.x);
            mTrackPoints.at(2).y =  mSize.y * (1 - mAnchor.y);
            mTrackPoints.at(3).x = -mSize.x *      mAnchor.x;
            mTrackPoints.at(3).y =  mSize.y * (1 - mAnchor.y);

            mTrackPoints.at(4).x = glm::clamp(mTrackPoints.at(4).x, mTrackPoints.at(0).x, mTrackPoints.at(2).x);
            mTrackPoints.at(4).y = glm::clamp(mTrackPoints.at(4).y, mTrackPoints.at(0).y, mTrackPoints.at(2).y);

            auto & offset = mTex.Instance<RawTexture>()->GetOffset();
            glm::vec2 center;
            center.x = glm::lerp(offset.x, offset.z, 0.5f);
            center.y = glm::lerp(offset.y, offset.w, 0.5f);
            
            std::vector<RawMesh::Vertex> vertexs;
            vertexs.emplace_back(mTrackPoints.at(0), glm::vec2(offset.x, offset.y));
            vertexs.emplace_back(mTrackPoints.at(1), glm::vec2(offset.z, offset.y));
            vertexs.emplace_back(mTrackPoints.at(2), glm::vec2(offset.z, offset.w));
            vertexs.emplace_back(mTrackPoints.at(3), glm::vec2(offset.x, offset.w));
            vertexs.emplace_back(mTrackPoints.at(4), center);

            mMesh->Update(vertexs, { 0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4 });
        }
        mUpdate = 0;
    }
}

void CompSpriteShader::OnDrawCallback(const RenderPipline::RenderCommand & command, uint texturePos)
{
    auto & foward = (const RenderPipline::FowardCommand &)command;
    auto & min = mTrackPoints.at(0);
    auto & max = mTrackPoints.at(2);
    auto & mid = mTrackPoints.at(4);
    auto w = max.x - min.x;
    auto h = max.y - min.y;
    glm::vec2 coord(
        1.0f - (mid.x - min.x) / w, 
        1.0f - (mid.y - min.y) / h);
    foward.mProgram->BindUniformVector("target_", coord);
    foward.mProgram->BindUniformNumber("radian_", 360 * mRadian.a);
}

void CompSpriteShader::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    glm::vec2 min = mTrackPoints.at(0);
    glm::vec2 max = mTrackPoints.at(2);
    switch (index)
    {
    case 0:
        min.x = std::min(point.x, mTrackPoints.at(2).x);
        min.y = std::min(point.y, mTrackPoints.at(2).y);
        max.x = mTrackPoints.at(2).x;
        max.y = mTrackPoints.at(2).y;
        break;
    case 1:
        min.x = mTrackPoints.at(0).x;
        min.y = std::min(point.y, mTrackPoints.at(3).y);
        max.x = std::max(point.x, mTrackPoints.at(3).x);
        max.y = mTrackPoints.at(2).y;
        break;
    case 2:
        min.x = mTrackPoints.at(0).x;
        min.y = mTrackPoints.at(0).y;
        max.x = std::max(point.x, mTrackPoints.at(0).x);
        max.y = std::max(point.y, mTrackPoints.at(0).y);
        break;
    case 3:
        min.x = std::min(point.x, mTrackPoints.at(1).x);
        min.y = mTrackPoints.at(0).y;
        max.x = mTrackPoints.at(2).x;
        max.y = std::max(point.y, mTrackPoints.at(1).y);
        break;
    case 4:
        mTrackPoints.at(index).x = glm::clamp(point.x, mTrackPoints.at(0).x, mTrackPoints.at(2).x);
        mTrackPoints.at(index).y = glm::clamp(point.y, mTrackPoints.at(0).y, mTrackPoints.at(2).y);
        break;
    }

    mUpdate |= kTrackPoint;
    mSize.x = max.x - min.x;
    mSize.y = max.y - min.y;

    auto coord = GetOwner()->LocalToParent(mSize * mAnchor + min);
    GetOwner()->GetTransform()->Position(coord.x, coord.y);

    AddState(StateEnum::kUpdate, true);
}
