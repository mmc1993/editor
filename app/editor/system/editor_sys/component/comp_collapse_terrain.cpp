#include "comp_collapse_terrain.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompCollapseTerrain::CompCollapseTerrain()
    : mAnchor(0.5f, 0.5f)
{
    mTrackPoints.resize(4);

    mMesh = std::create_ptr<RawMesh>();
    mMesh->Init({},{}, RawMesh::Vertex::kV | 
                       RawMesh::Vertex::kUV);

    mTexture = std::create_ptr<RawImage>();
    mTexture->InitNull(GL_RGBA);
    mTexture->SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    mTexture->SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    mProgramInit = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_COLLAPSE_TERRAIN_INIT);
    mProgramDraw = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_COLLAPSE_TERRAIN_DRAW);
    mProgramQuad = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_SPRITE);
}

void CompCollapseTerrain::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    if (Update(canvas))
    {
        if (!mEraseList.empty())
        {
            ClearErase(canvas);
            mEraseList.clear();
        }

        RenderPipline::FowardCommand command;
        command.mMesh = mMesh;command.mProgram = mProgramQuad;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mPairImages.emplace_back("texture0",mTexture);
        command.mBlendSrc = GL_SRC_ALPHA;
        command.mBlendDst = GL_ONE_MINUS_SRC_ALPHA;
        command.mEnabledFlag = RenderPipline::RenderCommand::kBlend;
        canvas->Post(command);

        //DebugPostDrawPolygons(canvas);
    }
}

const std::string & CompCollapseTerrain::GetName()
{
    static const std::string name = "CollapseTerrain";
    return name;
}

void CompCollapseTerrain::EncodeBinary(std::ostream & os, Project * project)
{
    Component::EncodeBinary(os, project);
    tools::Serialize(  os, mAnchor);
    mMap.EncodeBinary( os, project);
    mJson.EncodeBinary(os, project);
}

void CompCollapseTerrain::DecodeBinary(std::istream & is, Project * project)
{
    Component::DecodeBinary(is, project);
    tools::Deserialize(is, mAnchor);
    mMap.DecodeBinary( is, project);
    mJson.DecodeBinary(is, project);
}

bool CompCollapseTerrain::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    return true;
}

void CompCollapseTerrain::Erase(const std::vector<glm::vec2> & points)
{
    ASSERT_LOG(mMap.Check(), "");
    static const glm::vec4 zeroColor(0, 0, 0, 0.0f);
    static const glm::vec4 edgeColor(0, 0, 0, 0.2f);
    static const glm::vec4 normColor(0, 0, 0, 1.0f);

    // points 世界坐标集
    const auto & map = mMap.Instance<RawMap>()->GetMap();
    glm::vec2 offset(
        map.mPixelW * mAnchor.x,
        map.mPixelH * mAnchor.y);
    std::vector<glm::vec2>  clip;
    for (const auto & point : points)
    {
        clip.push_back(GetOwner()->WorldToLocal(point));
    }
    for (const auto & convex : tools::StripConvexPoints(clip))
    {
        for (auto & point : tools::StripTrianglePoints(convex))
        {
            mEraseList.emplace_back(point + offset, zeroColor);
        }
    }
    HandleClip(clip);

    //  边缘处理
    auto order = tools::CalePointsOrder(points);
    ASSERT_LOG(order != 0, "");
    order = order >= 0.0f ? 1.0f : -1.0f;
    std::vector<RawMesh::Vertex> vertexs;
    auto count  = points.size();
    for (auto i = 0; i != count; ++i)
    {
        auto & a = points.at(i);
        auto & b = points.at((i + 1) % count);
        auto & c = points.at((i + 2) % count);
        auto ab = b - a;
        auto bc = c - b;
        auto abr = glm::vec2(ab.y, -ab.x);
        auto bcr = glm::vec2(bc.y, -bc.x);
        abr = glm::normalize(abr) * 5.0f * order;
        bcr = glm::normalize(bcr) * 5.0f * order;

        auto ab0 = a, ab1 = a + abr;
        auto ab2 = b, ab3 = b + abr;
        auto bc0 = b, bc1 = b + bcr;

        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab0) + offset, edgeColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab3) + offset, normColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab1) + offset, normColor);

        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab2) + offset, edgeColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab3) + offset, normColor);
        mEraseList.emplace_back(GetOwner()->WorldToLocal(ab0) + offset, edgeColor);

        if (glm::cross(ab, bc) * order >= 0)
        {
            mEraseList.emplace_back(GetOwner()->WorldToLocal(ab2) + offset, edgeColor);
            mEraseList.emplace_back(GetOwner()->WorldToLocal(ab3) + offset, normColor);
            mEraseList.emplace_back(GetOwner()->WorldToLocal(bc1) + offset, normColor);
        }
    }
}

std::vector<Component::Property> CompCollapseTerrain::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Map",      &mMap,      std::vector<uint>{ Res::TypeEnum::kMap  });
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset,   "Json",     &mJson,     std::vector<uint>{ Res::TypeEnum::kJson });
    props.emplace_back(UIParser::StringValueTypeEnum::kVector2, "Anchor",   &mAnchor);
    return std::move(props);
}

void CompCollapseTerrain::Init(UIObjectGLCanvas* canvas)
{
    mEraseList.clear();

    //  初始化地图.开始
    const auto & map = mMap.Instance<RawMap>();
    ASSERT_LOG(map->GetAtlass().size() == 1,"");

    RenderPipline::TargetCommand targetCommand;
    targetCommand.mRenderTextures[0] = mTexture;
    targetCommand.mClearColor = glm::vec4(0.0f);
    targetCommand.mClipView.x = 0;
    targetCommand.mClipView.y = 0;
    targetCommand.mClipView.z = (float)map->GetMap().mPixelW;
    targetCommand.mClipView.w = (float)map->GetMap().mPixelH;
    targetCommand.mType = RenderPipline::TargetCommand::kPush;
    targetCommand.mEnabledFlag = RenderPipline::RenderCommand::kTargetColor0
                               | RenderPipline::RenderCommand::kTargetColor1
                               | RenderPipline::RenderCommand::kClipView;
    canvas->Post(targetCommand);

    //  填充地图
    RenderPipline::FowardCommand fowardCommand;
    fowardCommand.mMesh = std::create_ptr<RawMesh>();
    fowardCommand.mMesh->Init(map->GetPoints(), { },
        RawMesh::Vertex::kV | RawMesh::Vertex::kUV);

    fowardCommand.mPairImages.emplace_back( "texture0",
        map->GetAtlass().front().mTexture->GetImage());

    fowardCommand.mProgram = mProgramInit;
    fowardCommand.mViewMat = glm::lookAt(glm::vec3(0, 0, 0.0f), glm::vec3(0, 0, -1.0f), glm::vec3(0, 1, 0.0f));
    fowardCommand.mProjMat = glm::ortho(0.0f, (float)map->GetMap().mPixelW, 0.0f, (float)map->GetMap().mPixelH);
    fowardCommand.mBlendSrc = GL_SRC_ALPHA;
    fowardCommand.mBlendDst = GL_ONE_MINUS_SRC_ALPHA;
    fowardCommand.mEnabledFlag = RenderPipline::RenderCommand::kProjMat
                               | RenderPipline::RenderCommand::kViewMat
                               | RenderPipline::RenderCommand::kBlend;
    canvas->Post(fowardCommand);

    //  初始化地图.结束
    targetCommand.mType = RenderPipline::TargetCommand::kPop;
    canvas->Post(targetCommand);

    //  初始化碰撞边框
    for (auto & area : mJson.Instance<mmc::Json>()->At("List"))
    {
        auto & points = mAreas.emplace_back();
        for (auto & point : area.mVal)
        {
            points.emplace_back(
                point.mVal->At("x")->ToNumber(),
                point.mVal->At("y")->ToNumber());
        }
    }
}

bool CompCollapseTerrain::Update(UIObjectGLCanvas * canvas)
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);
        
        if ( mMap.Check()  && mJson.Check() &&
            (mMap.Modify() || mJson.Modify()))
        {
            Init(canvas);
        }

        if (mMap.Check())
        {
            auto w = (iint)mMap.Instance<RawMap>()->GetMap().mPixelW;
            auto h = (iint)mMap.Instance<RawMap>()->GetMap().mPixelH;
            mTrackPoints.at(0).x = -w *      mAnchor.x;
            mTrackPoints.at(0).y = -h *      mAnchor.y;
            mTrackPoints.at(1).x =  w * (1 - mAnchor.x);
            mTrackPoints.at(1).y = -h *      mAnchor.y;
            mTrackPoints.at(2).x =  w * (1 - mAnchor.x);
            mTrackPoints.at(2).y =  h * (1 - mAnchor.y);
            mTrackPoints.at(3).x = -w *      mAnchor.x;
            mTrackPoints.at(3).y =  h * (1 - mAnchor.y);
        }

        mMesh->Update({
            { mTrackPoints.at(0), glm::vec2(0, 0) },
            { mTrackPoints.at(1), glm::vec2(1, 0) },
            { mTrackPoints.at(2), glm::vec2(1, 1) },

            { mTrackPoints.at(0), glm::vec2(0, 0) },
            { mTrackPoints.at(2), glm::vec2(1, 1) },
            { mTrackPoints.at(3), glm::vec2(0, 1) },
        }, {}, GL_DYNAMIC_DRAW, GL_STATIC_READ);
    }
    return mMap.Check() && mJson.Check();
}

void CompCollapseTerrain::ClearErase(UIObjectGLCanvas * canvas)
{
    ASSERT_LOG(!mEraseList.empty(), "");
    const auto & map = mMap.Instance<RawMap>();
    RenderPipline::TargetCommand  targetCommand;
    targetCommand.mRenderTextures[0] = mTexture;
    targetCommand.mClipView.x = 0;
    targetCommand.mClipView.y = 0;
    targetCommand.mClipView.z = (float)map->GetMap().mPixelW;
    targetCommand.mClipView.w = (float)map->GetMap().mPixelH;
    targetCommand.mType        = RenderPipline::TargetCommand::kPush;
    targetCommand.mEnabledFlag = RenderPipline::RenderCommand::kClipView;
    canvas->Post(targetCommand);

    RenderPipline::FowardCommand fowardCommand;
    fowardCommand.mMesh = std::create_ptr<RawMesh>();
    fowardCommand.mMesh->Init(mEraseList, {}, RawMesh::Vertex::kV | RawMesh::Vertex::kC);

    fowardCommand.mProgram = mProgramDraw;
    fowardCommand.mViewMat = glm::lookAt(glm::vec3(0, 0, 0.0f), glm::vec3(0, 0, -1.0f), glm::vec3(0, 1, 0.0f));
    fowardCommand.mProjMat = glm::ortho(0.0f, (float)map->GetMap().mPixelW, 0.0f, (float)map->GetMap().mPixelH);

    fowardCommand.mBlendSrc = GL_DST_ALPHA;
    fowardCommand.mBlendDst = GL_SRC_ALPHA;

    fowardCommand.mEnabledFlag = RenderPipline::RenderCommand::kViewMat
                               | RenderPipline::RenderCommand::kProjMat
                               | RenderPipline::RenderCommand::kBlend;
    canvas->Post(fowardCommand);

    targetCommand.mType = RenderPipline::TargetCommand::kPop;
    canvas->Post(targetCommand);
}

auto CompCollapseTerrain::GenClipNums(const Clip & clip) -> ClipNums
{
    ClipNums result;
    auto clip2=clip;
    for (auto & area : mAreas)
    {
        if (UpdateClip(area, clip2))
        {
            auto cps = GenClipPoint(area,clip2);
            result.emplace_back(cps.size() / 2);
        }
        else
        {
            result.emplace_back(0);
        }
    }
    return std::move(result);
}

auto CompCollapseTerrain::GenClipLine(const Clip & clip, const ClipPoint & cp0, const ClipPoint & cp1) -> ClipLine
{
    ClipLine result;
    result.mAreaEnd0 = cp0.mAreaIdx1;
    result.mAreaEnd1 = cp1.mAreaIdx1;

    result.mLine.emplace_back(glm::lerp(clip.at(cp0.mClipIdx0), clip.at(cp0.mClipIdx1), cp0.mClipCross));
    for (auto i = cp0.mClipIdx1; 
             i != cp1.mClipIdx1; 
             i  = (i + 1) % clip.size())
    {
        result.mLine.emplace_back(clip.at(i));
    }
    result.mLine.emplace_back(glm::lerp(clip.at(cp1.mClipIdx0), clip.at(cp1.mClipIdx1), cp1.mClipCross));

    if (cp0.mAreaIdx1 == cp1.mAreaIdx1 && cp0.mAreaCross < cp1.mAreaCross)
    {
        std::reverse(result.mLine.begin(), result.mLine.end());
    }
    return std::move(result);
}

auto CompCollapseTerrain::GenClipPoint(const Area & area, const Clip & clip, bool onlyone) -> std::vector<ClipPoint>
{
    using Cross = std::tuple<uint, uint, float, float>;
    std::vector<ClipPoint>  results;
    std::vector<Cross>      cross;
    for (auto i = 0u, size = clip.size(); i != size; ++i)
    {
        cross.clear();
        auto j = (i + 1) % size;
        auto & a = clip.at(i);
        auto & b = clip.at(j);
        ASSERT_LOG(a != b,"");
        if (tools::IsCrossSegment(a, b, area, &cross))
        {
            std::sort(cross.begin(), cross.end(), [] (const auto & a, const auto & b)
                {
                    return std::get<3>(a) < std::get<3>(b);
                });

            for (auto k = 0; k != cross.size(); ++k)
            {
                auto ok = (results.size() & 1) != 0;
                if (!ok && (cross.size() == 1 + k || cross.size() == 1))
                {
                    ok = tools::IsContains(area, b, false);
                }
                if (!ok && (cross.size() != 1 + k))
                {
                    const auto & curr = cross.at(k);
                    const auto & next = cross.at(k + 1);
                    auto p0 = area.at(std::get<0>(curr));
                    auto p1 = area.at(std::get<1>(curr));
                    auto p2 = area.at(std::get<0>(next));
                    auto p3 = area.at(std::get<1>(next));
                    auto v0 = glm::lerp(p0, p1, std::get<2>(curr));
                    auto v1 = glm::lerp(p2, p3, std::get<2>(next));
                    ok = tools::DistanceSqrt(p0, v0) > 1 && tools::DistanceSqrt(p1, v0) > 1 ||
                         tools::DistanceSqrt(p2, v1) > 1 && tools::DistanceSqrt(p3, v1) > 1;
                }
                if (ok)
                {
                    results.emplace_back(
                        std::get<0>(cross.at(k)),
                        std::get<1>(cross.at(k)),
                        std::get<2>(cross.at(k)),
                        i, j, std::get<3>(cross.at(k)));
                }
                if (onlyone && results.size() == 2) { break; }
            }
        }
        if (onlyone && results.size() == 2) { break; }
    }
    return std::move(results);
}

void CompCollapseTerrain::BinaryPoints(const Area & area, const Clip & clip, const ClipLine & clipLine, std::vector<glm::vec2> * output)
{
    auto size = area.size();
    for (auto i = clipLine.mAreaEnd0;
        clipLine.mAreaEnd1 != i ||
        output[0].empty();
        i = (i +1) % size)
    {
        output[0].emplace_back(area.at(i));
    }
    std::copy(clipLine.mLine.rbegin(), clipLine.mLine.rend(), std::back_inserter(output[0]));

    for (auto i = clipLine.mAreaEnd1; 
        clipLine.mAreaEnd0 != clipLine.mAreaEnd1 &&
        clipLine.mAreaEnd0 != i; i = (i + 1) % size)
    {
        output[1].emplace_back(area.at(i));
    }
    std::copy(clipLine.mLine.begin(), clipLine.mLine.end(), std::back_inserter(output[1]));
}

bool CompCollapseTerrain::UpdateClip(const Area & area, Clip & clip)
{
    auto it = std::find_if(clip.begin(), clip.end(), [&] (const glm::vec2 & point)
        {
            return !tools::IsContains(area, point);
        });
    if (it != clip.end())
    {
        std::rotate(clip.begin(), it, clip.end());
    }
    return it != clip.end();
}

void CompCollapseTerrain::HandleClip(const Clip & clip, const std::vector<Area> & input, std::vector<Area> & output)
{
    Area binary[2];
    for (auto & area : input)
    {
        auto cps = GenClipPoint(area, clip, true);
        if (cps.empty())
        {
            output.emplace_back(area);
        }
        else
        {
            ASSERT_LOG(cps.size() != 1, "");
            auto & p0 = cps.at(0);
            auto & p1 = cps.at(1);
            auto clipLine = GenClipLine(clip, p0, p1);
            BinaryPoints(area, clip, clipLine,binary);
            if (!IsContains(clip, binary[0]))
            {
                output.emplace_back(std::move(binary[0]));
            }
            if (!IsContains(clip, binary[1]))
            {
                output.emplace_back(std::move(binary[1]));
            }
        }
    }
}

void CompCollapseTerrain::HandleClip(const Clip & clip)
{
    auto clipNums = GenClipNums(clip);
    std::vector<Clip> buffers[2];
    auto clip2            = clip;
    for (auto i = 0; i != clipNums.size(); ++i)
    {
        buffers[0].clear();
        buffers[1].clear();
        buffers[0].emplace_back(std::move(mAreas.at(i)));
        if (UpdateClip(buffers[0].front(), clip2))
        {
            for (auto j = 0; j != clipNums.at(i); ++j)
            {
                HandleClip(clip2, buffers[0], buffers[1]);
                buffers[0]        = std::move(buffers[1]);
            }
            if (clipNums.at(i) == 0 && IsContains(clip, buffers[0].front()))
            {
                buffers[0].clear();
            }
        }
        mAreas.insert(mAreas.end(), buffers[0].begin(), buffers[0].end());
    }
    mAreas.erase(mAreas.begin(), mAreas.begin() + clipNums.size());
}

bool CompCollapseTerrain::IsContains(const std::vector<glm::vec2> & points0, const std::vector<glm::vec2> & points1)
{
    //  顶点是否在边附近
    for (auto & p : points1)
    {
        auto pass = true;
        auto size = points0.size();
        for (auto i = 0; i != size; ++i)
        {
            auto j = (i + 1) % size;
            auto & a = points0.at(i);
            auto & b = points0.at(j);
            auto [cross, diff] = tools::Distance(a, b, p);
            if (glm::length_sqrt(diff)<10) { pass=false; }
        }
        if (pass && !tools::IsContains(points0, p)) { return false; }
    }
    return true;
}

void CompCollapseTerrain::DebugPostDrawPolygons(UIObjectGLCanvas * canvas)
{
    for (auto & polygon : mAreas)
    {
        DebugPostDrawPolygon(canvas, polygon);
    }
}

void CompCollapseTerrain::DebugPostDrawPolygon(UIObjectGLCanvas * canvas, const Area & polygon)
{
    std::vector<RawMesh::Vertex> points;
    auto count = polygon.size();
    for (auto i = 0; i != count; ++i)
    {
        auto & a = polygon.at(i);
        auto & b = polygon.at((i + 1) % count);
        auto r = b - a;r = glm::vec2(r.y,-r.x);
        r = glm::normalize(r) * 2.0f;

        auto p0 = a - r;
        auto p1 = a + r;
        auto p2 = b - r;
        auto p3 = b + r;

        auto color = glm::vec4(1, 1, 1, 1);
        points.emplace_back(p0, color);
        points.emplace_back(p1, color);
        points.emplace_back(p3, color);

        points.emplace_back(p0, color);
        points.emplace_back(p3, color);
        points.emplace_back(p2, color);
    }

    RenderPipline::FowardCommand command;
    command.mMesh       = std::create_ptr<RawMesh>();
    command.mMesh->Init(points, {}, RawMesh::Vertex::kV | RawMesh::Vertex::kC);
    command.mProgram    = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_SOLID_FILL);
    command.mTransform  = canvas->GetMatrixStack().GetM();

    canvas->Post(command);
}
