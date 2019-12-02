#pragma once

#include "../../raw_sys/component.h"

class CompRenderTarget : public Component {
public:
    CompRenderTarget();

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ofstream & os) override;
    virtual void DecodeBinary(std::ifstream & is) override;
    virtual bool OnModifyProperty(const std::any & oldValue,
                                  const std::any & newValue,
                                  const std::string & title) override;
    SharePtr<GLImage> RefTextureBuffer();

protected:
    virtual void OnAdd();
    virtual void OnDel();
    virtual void OnStart(UIObjectGLCanvas * canvas) override;
    virtual void OnLeave(UIObjectGLCanvas * canvas) override;

private:
    SharePtr<GLImage> _textureBuffer;
};