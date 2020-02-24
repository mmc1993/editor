#include "comp_noise_perlin.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompNoisePerlin::CompNoisePerlin()
    : mSize(0.0f, 0.0f)
    , mAnchor(0.5f, 0.5f)
    , mCellWH(0.0f, 0.0f)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({}, {}, RawMesh::Vertex::kV |
                        RawMesh::Vertex::kUV);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_NOISE_PERLIN);
}

void CompNoisePerlin::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mSize.x   != 0.0f && mSize.y   != 0.0f && 
        mCellWH.x != 0.0f && mCellWH.y != 0.0f)
    {
        Update();

        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mTransform  = canvas->GetMatrixStack().GetM();

        command.mCallback = std::bind(
            &CompNoisePerlin::OnDrawCallback,
            CastPtr<CompNoisePerlin>(shared_from_this()),
            std::placeholders::_1, std::placeholders::_2);

        canvas->Post(command);
    }
}

const std::string & CompNoisePerlin::GetName()
{
    static const std::string name = "NoisePerlin";
    return name;
}

void CompNoisePerlin::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mSize);
    tools::Serialize(os, mCellWH);
    tools::Serialize(os, mAnchor);
}

void CompNoisePerlin::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mCellWH);
    tools::Deserialize(is, mAnchor);
}

bool CompNoisePerlin::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

std::vector<Component::Property> CompNoisePerlin::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",   &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "CellWH", &mCellWH);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor", &mAnchor);
    return std::move(props);
}

void CompNoisePerlin::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        static const glm::vec2 s_GRADS[] = {
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
            glm::normalize(glm::vec2(tools::Random(0.0f, 1.0f), tools::Random(0.0f, 1.0f))), // * std::sqrt(glm::dot(mCellWH, mCellWH)),
        };
        mGrads.clear();
        auto xnum = (int)(mSize.x / mCellWH.x) + 1;
        auto ynum = (int)(mSize.y / mCellWH.y) + 1;
        for (auto i = 0; i != xnum * ynum; ++i)
        {
            auto size  = std::length(s_GRADS);
            auto index = tools::Random(0, size - 1);
            mGrads.emplace_back(s_GRADS[index]);
            //mGrads.emplace_back(tools::Random(0.0f, 1.0f), 0.0f);
        }

        //mGrads.emplace_back(s_GRADS[0]);
        //mGrads.emplace_back(s_GRADS[0]);
        //mGrads.emplace_back(s_GRADS[1]);
        //mGrads.emplace_back(s_GRADS[1]);
        //mGrads.emplace_back(s_GRADS[1]);
        //mGrads.emplace_back(s_GRADS[1]);


        //mGrads.emplace_back(glm::vec2(0.0f, 0.0f));
        //mGrads.emplace_back(glm::vec2(0.3f, 0.3f));
        //mGrads.emplace_back(glm::vec2(0.8f, 0.1f));
        //mGrads.emplace_back(glm::vec2(0.8f, 0.5f));
        //mGrads.emplace_back(glm::vec2(0.1f, 0.10));
        //mGrads.emplace_back(glm::vec2(0.9f, 0.0f));
        //mGrads.emplace_back(glm::vec2(0.5f, 0.5f));
        //mGrads.emplace_back(glm::vec2(0.2f, 0.10));
        //mGrads.emplace_back(glm::vec2(0.7f, 0.0f));





        mTrackPoints.at(0).x = -mSize.x * mAnchor.x;
        mTrackPoints.at(0).y = -mSize.y * mAnchor.y;
        mTrackPoints.at(1).x =  mSize.x * (1 - mAnchor.x);
        mTrackPoints.at(1).y = -mSize.y * mAnchor.y;
        mTrackPoints.at(2).x =  mSize.x * (1 - mAnchor.x);
        mTrackPoints.at(2).y =  mSize.y * (1 - mAnchor.y);
        mTrackPoints.at(3).x = -mSize.x * mAnchor.x;
        mTrackPoints.at(3).y =  mSize.y * (1 - mAnchor.y);

        std::vector<RawMesh::Vertex> vertexs;
        vertexs.emplace_back(mTrackPoints.at(0), glm::vec2(0, 1));
        vertexs.emplace_back(mTrackPoints.at(1), glm::vec2(1, 1));
        vertexs.emplace_back(mTrackPoints.at(2), glm::vec2(1, 0));
        vertexs.emplace_back(mTrackPoints.at(3), glm::vec2(0, 0));
        mMesh->Update(vertexs, { 0, 1, 2, 0, 2, 3 });
    }
}

void CompNoisePerlin::OnDrawCallback(const RenderPipline::RenderCommand & command, uint texturePos)
{
    auto xnum = (int)mSize.y / mCellWH.y;
    auto ynum = (int)mSize.x / mCellWH.x;

    auto & foward = (const RenderPipline::FowardCommand &)command;
    foward.mProgram->BindUniformVector("grads_",  mGrads[0], mGrads.size());
    foward.mProgram->BindUniformVector("cellwh_", mCellWH);
    foward.mProgram->BindUniformVector("size_",   mSize);
    foward.mProgram->BindUniformNumber("xnum_",   xnum);
    foward.mProgram->BindUniformNumber("ynum_",   ynum);
}
