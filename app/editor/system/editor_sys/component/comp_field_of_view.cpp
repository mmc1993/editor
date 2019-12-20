#include "comp_field_of_view.h"
#include "../editor_sys.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"

CompFieldOfView::CompFieldOfView()
    : _color(1.0f)
{
    _mesh = std::create_ptr<RawMesh>();
    _mesh->Init({}, {}, RawMesh::Vertex::kV | 
                        RawMesh::Vertex::kC);

    _program = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_FIELD_OF_VIEW);
}

const std::string & CompFieldOfView::GetName()
{
    static const std::string name = "FieldOfView";
    return name;
}

void CompFieldOfView::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    _clipObject.EncodeBinary(os, project);
    _polyObject.EncodeBinary(os, project);
    tools::Serialize(os, _color);
}

void CompFieldOfView::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    _clipObject.DecodeBinary(is, project);
    _polyObject.DecodeBinary(is, project);
    tools::Deserialize(is, _color);
}

bool CompFieldOfView::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompFieldOfView::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (_clipObject.Check() && _polyObject.Check())
    {
        Update();

        interface::PostCommand command;
        command.mMesh           = _mesh;
        command.mProgram        = _program;
        command.mTransform      = canvas->GetMatrixStack().GetM();
        command.mType           = interface::PostCommand::kSample;
        command.mCallback       = std::bind(&CompFieldOfView::OnDrawCallback,
                                CastPtr<CompFieldOfView>(shared_from_this()), 
                                std::placeholders::_1, std::placeholders::_2);
        canvas->Post(command);
    }
}

std::vector<Component::Property> CompFieldOfView::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Clip Obj", &_clipObject, std::vector<uint>{ Res::TypeEnum::kObj });
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Poly Obj", &_polyObject, std::vector<uint>{ Res::TypeEnum::kObj });
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4, "Color",   &_color);
    return std::move(props);
}

void CompFieldOfView::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (_clipObject.Modify())
        {
            ASSERT_LOG(_clipObject.Instance<GLObject>()->Relation(GetOwner()) == 0, "");
        }
        if (_polyObject.Modify())
        {
            ASSERT_LOG(_polyObject.Instance<GLObject>()->Relation(GetOwner()) == 0, "");
        }
    }
    GenView();
    GenMesh();
}

void CompFieldOfView::GenView()
{
    const auto origin = GetOwner()->LocalToWorld(glm::vec2(0));

    std::vector<glm::vec2> segments;
    for (auto & polygon : _polyObject.Instance<GLObject>()->GetComponentsInChildren<CompPolygon>())
    {
        for (auto i = 0u, n = polygon->GetSegments().size(); i != n; ++i)
        {
            const auto & a = polygon->GetSegments().at(i          );
            const auto & b = polygon->GetSegments().at((i + 1) % n);
            auto worldA = polygon->GetOwner()->LocalToWorld(a) - origin;
            auto worldB = polygon->GetOwner()->LocalToWorld(b) - origin;
            segments.emplace_back(worldA);
            segments.emplace_back(worldB);
        }
    }

    _segments.clear();
    _segments.emplace_back(0.0f);
    for (auto i = 0; i != segments.size(); i += 2)
    {
        auto point = RayTracking(segments, segments.at(i));
        if (tools::Equal(point, segments.at(i)))
        {
            {
                //  向左延长
                auto offset = glm::normalize(glm::vec2(-point.y, point.x)) * 1.0f;
                _segments.emplace_back(RayExtended(segments, point + offset));
            }
            {
                //  向右延长
                auto offset = glm::normalize(glm::vec2(point.y, -point.x)) * 1.0f;
                _segments.emplace_back(RayExtended(segments, point + offset));
            }
        }
        _segments.emplace_back(point);
    }

    std::sort(_segments.begin() + 1, _segments.end(), [] (const glm::vec2 & a, const glm::vec2 & b)
        {
            uint q0 = 0, q1 = 0;
            if      (a.x >= 0 && a.y >= 0) q0 = 0;
            else if (a.x <  0 && a.y >= 0) q0 = 1;
            else if (a.x <  0 && a.y <  0) q0 = 2;
            else if (a.x >= 0 && a.y <  0) q0 = 3;

            if      (b.x >= 0 && b.y >= 0) q1 = 0;
            else if (b.x <  0 && b.y >= 0) q1 = 1;
            else if (b.x <  0 && b.y <  0) q1 = 2;
            else if (b.x >= 0 && b.y <  0) q1 = 3;

            return q0 != q1 ? q0 < q1 : glm::cross(a, b) > 0;
        });
}

void CompFieldOfView::GenMesh()
{
    std::vector<RawMesh::Vertex> points;
    auto count = _segments.size() - 1;
    for (auto i = 0; i != count; ++i)
    {
        auto & a = _segments.at((i + count - 1) % count + 1);
        auto & b = _segments.at( i                      + 1);
        auto & c = _segments.at((i         + 1) % count + 1);

        points.emplace_back(_segments.front(), _color);
        points.emplace_back(b, _color);
        points.emplace_back(c, _color);
    }

    _mesh->Update(points, { }, GL_DYNAMIC_DRAW, GL_DYNAMIC_DRAW);
}

glm::vec2 CompFieldOfView::RayTracking(const std::vector<glm::vec2> & segments, const glm::vec2 & point)
{
    static const glm::vec2 zero(0, 0);

    glm::vec2 result = point;
    auto crossA = 0.0f;
    auto crossB = 0.0f;
    auto crossC = 1.0f;
    for (auto i = 0u, count = segments.size(); i != count; i += 2)
    {
        auto & a = segments.at(i    );
        auto & b = segments.at(i + 1);
        if (tools::IsCrossSegment(zero, point, a, b, &crossA, &crossB)
            && crossA < crossC 
            && point != a 
            && point != b)
        {
            result = glm::lerp(zero, point, crossC = crossA);
        }
    }
    return result;
}

glm::vec2 CompFieldOfView::RayExtended(const std::vector<glm::vec2>& segments, const glm::vec2 & point)
{
    static const glm::vec2 zero(0, 0);

    glm::vec2 result;
    auto crossA = 0.0f;
    auto crossB = 0.0f;
    auto crossC = std::numeric_limits<float>::max();
    for (auto i = 0u, count = segments.size(); i != count; i += 2)
    {
        auto & a = segments.at(i    );
        auto & b = segments.at(i + 1);
        if (tools::IsCrossLine(zero, point, a, b, &crossA, &crossB)
            && crossA >  0.0f && crossA < crossC
            && crossB >= 0.0f && crossB <= 1.0f)
        {
            result = glm::lerp(zero, point, crossC = crossA);
        }
    }
    ASSERT_LOG(crossC != std::numeric_limits<float>::max(), "");
    return result;
}

void CompFieldOfView::OnDrawCallback(const interface::RenderCommand & command, uint texturePos)
{
    auto & cmd = (const interface::PostCommand &)command;
    cmd.mProgram->BindUniformTex2D("uniform_sample", 
        _clipObject.Instance<GLObject>()
            ->GetComponent<CompRenderTarget>()
            ->RefTextureBuffer()->mID, 
        texturePos);
}

