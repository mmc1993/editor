#pragma once

#include "../../raw_sys/component.h"

class CompRenderTarget : public Component {
public:
    CompRenderTarget();

    virtual const std::string & GetName() override;
    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;
    virtual bool OnModifyProperty(const std::any & oldValue,
                                  const std::any & newValue,
                                  const std::string & title) override;
    SharePtr<RawImage> GetImage();

protected:
    virtual void OnStart(UIObjectGLCanvas * canvas) override;
    virtual void OnLeave(UIObjectGLCanvas * canvas) override;
    virtual std::vector<Property> CollectProperty() override;

private:
    bool mDraw;

    SharePtr<RawMesh>       mMesh;
    SharePtr<RawProgram>    mProgram;

    SharePtr<RawImage> mImage;
};