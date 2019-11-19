#include "comp_render_target.h"
#include "../editor_sys.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompRenderTarget::CompRenderTarget()
{
}

const std::string & CompRenderTarget::GetName()
{
    static const std::string name = "RenderTarget";
    return name;
}

void CompRenderTarget::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
}

void CompRenderTarget::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
}

bool CompRenderTarget::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    return true;
}

SharePtr<GLImage> CompRenderTarget::RefTextureBuffer()
{
    return _textureBuffer;
}

void CompRenderTarget::OnAdd()
{ 
    _textureBuffer = std::create_ptr<GLImage>();
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

