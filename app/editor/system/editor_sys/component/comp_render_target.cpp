#include "comp_render_target.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../editor_sys.h"

CompRenderTarget::CompRenderTarget(): mDraw(false)
{
    mProgram = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_SCREEN);
    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({ RawMesh::Vertex({ -1.0f, -1.0f }, { 0.0f, 0.0f }),
                  RawMesh::Vertex({  1.0f, -1.0f }, { 1.0f, 0.0f }),
                  RawMesh::Vertex({  1.0f,  1.0f }, { 1.0f, 1.0f }),
                  RawMesh::Vertex({ -1.0f,  1.0f }, { 0.0f, 1.0f }) }, { 0, 1, 2, 0, 2, 3 }, 
                  RawMesh::Vertex::kV | RawMesh::Vertex::kUV);
}

const std::string & CompRenderTarget::GetName()
{
    static const std::string name = "RenderTarget";
    return name;
}

void CompRenderTarget::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(os, mDraw);
}

void CompRenderTarget::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mDraw);
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
    command.mClearColor.r       = 0;
    command.mClearColor.g       = 0;
    command.mClearColor.b       = 0;
    command.mClearColor.a       = 0;
    command.mRenderTextures[0]  = mImage;
    command.mEnabledFlag = RenderPipline::RenderCommand::kTargetColor0 |
                           RenderPipline::RenderCommand::kTargetColor1;
    canvas->Post(command);
}

void CompRenderTarget::OnLeave(UIObjectGLCanvas * canvas)
{
    RenderPipline::TargetCommand command;
    command.mType = RenderPipline::TargetCommand::TypeEnum::kPop;
    canvas->Post(command);

    if (mDraw)
    {
        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mBlendSrc   = GL_SRC_ALPHA;
        command.mBlendDst   = GL_ONE_MINUS_SRC_ALPHA;
        command.mPairImages.emplace_back("texture0", mImage);
        command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;
        canvas->Post(command);
    }
}

std::vector<CompRenderTarget::Property> CompRenderTarget::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kBool, "Draw", &mDraw);
    return std::move(props);
}

