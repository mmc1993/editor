#include "comp_polygon.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

#include "comp_collapse_terrain.h"

CompPolygon::CompPolygon()
{
    mSegments.emplace_back(-50, -50);
    mSegments.emplace_back( 50, -50);

    mTrackPoints.emplace_back(-50, -50);
    mTrackPoints.emplace_back( 50, -50);

    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
}

void CompPolygon::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (mLastCoord != GetOwner()->GetTransform()->GetPosition() && mTrackPoints.size() > 2)
    {
        mLastCoord = GetOwner()->GetTransform()->GetPosition();
        auto owner = GetOwner();
        auto parent = GetOwner()->GetParent();
        auto terrain = parent->GetComponent<CompCollapseTerrain>();
        ASSERT_LOG(terrain != nullptr, "");

        std::vector<glm::vec2> points;
        std::transform(mTrackPoints.begin(), mTrackPoints.end(), std::back_inserter(points), [&owner] (const auto & point)
            {
                return owner->LocalToWorld(point);
            });
        terrain->Erase(points);
    }
}

const std::string & CompPolygon::GetName()
{
    static const std::string name = "Polygon";
    return name;
}

void CompPolygon::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mSegments);
    tools::Serialize(os, mTrackPoints);
}

void CompPolygon::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mSegments);
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
    mSegments.at(index) = point;
}

void CompPolygon::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    mSegments.insert(std::next(mSegments.begin(), index), point);
    mTrackPoints.insert(std::next(mTrackPoints.begin(), index), point);
}

void CompPolygon::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    mSegments.erase(std::next(mSegments.begin(), index));
    mTrackPoints.erase(std::next(mTrackPoints.begin(), index));
}

