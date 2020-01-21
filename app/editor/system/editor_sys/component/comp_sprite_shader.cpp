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

            if (sPoints.empty())
            {
                for (auto y = 0; y != 10; ++y)
                {
                    auto py = glm::lerp(mTrackPoints.at(2).y, mTrackPoints.at(0).y, y / 10.0f);
                    for (auto x = 0; x != 10; ++x)
                    {
                        auto px = glm::lerp(mTrackPoints.at(2).x, mTrackPoints.at(0).x, x / 10.0f);
                        mTrackPoints.emplace_back(px, py);
                        sPoints.emplace_back(px, py);
                    }
                }
            }

            std::vector<RawMesh::Vertex> vertexs;
            auto & offset = mTex.Instance<RawTexture>()->GetOffset();
            vertexs.emplace_back(mTrackPoints.at(0), glm::vec2(offset.x, offset.y));
            vertexs.emplace_back(mTrackPoints.at(1), glm::vec2(offset.z, offset.y));
            vertexs.emplace_back(mTrackPoints.at(2), glm::vec2(offset.z, offset.w));
            vertexs.emplace_back(mTrackPoints.at(3), glm::vec2(offset.x, offset.w));
            mMesh->Update(vertexs, { 0, 1, 2, 0, 2, 3 });
        }
        mUpdate = 0;
    }
}

void CompSpriteShader::OnDrawCallback(const RenderPipline::RenderCommand & command, uint texturePos)
{
    auto & foward = (const RenderPipline::FowardCommand &)command;
    auto & min = mTrackPoints.at(0);
    auto & max = mTrackPoints.at(2);
    auto & mid = mTrackPoints.back();
    auto w = max.x - min.x;
    auto h = max.y - min.y;
    glm::vec2 coord(
        (mid.x - min.x) / w, 
        (mid.y - min.y) / h);
    foward.mProgram->BindUniformVector("coord_", coord);
}

void CompSpriteShader::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    glm::vec2 min(0);
    glm::vec2 max(0);
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
        {
            mTrackPoints.at(index).x = glm::clamp(point.x, mTrackPoints.at(0).x, mTrackPoints.at(2).x);
            mTrackPoints.at(index).y = glm::clamp(point.y, mTrackPoints.at(0).y, mTrackPoints.at(2).y);
            glm::vec2 center(
                0.5f * (mTrackPoints.at(2).x - mTrackPoints.at(0).x),
                0.5f * (mTrackPoints.at(2).y - mTrackPoints.at(0).y));

            for (auto i = 5; i != sPoints.size(); ++i)
            {
                auto & point = sPoints.at(i);

                auto offset = mTrackPoints.at(index) - center;

                auto direct = point - mTrackPoints.at(index);

                auto normal = point - center;

                auto diff = direct * glm::dot(glm::normalize(normal), glm::normalize(offset));

                mTrackPoints.at(i + 5) = point + diff;
            }
        }
        break;
    }

    if (index < 4)
    {
        mSize.x = max.x - min.x;
        mSize.y = max.y - min.y;

        auto coord = GetOwner()->LocalToParent(mSize * mAnchor + min);
        GetOwner()->GetTransform()->Position(coord.x, coord.y);

        mUpdate |= kTrackPoint;

        AddState(StateEnum::kUpdate, true);
    }
}
