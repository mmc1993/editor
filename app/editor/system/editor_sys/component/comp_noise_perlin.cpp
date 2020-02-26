#include "comp_noise_perlin.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

glm::vec2 Random(const glm::vec2 & v)
{
    auto p = glm::vec2(glm::dot(v, glm::vec2(127.1f, 311.7f)),
                       glm::dot(v, glm::vec2(269.5f, 183.3f)));
    return -1.0f + 2.0f * glm::fract(glm::sin(p) * 43758.5453f);
}

float NoisePerlin(glm::vec2 p)
{
    auto i = glm::floor(p);
    auto f = glm::fract(p);

    float a = glm::dot(Random(i), f);
    float b = glm::dot(Random(i + glm::vec2(1, 0)), f - glm::vec2(1, 0));
    float c = glm::dot(Random(i + glm::vec2(0, 1)), f - glm::vec2(0, 1));
    float d = glm::dot(Random(i + glm::vec2(1, 1)), f - glm::vec2(1, 1));

    f = glm::smoothstep(0.0f, 1.0f, f);
    float v0 = glm::mix(a, b, f.x);
    float v1 = glm::mix(c, d, f.x);
    return glm::mix(v0, v1, f.y);
}

CompNoisePerlin::CompNoisePerlin()
    : mAnchor(0.5f, 0.5f)
    , mSize(0.0f, 0.0f)
    , mSpan(0.0f, 0.0f)
    , mFrequency(0.0f)
    , mAmplitude(0.0f)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({}, {}, RawMesh::Vertex::kV |
                        RawMesh::Vertex::kUV);

    mProgram = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_NOISE_PERLIN);
    mSolidFill = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_SOLID_FILL);
}

void CompNoisePerlin::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mSize.x != 0.0f && mSize.y != 0.0f &&
        mSpan.x != 0.0f && mSpan.y != 0.0f)
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

        //std::vector<RawMesh::Vertex> points;
        //auto num = mSize / mSpan;
        //num.x = std::floor(num.x);
        //num.y = std::floor(num.y);

        //points.clear();
        //for (auto x = 0.0f; x != mSize.x; ++x)
        //{
        //    for (auto y = 0.0f; y != mSize.y; ++y)
        //    {
        //        auto coord = glm::vec2(x, y);
        //        auto v = NoisePerlin(coord / mSpan);
        //        points.emplace_back(coord, glm::vec4(v, v, v, 1.0f));
        //    }
        //}
        //command.mMesh = std::create_ptr<RawMesh>();
        //command.mMesh->Init(points, {}, RawMesh::Vertex::kV
        //                              | RawMesh::Vertex::kC);
        //command.mTransform = canvas->GetMatrixStack().GetM();
        //command.mProgram = mSolidFill;
        //command.mDrawMode = GL_POINTS;
        //command.mCallback = nullptr;
        //canvas->Post(command);


        //points.clear();
        //for (auto x = 0; x != num.x; ++x)
        //{
        //    points.emplace_back(glm::vec2(mTrackPoints.at(0).x + x * mSpan.x, mTrackPoints.at(1).y), glm::vec4(1, 0, 0, 1));
        //    points.emplace_back(glm::vec2(mTrackPoints.at(0).x + x * mSpan.x, mTrackPoints.at(2).y), glm::vec4(1, 0, 0, 1));
        //}
        //for (auto y = 0; y != num.y; ++y)
        //{
        //    points.emplace_back(glm::vec2(mTrackPoints.at(0).x, mTrackPoints.at(0).y + y * mSpan.y), glm::vec4(1, 0, 0, 1));
        //    points.emplace_back(glm::vec2(mTrackPoints.at(1).x, mTrackPoints.at(0).y + y * mSpan.y), glm::vec4(1, 0, 0, 1));
        //}
        //command.mMesh = std::create_ptr<RawMesh>();
        //command.mMesh->Init(points, {}, RawMesh::Vertex::kV 
        //                              | RawMesh::Vertex::kC);
        //command.mTransform = canvas->GetMatrixStack().GetM();
        //command.mProgram = mSolidFill;
        //command.mDrawMode = GL_LINES;
        //command.mCallback = nullptr;
        //canvas->Post(command);

        //points.clear();
        //for (auto x = 0; x != num.x + 1; ++x)
        //{
        //    for (auto y = 0; y != num.y + 1; ++y)
        //    {
        //        auto coord = glm::vec2(mTrackPoints.at(0).x + x * mSpan.x,
        //                               mTrackPoints.at(0).y + y * mSpan.y);
        //        auto normal = coord + Random(glm::floor(coord / mSpan)) * 10.0f;
        //        points.emplace_back(coord,  glm::vec4(0, 1, 0, 1));
        //        points.emplace_back(normal, glm::vec4(0, 1, 0, 1));
        //    }
        //}
        //command.mMesh = std::create_ptr<RawMesh>();
        //command.mMesh->Init(points, {}, RawMesh::Vertex::kV
        //                              | RawMesh::Vertex::kC);
        //command.mTransform = canvas->GetMatrixStack().GetM();
        //command.mProgram = mSolidFill;
        //command.mDrawMode = GL_LINES;
        //command.mCallback = nullptr;
        //canvas->Post(command);
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
    tools::Serialize(os, mSpan);
    tools::Serialize(os, mAnchor);
    tools::Serialize(os, mFrequency);
    tools::Serialize(os, mAmplitude);
}

void CompNoisePerlin::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mSpan);
    tools::Deserialize(is, mAnchor);
    tools::Deserialize(is, mFrequency);
    tools::Deserialize(is, mAmplitude);
}

bool CompNoisePerlin::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

std::vector<Component::Property> CompNoisePerlin::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",     &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Span",     &mSpan);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Frequency",  &mFrequency);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Amplitude",  &mAmplitude);
    return std::move(props);
}

void CompNoisePerlin::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

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
    auto & foward = (const RenderPipline::FowardCommand &)command;
    foward.mProgram->BindUniformVector("size_", mSize);
    foward.mProgram->BindUniformVector("span_", mSpan);
    foward.mProgram->BindUniformNumber("frequency_", mFrequency);
    foward.mProgram->BindUniformNumber("amplitude_", mAmplitude);
}
