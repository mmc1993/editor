#include "comp_polygon.h"
#include "comp_transform.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompPolygon::CompPolygon()
{
    _segments.emplace_back(-50, -50);
    _segments.emplace_back( 50, -50);
    _segments.emplace_back( 50,  50);
    _segments.emplace_back(-50,  50);

    _trackPoints.emplace_back(-50, -50);
    _trackPoints.emplace_back( 50, -50);
    _trackPoints.emplace_back( 50,  50);
    _trackPoints.emplace_back(-50,  50);

    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
}

void CompPolygon::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
}

const std::string & CompPolygon::GetName()
{
    static const std::string name = "Polygon";
    return name;
}

void CompPolygon::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _segments);
    tools::Serialize(os, _trackPoints);
}

void CompPolygon::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _segments);
    tools::Deserialize(is, _trackPoints);
}

bool CompPolygon::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    return true;
}

std::vector<Component::Property> CompPolygon::CollectProperty()
{
    return Component::CollectProperty();
}

void CompPolygon::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _trackPoints.at(index) = point;
    _segments.at(index) = point;
}

void CompPolygon::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _segments.insert(std::next(_segments.begin(), index), point);
    _trackPoints.insert(std::next(_trackPoints.begin(), index), point);
}

void CompPolygon::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _segments.erase(std::next(_segments.begin(), index));
    _trackPoints.erase(std::next(_trackPoints.begin(), index));
}

