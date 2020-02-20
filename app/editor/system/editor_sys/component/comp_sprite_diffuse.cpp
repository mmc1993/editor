#include "comp_sprite_diffuse.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompSpriteDiffuse::CompSpriteDiffuse()
    : mSize(0.0f, 0.0f)
    , mAnchor(0.5f, 0.5f)
    , mUpdate(kTexture | kTrackPoint)
    , mThreshold(0)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({}, {}, RawMesh::Vertex::kV |
                        RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_SPRITE_DIFFUSE);

    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompSpriteDiffuse::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mImage.Check() && mNoise.Check())
    {
        Update();

        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mPairImages.emplace_back("texture0", mImage.Instance<RawTexture>()->GetImage());
        command.mPairImages.emplace_back("texture1", mNoise.Instance<RawTexture>()->GetImage());

        command.mBlendSrc = GL_SRC_ALPHA;
        command.mBlendDst = GL_ONE_MINUS_SRC_ALPHA;

        command.mCallback = std::bind(
            &CompSpriteDiffuse::OnDrawCallback,
            CastPtr<CompSpriteDiffuse>(shared_from_this()),
            std::placeholders::_1, std::placeholders::_2);

        command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;

        canvas->Post(command);
    }
}

const std::string & CompSpriteDiffuse::GetName()
{
    static const std::string name = "SpriteDiffuse";
    return name;
}

void CompSpriteDiffuse::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mSize);
    tools::Serialize(os, mAnchor);
    mImage.EncodeBinary(os, project);
    mNoise.EncodeBinary(os, project);
}

void CompSpriteDiffuse::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mAnchor);
    mImage.DecodeBinary(is, project);
    mNoise.DecodeBinary(is, project);
}

bool CompSpriteDiffuse::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
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

std::vector<Component::Property> CompSpriteDiffuse::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Image", &mImage, std::vector<uint>{ (uint)Res::TypeEnum::kImg });
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Noise", &mNoise, std::vector<uint>{ (uint)Res::TypeEnum::kImg });
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4, "Threshold",     &mThreshold);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",       &mAnchor);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",         &mSize);
    return std::move(props);
}

void CompSpriteDiffuse::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (mUpdate & kTexture)
        {
            if (mSize.x == 0.0f || mSize.y == 0.0f)
            {
                mSize.x = (float)mImage.Instance<RawTexture>()->GetW();
                mSize.y = (float)mImage.Instance<RawTexture>()->GetH();
            }
        }

        if (mUpdate & kTrackPoint)
        {
            mTrackPoints.at(0).x = -mSize.x * mAnchor.x;
            mTrackPoints.at(0).y = -mSize.y * mAnchor.y;
            mTrackPoints.at(1).x = mSize.x * (1 - mAnchor.x);
            mTrackPoints.at(1).y = -mSize.y * mAnchor.y;
            mTrackPoints.at(2).x = mSize.x * (1 - mAnchor.x);
            mTrackPoints.at(2).y = mSize.y * (1 - mAnchor.y);
            mTrackPoints.at(3).x = -mSize.x * mAnchor.x;
            mTrackPoints.at(3).y = mSize.y * (1 - mAnchor.y);

            std::vector<RawMesh::Vertex> vertexs;
            auto & offset = mImage.Instance<RawTexture>()->GetOffset();
            vertexs.emplace_back(mTrackPoints.at(0), glm::vec2(offset.x, offset.y));
            vertexs.emplace_back(mTrackPoints.at(1), glm::vec2(offset.z, offset.y));
            vertexs.emplace_back(mTrackPoints.at(2), glm::vec2(offset.z, offset.w));
            vertexs.emplace_back(mTrackPoints.at(3), glm::vec2(offset.x, offset.w));
            mMesh->Update(vertexs, { 0, 1, 2, 0, 2, 3 });
        }
        mUpdate = 0;
    }
}

void CompSpriteDiffuse::OnDrawCallback(const RenderPipline::RenderCommand & command, uint texturePos)
{
    auto & foward = (const RenderPipline::FowardCommand &)command;
    //foward.mProgram->BindUniformNumber("threshold_",mThreshold.x);
    foward.mProgram->BindUniformVector("params_", mThreshold);
}

void CompSpriteDiffuse::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
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
