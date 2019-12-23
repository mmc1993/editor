#include "comp_text.h"
#include "../editor_sys.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompText::CompText()
    : mText("M")
    , mColor(1.0f)
    , mAnchor(0.0f)
    , mSize(100.0f)
    , mOutDelta(0)
    , mOutColor(0)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kC | 
                       RawMesh::Vertex::kUV);
    mProgram = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_FONT_SDF);

    AddState(StateEnum::kModifyTrackPoint, true);
}

const std::string & CompText::GetName()
{
    static const std::string name = "Text";
    return name;
}

void CompText::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    mFont.EncodeBinary(os, project);

    tools::Serialize(os, mText);
    tools::Serialize(os, mSize);
    tools::Serialize(os, mAnchor);

    tools::Serialize(os, mColor);
    tools::Serialize(os, mOutColor);
    tools::Serialize(os, mOutDelta);
}

void CompText::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    mFont.DecodeBinary(is, project);
    
    tools::Deserialize(is, mText);
    tools::Deserialize(is, mSize);
    tools::Deserialize(is, mAnchor);

    tools::Deserialize(is, mColor);
    tools::Deserialize(is, mOutColor);
    tools::Deserialize(is, mOutDelta);
}

bool CompText::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompText::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        if (mFont.Check()) { UpdateMesh(); }
    }

    if (mFont.Check())
    {
        RenderPipline::FowardCommand command;
        command.mMesh       = mMesh;
        command.mProgram    = mProgram;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mTextures.emplace_back("uniform_texture",
                mFont.Instance<RawFont>()->RefTexture());
        command.mCallback = std::bind(
            &CompText::OnDrawCallback, this,
            std::placeholders::_1,
            std::placeholders::_2);
        canvas->Post(command);
    }
}
std::vector<Component::Property> CompText::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Font",      &mFont,     (uint)(Res::TypeEnum::kFnt));
    props.emplace_back(UIParser::StringValueTypeEnum::kString,  "Text",      &mText);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Size",      &mSize);
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",    &mAnchor);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4,  "Color",     &mColor);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4,  "OutColor",  &mOutColor);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat,   "OutDelta",  &mOutDelta);

    return std::move(props);
}

void CompText::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{ 
    glm::vec2 min, max;
    switch (index)
    {
    case 0:
        min.x = std::min(point.x, mTrackPoints.at(2).x);
        min.y = std::min(point.y, mTrackPoints.at(2).y);
        max.x = mTrackPoints.at(2).x;
        max.y = mTrackPoints.at(2).y;
        break;
    case 1:
        min.x = mTrackPoints.at(0).x;
        min.y = std::min(point.y, mTrackPoints.at(3).y);
        max.x = std::max(point.x, mTrackPoints.at(3).x);
        max.y = mTrackPoints.at(2).y;
        break;
    case 2:
        min.x = mTrackPoints.at(0).x;
        min.y = mTrackPoints.at(0).y;
        max.x = std::max(point.x, mTrackPoints.at(0).x);
        max.y = std::max(point.y, mTrackPoints.at(0).y);
        break;
    case 3:
        min.x = std::min(point.x, mTrackPoints.at(1).x);
        min.y = mTrackPoints.at(0).y;
        max.x = mTrackPoints.at(2).x;
        max.y = std::max(point.y, mTrackPoints.at(1).y);
        break;
    }

    mSize.x = max.x - min.x;
    mSize.y = max.y - min.y;

    auto coord = GetOwner()->LocalToParent(mSize * mAnchor + min);
    GetOwner()->GetTransform()->Position(coord.x, coord.y);

    AddState(StateEnum::kUpdate, true);
}

void CompText::UpdateMesh()
{
    mTrackPoints.at(0).x = -mSize.x *      mAnchor.x;
    mTrackPoints.at(0).y = -mSize.y *      mAnchor.y;
    mTrackPoints.at(1).x =  mSize.x * (1 - mAnchor.x);
    mTrackPoints.at(1).y = -mSize.y *      mAnchor.y;
    mTrackPoints.at(2).x =  mSize.x * (1 - mAnchor.x);
    mTrackPoints.at(2).y =  mSize.y * (1 - mAnchor.y);
    mTrackPoints.at(3).x = -mSize.x *      mAnchor.x;
    mTrackPoints.at(3).y =  mSize.y * (1 - mAnchor.y);

    std::vector<RawMesh::Vertex> points;

    auto codes = mFont.Instance<RawFont>()->RefWord(mText);
    auto texW  = mFont.Instance<RawFont>()->RefTexture()->GetW();
    auto texH  = mFont.Instance<RawFont>()->RefTexture()->GetH();
    auto posX  = 0.0f;
    auto posY  = 0.0f;
    auto lineH = 0.0f;
    for (auto i = 0; i != codes.size(); ++i)
    {
        const auto & word = mFont.Instance<RawFont>()->RefWord(codes.at(i));
        auto wordW = (word.mUV.z - word.mUV.x) * texW;
        auto wordH = (word.mUV.w - word.mUV.y) * texH;
        //  ½Ø¶Ï¿í¶È
        if (posX + wordW  > mSize.x)
        {
            posX = 0.0f; posY += lineH;
        }
        //  ½Ø¶Ï¸ß¶È
        if (posX + wordW > mSize.x ||
            posY + wordH > mSize.y)
        {
            break;
        }
        lineH = std::max(lineH, wordH);

        auto x = mTrackPoints.at(3).x + posX;
        auto y = mTrackPoints.at(3).y - posY;
        glm::vec2 p0(x,         y);
        glm::vec2 p1(x + wordW, y);
        glm::vec2 p2(x + wordW, y - wordH);
        glm::vec2 p3(x,         y - wordH);

        points.emplace_back(p0, mColor, glm::vec2(word.mUV.x, word.mUV.w));
        points.emplace_back(p1, mColor, glm::vec2(word.mUV.z, word.mUV.w));
        points.emplace_back(p2, mColor, glm::vec2(word.mUV.z, word.mUV.y));

        points.emplace_back(p0, mColor, glm::vec2(word.mUV.x, word.mUV.w));
        points.emplace_back(p2, mColor, glm::vec2(word.mUV.z, word.mUV.y));
        points.emplace_back(p3, mColor, glm::vec2(word.mUV.x, word.mUV.y));

        posX += wordW;
    }

    mMesh->Update(points, { });
}

void CompText::OnDrawCallback(const RenderPipline::RenderCommand & command, uint pos)
{ 
    auto forward = (const RenderPipline::FowardCommand &)(command);
    forward.mProgram->BindUniformNumber("out_delta_", mOutDelta);
    forward.mProgram->BindUniformVector("out_color_", mOutColor);
}
