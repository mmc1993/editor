#include "comp_lightning.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"

CompLightning::CompLightning()
    : _color(1)
    , _scale(1)
    , _width(1)
    , _update(kTexture | kSegment | kMesh)
{
    _trackPoints.emplace_back(0, 0  );
    _trackPoints.emplace_back(0, 100);

    _mesh = std::create_ptr<GLMesh>();
    _mesh->Init({},{}, GLMesh::Vertex::kV | 
                       GLMesh::Vertex::kC |
                       GLMesh::Vertex::kUV);

    _program = std::create_ptr<GLProgram>();
    _program->Init(tools::GL_PROGRAM_LIGHTNING);

    AddState(StateEnum::kModifyTrackPoint, true);
    AddState(StateEnum::kInsertTrackPoint, true);
    AddState(StateEnum::kDeleteTrackPoint, true);
}

void CompLightning::OnUpdate(UIObjectGLCanvas * canvas, float dt)
{
    Update();

    if (_texture != nullptr)
    {
        interface::FowardCommand command;
        command.mMesh       = _mesh;
        command.mProgram    = _program;
        command.mTransform  = canvas->GetMatrixStack().GetM();
        command.mTextures.push_back(std::make_pair("texture0", _texture));
        canvas->Post(command);
    }
}

const std::string & CompLightning::GetName()
{
    static const std::string name = "Lightning";
    return name;
}

void CompLightning::EncodeBinary(std::ofstream & os)
{
    Component::EncodeBinary(os);
    tools::Serialize(os, _url);
    tools::Serialize(os, _scale);
    tools::Serialize(os, _width);
    tools::Serialize(os, _color);
    tools::Serialize(os, _trackPoints);
}

void CompLightning::DecodeBinary(std::ifstream & is)
{
    Component::DecodeBinary(is);
    tools::Deserialize(is, _url);
    tools::Deserialize(is, _scale);
    tools::Deserialize(is, _width);
    tools::Deserialize(is, _color);
    tools::Deserialize(is, _trackPoints);
}

bool CompLightning::OnModifyProperty(const std::any & oldValue, const std::any & newValue, const std::string & title)
{
    AddState(StateEnum::kUpdate, true);
    if (title == "Url")
    {
        _update |= kTexture;
    }
    else if (title == "Width")
    {
        _update |= kSegment;
    }
    else if (title == "Scale" 
          || title == "Color")
    {
        _update |= kMesh;
    }
    return true;
}

std::vector<Component::Property> CompLightning::CollectProperty()
{
    auto props = Component::CollectProperty();
    props.emplace_back(UIParser::StringValueTypeEnum::kAsset, "Url", &_url);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Scale", &_scale);
    props.emplace_back(UIParser::StringValueTypeEnum::kFloat, "Width", &_width);
    props.emplace_back(UIParser::StringValueTypeEnum::kColor4, "Color", &_color);
    return std::move(props);
}

void CompLightning::Update()
{
    if (HasState(StateEnum::kUpdate))
    {
        AddState(StateEnum::kUpdate, false);

        if (!_url.empty() && _update & kTexture)
        {
            _texture = Global::Ref().mRawSys->Get<GLTexture>(_url);
            _texture->GetRefImage()->SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            _texture->GetRefImage()->SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        if (!_url.empty() && _update & kSegment)
        {
            std::queue<Segment> input;
            for (auto i = 0; i != _trackPoints.size() - 1; ++i)
            {
                auto & a = _trackPoints.at(i    );
                auto & b = _trackPoints.at(i + 1);
                input.emplace(a, b, 0);
            }
            GenSegm(_width, input, _segments);
        }

        if (!_url.empty() && (_update & (kSegment | kMesh)))
        {
            GenMesh(_scale, _segments);
        }

        _update = 0;
    }
}

void CompLightning::GenSegm(float width, std::queue<Segment> & input, std::vector<Segment> & output)
{
    output.clear();
    while (!input.empty())
    {
        auto & segment = input.front();
        if (auto len = glm::length(segment.mEnded - segment.mStart); len >= width)
        {
            auto mid = glm::lerp(segment.mStart, segment.mEnded, 0.5f);
            auto dir = glm::normalize(segment.mEnded - segment.mStart);
            auto offset = glm::vec2(dir.y, -dir.x) * len;
            offset *= tools::Random(-1.0f, +1.0f) * 0.3f;
            input.emplace(segment.mStart, mid + offset, segment.mBranch);
            input.emplace(mid + offset, segment.mEnded, segment.mBranch);
        }
        else
        {
            if (tools::Random(0.0f, 1.0f) < std::pow(0.06f, segment.mBranch + 1) - 0.01f)
            {
                //  产生分支
                auto ended = glm::lerp(segment.mStart, segment.mEnded, 5.0f);
                auto length = glm::length(ended - segment.mStart);
                auto offset = glm::vec2(tools::Random(-length, length),
                                        tools::Random(-length, length));
                input.emplace(segment.mStart, ended + offset, segment.mBranch + 1);
            }
            output.emplace_back(segment);
        }
        input.pop();
    }
}

void CompLightning::GenMesh(float scale, const std::vector<Segment> & segments)
{
    std::vector<GLMesh::Vertex> points;
    for (auto & segment : segments)
    {
        auto mid = glm::lerp(segment.mStart, segment.mEnded, 0.5f);
        auto dir = glm::vec2(+(segment.mEnded - segment.mStart).y, 
                             -(segment.mEnded - segment.mStart).x);
        auto width = glm::length(segment.mEnded - segment.mStart);
        dir = glm::normalize(dir) * width * 0.5f;

        auto p0 = mid + (segment.mStart + dir - mid) * scale;
        auto p1 = mid + (segment.mEnded + dir - mid) * scale;
        auto p2 = mid + (segment.mEnded - dir - mid) * scale;
        auto p3 = mid + (segment.mStart - dir - mid) * scale;

        points.emplace_back(p0, _color, glm::vec2(0, 0));
        points.emplace_back(p1, _color, glm::vec2(1, 0));
        points.emplace_back(p2, _color, glm::vec2(1, 1));

        points.emplace_back(p0, _color, glm::vec2(0, 0));
        points.emplace_back(p2, _color, glm::vec2(1, 1));
        points.emplace_back(p3, _color, glm::vec2(0, 1));
    }
    _mesh->Update(points, { });
}

void CompLightning::OnModifyTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _update |= UpdateEnum::kSegment;
    _trackPoints.at(index) = point;
}

void CompLightning::OnInsertTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    _update |= UpdateEnum::kSegment;
    _trackPoints.insert(std::next(_trackPoints.begin(), index), point);
}

void CompLightning::OnDeleteTrackPoint(const size_t index, const glm::vec2 & point)
{
    AddState(StateEnum::kUpdate, true);
    if (_trackPoints.size() > 1)
    {
        _update |= UpdateEnum::kSegment;
        _trackPoints.erase(std::next(_trackPoints.begin(), index));
    }
}

