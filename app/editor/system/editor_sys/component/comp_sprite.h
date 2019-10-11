#pragma once

#include "component.h"
#include "../../interface/render.h"

class CompSprite : public Component {
public:
    virtual void OnAdd() override;
    virtual void OnDel() override;
    virtual void OnUpdate(UIObjectGLCanvas * canvas, float dt) override;

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;

    virtual bool OnModifyProperty(
        const std::any & value,
        const std::any & backup,
        const std::string & title) override;

    virtual std::vector<Property> CollectProperty() override;

private:
    std::string _url;
    glm::vec2 _size;
    glm::vec2 _anchor;
    SharePtr<Interface::FowardCommand> _command;
};