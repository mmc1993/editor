#include "comp_render_target.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../editor_sys.h"

CompRenderTarget::CompRenderTarget()
{
}

const std::string & CompRenderTarget::GetName()
{
    static const std::string name = "RenderTarget";
    return name;
}

void CompRenderTarget::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
}

void CompRenderTarget::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
}

bool CompRenderTarget::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    return true;
}

SharePtr<RawImage> CompRenderTarget::GetImage()
{
    return mImage;
}

void CompRenderTarget::OnAdd()
{ 
    mImage = std::create_ptr<RawImage>();
    mImage->InitNull(GL_RGBA);
}

void CompRenderTarget::OnDel()
{
    mImage.reset();
}

void CompRenderTarget::OnStart(UIObjectGLCanvas * canvas)
{
    RenderPipline::TargetCommand command;
    command.mType = RenderPipline::TargetCommand::TypeEnum::kPush;
    command.mRenderTextures[0] = mImage;
    canvas->Post(command);
}

void CompRenderTarget::OnLeave(UIObjectGLCanvas * canvas)
{
    RenderPipline::TargetCommand command;
    command.mType = RenderPipline::TargetCommand::TypeEnum::kPop;
    command.mRenderTextures[0] = mImage;
    canvas->Post(command);
}

