#pragma once

#include "component.h"
#include "../../interface/render.h"

class CompSprite : public Component {
public:
    CompSprite();
    virtual void OnAdd() override;
    virtual void OnDel() override;
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
    void OnRenderCallback(const interface::RenderCommand & command);
    virtual void OnModifyTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnInsertTrackPoint(const size_t index, const glm::vec2 & point) override;
    virtual void OnDeleteTrackPoint(const size_t index, const glm::vec2 & point) override;

private:
    bool            _update;
    std::string     _url;
    glm::vec2       _size;
    glm::vec2       _anchor;
    SharePtr<GLMesh>  _mesh;
    SharePtr<GLTexture> _texture;
    SharePtr<GLProgram> _program;
};