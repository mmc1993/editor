#include "comp_light.h"
#include "../../raw_sys/comp_transform.h"

CompLight::CompLight()
    : mBorder(20)
    , mUpdate(kTrackPoint | kBorder)
    , mColor(1.0f, 1.0f, 1.0f, 0.5f)
{
    mTrackPoints.emplace_back(0,  0);
    mTrackPoints.emplace_back(0, 30);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kC);

    mProgram = std::create_ptr<RawProgram>();
    mProgram->Init(tools::GL_PROGRAM_LIGHT);

    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompLight::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    Update();

    RenderPipline::PostCommand command;
    command.mTransform  = canvas->GetMatrixStack().GetM();
    command.mType       = RenderPipline::PostCommand::kSample;
    command.mProgram    = mProgram;
    command.mMesh       = mMesh;

    command.mBlendSrc = GL_SRC_ALPHA;
    command.mBlendDst = GL_ONE;

    command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;

    canvas->Post(command);
}

const std::string & CompLight::GetName()
{
    static const std::string name = "Light";
    return name;
}

void CompLight::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mColor);
    tools::Serialize(os, mBorder);
    tools::Serialize(os, mTrackPoints);
}

void CompLight::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mColor);
    tools::Deserialize(is, mBorder);
    tools::Deserialize(is, mTrackPoints);
}

bool CompLight::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Border")
    {
        mBorder = std::max(0.0f, std::any_cast<float>(newValue));
        return false;
    }
    return true;
}

std::vector<Component::Property> CompLight::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4, "Color", &mColor);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Border", &mBorder);
    return std::move(props);
}

void CompLight::Update()
{
    ASSERT_LOG(mTrackPoints.size() >= 2, "");

    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (mTrackPoints.size() == 2)
        {
            std::vector<uint>           indexs;
            std::vector<RawMesh::Vertex> points;

            auto radius = glm::length(mTrackPoints.at(1) - mTrackPoints.at(0));
            for (auto i = 0; i != 360; i += 10)
            {
                auto x = std::cos(glm::radians((float)i));
                auto y = std::sin(glm::radians((float)i));
                //  内环
                points.emplace_back(glm::vec2(
                    x * radius + mTrackPoints.at(0).x, 
                    y * radius + mTrackPoints.at(0).y), mColor);
                //  外环
                points.emplace_back(glm::vec2(
                    x * (radius + mBorder) + mTrackPoints.at(0).x, 
                    y * (radius + mBorder) + mTrackPoints.at(0).y), 
                    glm::vec4(0));
            }
            ASSERT_LOG(points.size() % 2 == 0, "");

            auto middle = points.size() / 2;
            //  内环
            for (auto i = 1; i != middle - 1; ++i)
            {
                indexs.emplace_back(0);
                indexs.emplace_back(2 * i);
                indexs.emplace_back(2 * (i + 1));
            }
            //  外环
            for (auto i = 0; i != middle; ++i)
            {
                indexs.emplace_back(2 * i);
                indexs.emplace_back(2 * i + 1);
                indexs.emplace_back(2 * ((i + 1) % middle) + 1);
                
                indexs.emplace_back(2 * i);
                indexs.emplace_back(2 * ((i + 1) % middle) + 1);
                indexs.emplace_back(2 * ((i + 1) % middle));
            }

            mMesh->Update(points, indexs);
        }
        else
        {
            std::vector<RawMesh::Vertex> points;
            auto convex = tools::GenConvexPoints(mTrackPoints);
            for (auto i = 1; i != convex.size() - 1; ++i)
            {
                points.emplace_back(convex.at(0), mColor);
                points.emplace_back(convex.at(i), mColor);
                points.emplace_back(convex.at(i + 1), mColor);
            }

            auto color = glm::vec4(mColor.x, mColor.y, mColor.z, 0);
            auto outer = tools::GenOuterRing(convex, mBorder, 10.0);
            for (auto i = 0; i != outer.size(); ++i)
            {
                auto & a0 = outer.at(i++);
                auto & a1 = outer.at(i++);
                auto & b0 = outer.at(i++);
                auto & b1 = outer.at(i++);
                points.emplace_back(a0, mColor);
                points.emplace_back(a1,  color);
                points.emplace_back(b1,  color);

                points.emplace_back(a0, mColor);
                points.emplace_back(b1,  color);
                points.emplace_back(b0, mColor);

                if (!tools::Equal(outer.at(i), a0))
                {
                    points.emplace_back(b0, mColor);
                    points.emplace_back(b1,  color);
                    for (; !tools::Equal(outer.at(i), a0); ++i)
                    {
                        points.emplace_back(outer.at(i), color);
                        points.emplace_back(b0,         mColor);
                        points.emplace_back(outer.at(i), color);
                    }
                    points.emplace_back(outer.at((i + 2) % outer.size()), color);
                }
                else
                {
                    points.emplace_back(b0, mColor);
                    points.emplace_back(b1,  color);
                    points.emplace_back(outer.at((i + 2) % outer.size()), color);
                }
            }
            mMesh->Update(points, {});
        }
    }
}

void CompLight::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    mTrackPoints.at(index) = point;
    AddState(StateEnum::kUpdate, true);
}

void CompLight::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    mTrackPoints.insert(std::next(mTrackPoints.begin(), index), point);
    AddState(StateEnum::kUpdate, true);
}

void CompLight::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
}

