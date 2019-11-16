#include "comp_field_of_view.h"
#include "../editor_sys.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompFieldOfView::CompFieldOfView()
    : _color(1.0f)
    , _distance(1.0f)
{
    _trackPoints.emplace_back(0,   -10);
    _trackPoints.emplace_back(-50, 100);
    _trackPoints.emplace_back( 50, 100);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({}, {}, GLMesh::Vertex::kV | 
                        GLMesh::Vertex::kC);

    _program = Global::Ref().mRawSys->Get<GLProgram>(tools::GL_PROGRAM_LIGHT);

    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
}

const std::string & CompFieldOfView::GetName()
{
    static const std::string name = "FieldOfView";
    return name;
}

void CompFieldOfView::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _url);
    tools::Serialize(os, _color);
    tools::Serialize(os, _distance);
    tools::Serialize(os, _trackPoints);
}

void CompFieldOfView::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _color);
    tools::Deserialize(is, _distance);
    tools::Deserialize(is, _trackPoints);
}

bool CompFieldOfView::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    if (title == "Debug")
    {
        if (std::any_cast<bool>(newValue))
        {
            GenView();
            GenMesh();
            _trackPoints = _segments;
        }
    }
    return true;
}

void CompFieldOfView::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    Update();

    interface::PostCommand command;
    command.mMesh           = _mesh;
    command.mProgram        = _program;
    command.mTransform      = canvas->GetMatrixStack().GetM();
    command.mType           = interface::PostCommand::kOverlay;
    canvas->Post(command);
}

std::vector<Component::Property> CompFieldOfView::CollectProperty()
{
    static bool debug;

    auto props = Component::CollectProperty();
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kString, "Url",      &_url);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kColor4, "Color",    &_color);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kFloat,  "Distance", &_distance);
    props.emplace_back(interface::Serializer::StringValueTypeEnum::kBool,   "Debug",    &debug);
    return std::move(props);
}

void CompFieldOfView::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        auto track = Global::Ref().mEditorSys->GetProject()->GetRoot();
        for (auto & name : tools::Split(_url, "/"))
        {
            track = track->GetObject(name);
            ASSERT_LOG(track != nullptr, name.c_str());
        }
        _polygons = track->GetComponentsInChildren<CompPolygon>();
    }
    _segments.clear();
    GenView();
    GenMesh();
}

void CompFieldOfView::GenView()
{
    std::vector<glm::vec2> segments;
    for (auto & polygon : _polygons)
    {
        for (auto i = 0u, n = polygon->GetSegments().size(); i != n; ++i)
        {
            const auto & a = polygon->GetSegments().at(i          );
            const auto & b = polygon->GetSegments().at((i + 1) % n);
            auto worldA = polygon->GetOwner()->LocalToWorld(a);
            auto worldB = polygon->GetOwner()->LocalToWorld(b);
            segments.emplace_back(GetOwner()->WorldToLocal(worldA));
            segments.emplace_back(GetOwner()->WorldToLocal(worldB));
        }
    }

    for (auto i = 0u, n = _trackPoints.size(); i != _trackPoints.size(); ++i)
    {
        segments.push_back(_trackPoints.at(i          ));
        segments.push_back(_trackPoints.at((i + 1) % n));
    }
    
    ASSERT_LOG(0 == std::count(segments.begin(), segments.end(), glm::vec2(0)), "");

    _segments.clear();
    glm::vec2 next = _segments.emplace_back(0.0f);
    for (auto i = 0; i != segments.size(); i += 2)
    {
        auto point = RayPoint(segments, segments.at(i), &next);
        if (tools::Equal(point, segments.at(i)))
        {
            if (glm::cross(point, next - point) < 0)
            {
                //  向左延长
                auto offset = glm::normalize(glm::vec2(-point.y, point.x)) * 10.0f;
                _segments.emplace_back(RayPoint(segments, point + offset));
            }
            else
            {
                //  向右延长
                auto offset = glm::normalize(glm::vec2(point.y, -point.x)) * 10.0f;
                _segments.emplace_back(RayPoint(segments, point + offset));
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

            return q0 != q1 ? q0 < q1
                : glm::cross(a, b) >= 0;
        });
}

void CompFieldOfView::GenMesh()
{
    std::vector<GLMesh::Vertex> points;
    std::vector<uint>           indexs;
    for (auto i = 0; i != _segments.size(); ++i)
    {
        auto & p = _segments.at(i);
        auto   s = 1.0f - std::max(0.0f,glm::length(p) / _distance);
        glm::vec4 color{_color.r, _color.g, _color.b, _color.a * s};
        points.emplace_back(p, color);

        if (i > 1)
        {
            indexs.emplace_back(0    );
            indexs.emplace_back(i - 1);
            indexs.emplace_back(i    );
        }
    }

    indexs.emplace_back(0                );
    indexs.emplace_back(points.size() - 1);
    indexs.emplace_back(1                );

    _mesh->Update(points, indexs, GL_DYNAMIC_DRAW, GL_DYNAMIC_DRAW);
}

glm::vec2 CompFieldOfView::RayPoint(const std::vector<glm::vec2> & segments, const glm::vec2 & point, glm::vec2 * next)
{
    const glm::vec2 zero(0, 0);

    glm::vec2 result;
    auto crossA = 0.0f;
    auto crossB = 0.0f;
    auto crossC = std::numeric_limits<float>::max();
    for (auto i = 0u, count = segments.size(); i != count; i += 2)
    {
        auto & a = segments.at(i    );
        auto & b = segments.at(i + 1);
        if (tools::IsCrossSegment(zero, point, a, b, &crossA, &crossB))
        {
            if (crossA < crossC)
            {
                result = glm::lerp(zero, point, crossC = crossA);
                *next = b;
            }
        }
    }
    ASSERT_LOG(std::numeric_limits<float>::max() != crossC, "");
    return result;
}

glm::vec2 CompFieldOfView::RayPoint(const std::vector<glm::vec2>& segments, const glm::vec2 & point)
{
    const glm::vec2 zero(0, 0);

    glm::vec2 result;
    auto crossA = 0.0f;
    auto crossB = 0.0f;
    auto crossC = std::numeric_limits<float>::max();
    for (auto i = 0u, count = segments.size(); i != count; i += 2)
    {
        auto & a = segments.at(i    );
        auto & b = segments.at(i + 1);
        if (tools::IsCrossLine(zero, point, a, b, &crossA, &crossB))
        {
            if (crossA > 0 && crossA < crossC &&
                crossB >= 0.0f && crossB <= 1.0f)
            {
                result = glm::lerp(zero, point, crossC = crossA);
            }
        }
    }
    ASSERT_LOG(crossC != std::numeric_limits<float>::max(), "");
    return result;
}

void CompFieldOfView::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    _trackPoints.at(index) = point;
}

void CompFieldOfView::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    _trackPoints.insert(std::next(_trackPoints.begin(), index), point);
}

void CompFieldOfView::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    _trackPoints.erase(std::next(_trackPoints.begin(), index));
}
