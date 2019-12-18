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
    return _textureBuffer;
}

void CompRenderTarget::OnAdd()
{ 
    _textureBuffer = std::create_ptr<RawImage>();
    _textureBuffer->InitNull(GL_RGBA);
}

void CompRenderTarget::OnDel()
{
    _textureBuffer.reset();
}

void CompRenderTarget::OnStart(UIObjectGLCanvas * canvas)
{ 
    interface::TargetCommand command;
    command.mType       = interface::TargetCommand::kPush;
    command.mTexture    = _textureBuffer;
    canvas->Post(command);
}

void CompRenderTarget::OnLeave(UIObjectGLCanvas * canvas)
{
    interface::TargetCommand command;
    command.mType       = interface::TargetCommand::kPop;
    canvas->Post(command);
}

