#pragma once

#include "../../raw_sys/component.h"

class CompLightning : public Component {
public:
    enum UpdateEnum {
        kTexture = 1,
        kSegment = 2,
        kMesh    = 4,
    };

    struct Segment {
        glm::vec2 mStart;
        glm::vec2 mEnded;
        uint mBranch;

        Segment(const glm::vec2 & start, const glm::vec2 & ended, uint branch)
            : mStart(start)
            , mEnded(ended)
            , mBranch(branch)
        { }
    };

public:
    CompLightning();
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
    virtual bool OnModifyProperty(const std::any & oldValue, 
                                  const std::any & newValue, 
                                  const std::string & title) override;

protected:
    virtual std::vector<Property> CollectProperty() override;

private:
    void Update();
    void GenMesh(float scale, const std::vector<Segment> & segments);
    void GenSegm(float width,
        std::queue<Segment> & input, 
        std::vector<Segment> & output);
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnInsertTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnDeleteTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    uint                _update;
    float               _scale;
    float               _width;
    glm::vec4           _color;
    std::string         _url;

    SharePtr<GLMesh>    _mesh;
    SharePtr<GLTexture> _texture;
    SharePtr<GLProgram> _program;
    std::vector<Segment> _segments;
};