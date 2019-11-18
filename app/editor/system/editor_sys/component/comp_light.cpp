#include "comp_light.h"
#include "comp_transform.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompLight::CompLight()
    : _border(20)
    , _update(kTrackPoint | kBorder)
    , _color(1.0f, 1.0f, 1.0f, 0.5f)
{
    _trackPoints.emplace_back(0,  0);
    _trackPoints.emplace_back(0, 30);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({},{}, GLMesh::Vertex::kV | 
                       GLMesh::Vertex::kC);

    _program = std::create_ptr<GLProgram>();
    _program->Init(tools::GL_PROGRAM_LIGHT);

    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
    AddState(StateEnum::kModifyTrackPoint, true);
}

void CompLight::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    Update();

    interface::PostCommand command;
    command.mTransform  = canvas->GetMatrixStack().GetM();
    command.mType       = interface::PostCommand::kSample;
    command.mProgram    = _program;
    command.mMesh       = _mesh;
    canvas->Post(command);
}

const std::string & CompLight::GetName()
{
    static const std::string name = "Light";
    return name;
}

void CompLight::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _color);
    tools::Serialize(os, _border);
    tools::Serialize(os, _trackPoints);
}

void CompLight::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _color);
    tools::Deserialize(is, _border);
    tools::Deserialize(is, _trackPoints);
}

bool CompLight::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Border")
    {
        _border = std::max(0.0f, std::any_cast<float>(newValue));
        return false;
    }
    return true;
}

std::vector<Component::Property> CompLight::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kColor4, "Color", &_color);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat, "Border", &_border);
    return std::move(props);
}

void CompLight::Update()
{
    ASSERT_LOG(_trackPoints.size() >= 2, "");

    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (_trackPoints.size() == 2)
        {
            std::vector<uint>           indexs;
            std::vector<GLMesh::Vertex> points;

            auto radius = glm::length(_trackPoints.at(1) - _trackPoints.at(0));
            for (auto i = 0; i != 360; i += 10)
            {
                auto x = std::cos(glm::radians((float)i));
                auto y = std::sin(glm::radians((float)i));
                //  内环
                points.emplace_back(glm::vec2(
                    x * radius + _trackPoints.at(0).x, 
                    y * radius + _trackPoints.at(0).y), _color);
                //  外环
                points.emplace_back(glm::vec2(
                    x * (radius + _border) + _trackPoints.at(0).x, 
                    y * (radius + _border) + _trackPoints.at(0).y), 
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

            _mesh->Update(points, indexs);
        }
        else
        {
            std::vector<GLMesh::Vertex> points;
            auto convex = tools::GenConvexPoints(_trackPoints);
            for (auto i = 1; i != convex.size() - 1; ++i)
            {
                points.emplace_back(convex.at(0), _color);
                points.emplace_back(convex.at(i), _color);
                points.emplace_back(convex.at(i + 1), _color);
            }

            auto color = glm::vec4(_color.x, _color.y, _color.z, 0);
            auto outer = tools::GenOuterRing(convex, _border, 10.0);
            for (auto i = 0; i != outer.size(); ++i)
            {
                auto & a0 = outer.at(i++);
                auto & a1 = outer.at(i++);
                auto & b0 = outer.at(i++);
                auto & b1 = outer.at(i++);
                points.emplace_back(a0, _color);
                points.emplace_back(a1,  color);
                points.emplace_back(b1,  color);

                points.emplace_back(a0, _color);
                points.emplace_back(b1,  color);
                points.emplace_back(b0, _color);

                if (!tools::Equal(outer.at(i), a0))
                {
                    points.emplace_back(b0, _color);
                    points.emplace_back(b1,  color);
                    for (; !tools::Equal(outer.at(i), a0); ++i)
                    {
                        points.emplace_back(outer.at(i), color);
                        points.emplace_back(b0,         _color);
                        points.emplace_back(outer.at(i), color);
                    }
                    points.emplace_back(outer.at((i + 2) % outer.size()), color);
                }
                else
                {
                    points.emplace_back(b0, _color);
                    points.emplace_back(b1,  color);
                    points.emplace_back(outer.at((i + 2) % outer.size()), color);
                }
            }
            _mesh->Update(points, {});
        }
    }
}

void CompLight::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    _trackPoints.at(index) = point;
    AddState(StateEnum::kUpdate, true);
}

void CompLight::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    _trackPoints.insert(std::next(_trackPoints.begin(), index), point);
    AddState(StateEnum::kUpdate, true);
}

void CompLight::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
}

