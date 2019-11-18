#include "comp_layer_render.h"
#include "../editor_sys.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../ui_sys/ui_object/ui_object.h"

CompLayerRender::CompLayerRender()
{
}

const std::string & CompLayerRender::GetName()
{
    static const std::string name = "LayerRender";
    return name;
}

void CompLayerRender::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
}

void CompLayerRender::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
}

bool CompLayerRender::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    return true;
}

SharePtr<GLImage> CompLayerRender::RefTextureBuffer()
{
    return _textureBuffer;
}

void CompLayerRender::OnAdd()
{ 
    _textureBuffer = std::create_ptr<GLImage>();
    _textureBuffer->InitNull(GL_RGBA);
}

void CompLayerRender::OnDel()
{
    _textureBuffer.reset();
}

void CompLayerRender::OnStart(UIObjectGLCanvas * canvas)
{ 
    interface::LayerCommand command;
    command.mType       = interface::LayerCommand::kPush;
    command.mTexture    = _textureBuffer;
    canvas->Post(command);
}

void CompLayerRender::OnLeave(UIObjectGLCanvas * canvas)
{
    interface::LayerCommand command;
    command.mType       = interface::LayerCommand::kPop;
    canvas->Post(command);
}

