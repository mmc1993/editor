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

SharePtr<RawImage> CompRenderTarget::RefTextureBuffer()
{
    return mTextureBuffer;
}

void CompRenderTarget::OnAdd()
{ 
    mTextureBuffer = std::create_ptr<RawImage>();
    mTextureBuffer->InitNull(GL_RGBA);
}

void CompRenderTarget::OnDel()
{
    mTextureBuffer.reset();
}

void CompRenderTarget::OnStart(UIObjectGLCanvas * canvas)
{
    RenderPipline::TargetCommand command;
    command.mType = RenderPipline::TargetCommand::TypeEnum::kPush;
    command.mRenderTextures[0] = mTextureBuffer;
    canvas->Post(command);
}

void CompRenderTarget::OnLeave(UIObjectGLCanvas * canvas)
{
    RenderPipline::TargetCommand command;
    command.mType = RenderPipline::TargetCommand::TypeEnum::kPop;
    canvas->Post(command);
}

