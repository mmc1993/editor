#include "comp_polygon.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompPolygon::CompPolygon()
{
    _segments.emplace_back(-50, -50);
    _segments.emplace_back( 50, -50);
    //_segments.emplace_back( 50,  50);
    //_segments.emplace_back(-50,  50);

    mTrackPoints.emplace_back(-50, -50);
    mTrackPoints.emplace_back( 50, -50);
    //mTrackPoints.emplace_back( 50,  50);
    //mTrackPoints.emplace_back(-50,  50);

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

void CompPolygon::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, _segments);
    tools::Serialize(os, mTrackPoints);
}

void CompPolygon::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, _segments);
    tools::Deserialize(is, mTrackPoints);
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
    mTrackPoints.at(index) = point;
    _segments.at(index) = point;
}

void CompPolygon::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _segments.insert(std::next(_segments.begin(), index), point);
    mTrackPoints.insert(std::next(mTrackPoints.begin(), index), point);
}

void CompPolygon::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _segments.erase(std::next(_segments.begin(), index));
    mTrackPoints.erase(std::next(mTrackPoints.begin(), index));
}

