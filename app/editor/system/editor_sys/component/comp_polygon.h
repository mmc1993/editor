#pragma once

#include "../../raw_sys/component.h"

class CompPolygon : public Component {
public:
    CompPolygon();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
    virtual bool OnModifyProperty(const std::any & oldValue, 
                                  const std::any & newValue, 
                                  const std::string & title) override;
    std::vector<glm::vec2> & GetSegments() { return _segments; }

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnInsertTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnDeleteTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    std::vector<glm::vec2> _segments;
};