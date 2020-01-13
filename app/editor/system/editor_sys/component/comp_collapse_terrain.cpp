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

        DebugPostDrawPolygons(canvas);
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
    std::vector<glm::vec2> clipLine;
    for (const auto & point: points)
    {
        clipLine.push_back(GetOwner()->WorldToLocal(point));
    }
    if (ClearErase(clipLine))
    {
        for (const auto & convex : tools::StripConvexPoints(clipLine))
        {
            for (auto & point : tools::StripTrianglePoints(convex))
            {
                mEraseList.emplace_back(point + offset, zeroColor);
            }
        }

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
        auto & points = mPolygons.emplace_back();
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

bool CompCollapseTerrain::ClearErase(const std::vector<glm::vec2> & points)
{
    std::vector<Polygon> polygons[2];
    polygons[0]=std::move(mPolygons);

    bool isCheckClip = false;
    for (auto clips = points;
        ClearErase(clips, polygons[0], polygons[1], &isCheckClip);
        polygons[0].clear(), std::swap(polygons[0], polygons[1]));
    mPolygons = std::move(polygons[1]);

    return isCheckClip;
}

bool CompCollapseTerrain::ClearErase(std::vector<glm::vec2> & points, std::vector<Polygon> & polygons0, std::vector<Polygon> & polygons1, bool * isCheckClip)
{
    //  调整切线集, 使得第一条切线起点不在多边形内
    auto UpdatePoints = [] (std::vector<glm::vec2> & points, const std::vector<glm::vec2> & polygon)
    {
        auto it = std::find_if(points.begin(), points.end(), [&polygon] (const glm::vec2 & point)
            {
                return !tools::IsContains(polygon, point, false);
            });
        if (it != points.end())
        {
            std::rotate(points.begin(), it, points.end());
        }
        return it != points.end();
    };

    std::vector<glm::vec2> result[2];
    for (auto & polygon : polygons0)
    {
        auto bskip = false;
        if (UpdatePoints(points, polygon))
        {
            if (auto [cross, endA, endB, clipLine] = CrossResult(points, polygon); cross)
            {
                bskip = true; result[0].clear(); result[1].clear();
                BinaryPoints(endA, endB, polygon, clipLine,result);
                if (!IsContains(points, result[0]))
                {
                    polygons1.emplace_back(std::move(result[0]));
                }
                if (!IsContains(points, result[1]))
                {
                    polygons1.emplace_back(std::move(result[1]));
                }
                *isCheckClip = *isCheckClip || true;
            }
            else
            {
                bskip = tools::IsContains(points, polygon);
            }
        }
        if (!bskip) { polygons1.emplace_back(polygon); }
        int debug = 0;
    }
    return polygons0.size() != polygons1.size();
}

auto CompCollapseTerrain::CrossResult(const std::vector<glm::vec2> & points, const std::vector<glm::vec2> & polygon) -> std::tuple<bool, uint, uint, std::vector<glm::vec2>>
{
    //  PointsBeg, PointsEnd, PointsCross, PolygonBeg, PolygonEnd, PolygonCross
    std::vector<std::tuple<uint, uint, float, uint, uint, float>> result1;
    //  PolygonBeg, PolygonEnd, PolygonCross, PointsCross,
    std::vector<std::tuple<uint, uint, float, float>>             result0;
    auto size   = points.size();
    for (auto i = 0; i != size && result1.size() != 2; ++i)
    {
        result0.clear();
        auto j = (i + 1) % size;
        auto & a = points.at(i);
        auto & b = points.at(j);
        ASSERT_LOG(a != b, " ");
        if (tools::IsCrossSegment(a, b, polygon, &result0))
        {
            std::sort(result0.begin(), result0.end(), [] (const auto & a, const auto & b)
                {
                    return std::get<3>(a) < std::get<3>(b);
                });

            for (auto k = 0; k != result0.size() && result1.size() != 2; ++k)
            {
                if (result1.empty())
                {
                    if (result0.size() == 1)
                    {
                        if (tools::IsContains(polygon, b, false))
                        {
                            result1.emplace_back(i, j, std::get<3>(result0.front()), std::get<0>(result0.front()),
                                                       std::get<1>(result0.front()), std::get<2>(result0.front()));
                        }
                    }
                    else
                    {
                        if (k + 1 == result0.size()) { break; }
                        const auto & curr = result0.at(k);
                        const auto & next = result0.at(k + 1);
                        if (!tools::Equal(std::get<2>(next), 0.0f) && !tools::Equal(std::get<2>(next), 1.0f) ||
                            !tools::Equal(std::get<2>(curr), 0.0f) && !tools::Equal(std::get<2>(curr), 1.0f) ||
                            !tools::Equal(std::get<3>(curr), std::get<3>(next)) &&
                            (tools::Equal(std::get<2>(curr), 0.0f) && std::abs((int)std::get<0>(curr) - (int)std::get<1>(next)) != 1 ||
                             tools::Equal(std::get<2>(curr), 1.0f) && std::abs((int)std::get<1>(curr) - (int)std::get<0>(next)) != 1))
                        {
                            result1.emplace_back(i, j, std::get<3>(curr), std::get<0>(curr),
                                                       std::get<1>(curr), std::get<2>(curr));
                        }
                    }
                }
                else
                {
                    result1.emplace_back(i, j, std::get<3>(result0.at(k)), std::get<0>(result0.at(k)),
                                               std::get<1>(result0.at(k)), std::get<2>(result0.at(k)));
                }
            }
        }
    }

    std::vector<glm::vec2> clipLine;
    if (result1.size() == 2)
    {
        auto beg = glm::lerp(
            points.at(std::get<0>(result1.at(0))),
            points.at(std::get<1>(result1.at(0))),
            std::get<2>(result1.at(0)));

        auto end = glm::lerp(
            points.at(std::get<0>(result1.at(1))),
            points.at(std::get<1>(result1.at(1))),
            std::get<2>(result1.at(1)));

        clipLine.emplace_back(beg);
        clipLine.emplace_back(end);

        for (auto i = std::get<1>(result1.at(0));
                 i != std::get<1>(result1.at(1));
                 i  = (i + 1) % points.size())
        {
            clipLine.insert(std::prev(clipLine.end()), points.at(i));
        }

        if (std::get<4>(result1.at(0)) == std::get<4>(result1.at(1)) &&
            std::get<5>(result1.at(0))  < std::get<5>(result1.at(1)))
        {
            std::reverse(clipLine.begin(), clipLine.end());
        }
        return std::make_tuple(true,
            std::get<4>(result1.at(0)),
            std::get<4>(result1.at(1)), clipLine);
    }
    return std::make_tuple(false, 0, 0, clipLine);
}

void CompCollapseTerrain::BinaryPoints(uint endA, uint endB, const std::vector<glm::vec2> & points, const std::vector<glm::vec2> & clipLine, std::vector<glm::vec2> * output)
{
    for (auto i = endA; output[0].empty() || i != endB; i = (i + 1) % points.size())
    {
        output[0].emplace_back(points.at(i));
    }
    std::copy(clipLine.rbegin(), clipLine.rend(), std::back_inserter(output[0]));

    for (auto i = endB; endB != endA && i != endA; i = (i + 1) % points.size())
    {
        output[1].emplace_back(points.at(i));
    }
    std::copy(clipLine.begin(), clipLine.end(), std::back_inserter(output[1]));
}

bool CompCollapseTerrain::IsContains(const std::vector<glm::vec2> & points0, const std::vector<glm::vec2> & points1)
{
    //  是否包含在内
    if (tools::IsContains(points0, points1))
    {
        return true;
    }

    //  顶点是否在边附近
    for (auto & p : points1)
    {
        auto skip = true;
        auto size = points0.size();
        for (auto i = 0; i != size; ++i)
        {
            auto j = (i + 1) % size;
            auto & a = points0.at(i);
            auto & b = points0.at(j);
            auto [cross, diff] = tools::Distance(a, b, p);
            if (glm::length_sqrt(diff)>10) { skip=false; }
        }
        if (skip) { return false; }
    }
    return true;
}

void CompCollapseTerrain::DebugPostDrawPolygons(UIObjectGLCanvas * canvas)
{
    for (auto & polygon : mPolygons)
    {
        DebugPostDrawPolygon(canvas, polygon);
    }
}

void CompCollapseTerrain::DebugPostDrawPolygon(UIObjectGLCanvas * canvas, const Polygon & polygon)
{
    std::vector<RawMesh::Vertex> points;
    auto count = polygon.size();
    for (auto i = 0; i != count; ++i)
    {
        auto & a = polygon.at(i);
        auto & b = polygon.at((i + 1) % count);
        auto r = b - a;r = glm::vec2(r.y,-r.x);
        r = glm::normalize(r) * 1.0f;

        auto p0 = a - r;
        auto p1 = a + r;
        auto p2 = b - r;
        auto p3 = b + r;

        points.emplace_back(p0, glm::vec4(1, 1, 1, 1));
        points.emplace_back(p1, glm::vec4(1, 1, 1, 1));
        points.emplace_back(p3, glm::vec4(1, 1, 1, 1));

        points.emplace_back(p0, glm::vec4(1, 1, 1, 1));
        points.emplace_back(p3, glm::vec4(1, 1, 1, 1));
        points.emplace_back(p2, glm::vec4(1, 1, 1, 1));
    }

    RenderPipline::FowardCommand command;
    command.mMesh       = std::create_ptr<RawMesh>();
    command.mMesh->Init(points, {}, RawMesh::Vertex::kV | RawMesh::Vertex::kC);
    command.mProgram    = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_SOLID_FILL);;
    command.mTransform  = canvas->GetMatrixStack().GetM();

    canvas->Post(command);
}
