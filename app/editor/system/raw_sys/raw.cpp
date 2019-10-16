#include "raw.h"
#include "raw_sys.h"

// ---
//  GLMesh
// ---
GLMesh::GLMesh() : _vao(0), _vbo(0), _ebo(0), _vCount(0), _eCount(0)
{ }

GLMesh::~GLMesh()
{
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
    glDeleteVertexArrays(1, &_vao);
}

bool GLMesh::Init(const std::string & url)
{
    //  _TODO
    //  暂时用不上
    return true;
}

void GLMesh::Init(const std::vector<Vertex> & points, const std::vector<uint> & indexs, uint enabled, uint vUsage, uint eUsage)
{
    Init(points.data(), (uint)points.size(), indexs.data(), (uint)indexs.size(), enabled, vUsage, eUsage);
}

void GLMesh::Init(const Vertex * points, const uint pointsLength, const uint * indexs, const uint indexsLength, uint enabled, uint vUsage, uint eUsage)
{
    _vCount = pointsLength;
    _eCount = indexsLength;

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _vCount, points, vUsage);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * _eCount, indexs, eUsage);

    auto idx = 0;
    if (Vertex::kV & enabled)
    {
        glVertexAttribPointer(idx, decltype(Vertex::v)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
        glEnableVertexAttribArray(idx++);
    }
    if (Vertex::kC & enabled)
    {
        glVertexAttribPointer(idx, decltype(Vertex::c)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, c));
        glEnableVertexAttribArray(idx++);
    }
    if (Vertex::kUV & enabled)
    {
        glVertexAttribPointer(idx, decltype(Vertex::uv)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
        glEnableVertexAttribArray(idx++);
    }
    glBindVertexArray(0);
}

void GLMesh::Update(const std::vector<Vertex> & points, const std::vector<uint> & indexs, uint vUsage, uint eUsage)
{
    _vCount = points.size();
    _eCount = indexs.size();
    glBindBuffer(GL_ARRAY_BUFFER,         _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ARRAY_BUFFER,         sizeof(Vertex) * points.size(), points.data(), vUsage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)   * indexs.size(), indexs.data(), eUsage);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// ---
//  GLTexture
// ---
GLTexture::GLTexture() : _fmt(0), _id(0), _w(0), _h(0)
{ }

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &_id);
}

bool GLTexture::Init(const std::string & url)
{
    int w, h, fmt;
    auto data = stbi_load(url.c_str(), &w, &h, &fmt, 0);
    ASSERT_LOG(data != nullptr, "URL: {0}", url);
    CHECK_RET(data != nullptr, false);

    switch (fmt)
    {
    case 1: _fmt = GL_RED; break;
    case 3: _fmt = GL_RGB; break;
    case 4: _fmt = GL_RGBA; break;
    }
    _w = w;
    _h = h;
    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexImage2D( GL_TEXTURE_2D, 0, _fmt, _w, _h, 0, _fmt, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return true;
}

GLProgram::GLProgram() : _use(GL_INVALID_INDEX)
{ }

GLProgram::~GLProgram()
{
    for (auto & pass : _passs)
    {
        glDeleteProgram(pass.mID);
    }
}

bool GLProgram::Init(const std::string & url)
{
    //  解析Shader
    const auto ParseShader = [&](
        std::stringstream & is,
        const char * endflag,
        std::string & buffer)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (tools::IsEqualSkipSpace(line, endflag))
            {
                break;
            }
            buffer.append(line);
            buffer.append("\n");
        }
        ASSERT_LOG(tools::IsEqualSkipSpace(line, endflag), "EndFlag Error: {0}", endflag);
    };

    //  解析Pass
    const auto ParsePass = [&](
        std::stringstream & is,
        const char * endFlag,
        std::string & vBuffer,
        std::string & fBuffer,
        GLProgram::Pass * pass)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (!tools::IsPrint(line)) { continue; }

            if (tools::IsEqualSkipSpace(line, endFlag))
            {
                break;
            }

            if (tools::IsEqualSkipSpace(line, "BlendMode") || 
                tools::IsEqualSkipSpace(line, "StencilTest"))
            {
                ASSERT_LOG(pass != nullptr, "解析Pass属性错误: {0}, {1}", endFlag, line);
                std::stringstream ss;
                std::string word;
                ss.str(line);
                ss >> word;

                if (word == "BlendMode")
                {
                    ss >> word;
                    if (word == "Zero") { pass->mBlendSrc = GL_ZERO; }
                    else if (word == "One") { pass->mBlendSrc = GL_ONE; }
                    else if (word == "SrcColor") { pass->mBlendSrc = GL_SRC_COLOR; }
                    else if (word == "SrcAlpha") { pass->mBlendSrc = GL_SRC_ALPHA; }
                    else if (word == "DstAlpha") { pass->mBlendSrc = GL_DST_ALPHA; }
                    else if (word == "OneMinusSrcColor") { pass->mBlendSrc = GL_ONE_MINUS_SRC_COLOR; }
                    else if (word == "OneMinusSrcAlpha") { pass->mBlendSrc = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (word == "OneMinusDstAlpha") { pass->mBlendSrc = GL_ONE_MINUS_DST_ALPHA; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Zero") { pass->mBlendDst = GL_ZERO; }
                    else if (word == "One") { pass->mBlendDst = GL_ONE; }
                    else if (word == "SrcColor") { pass->mBlendDst = GL_SRC_COLOR; }
                    else if (word == "SrcAlpha") { pass->mBlendDst = GL_SRC_ALPHA; }
                    else if (word == "DstAlpha") { pass->mBlendDst = GL_DST_ALPHA; }
                    else if (word == "OneMinusSrcColor") { pass->mBlendDst = GL_ONE_MINUS_SRC_COLOR; }
                    else if (word == "OneMinusSrcAlpha") { pass->mBlendDst = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (word == "OneMinusDstAlpha") { pass->mBlendDst = GL_ONE_MINUS_DST_ALPHA; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
                }
                else if (word == "StencilTest")
                {
                    ss >> word;
                    if (word == "Keep") { pass->mStencilOpFail = GL_KEEP; }
                    else if (word == "Zero") { pass->mStencilOpFail = GL_ZERO; }
                    else if (word == "Incr") { pass->mStencilOpFail = GL_INCR; }
                    else if (word == "Decr") { pass->mStencilOpFail = GL_DECR; }
                    else if (word == "Invert") { pass->mStencilOpFail = GL_INVERT; }
                    else if (word == "Replace") { pass->mStencilOpFail = GL_REPLACE; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Keep") { pass->mStencilOpZFail = GL_KEEP; }
                    else if (word == "Zero") { pass->mStencilOpZFail = GL_ZERO; }
                    else if (word == "Incr") { pass->mStencilOpZFail = GL_INCR; }
                    else if (word == "Decr") { pass->mStencilOpZFail = GL_DECR; }
                    else if (word == "Invert") { pass->mStencilOpZFail = GL_INVERT; }
                    else if (word == "Replace") { pass->mStencilOpZFail = GL_REPLACE; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Keep") { pass->mStencilOpZPass = GL_KEEP; }
                    else if (word == "Zero") { pass->mStencilOpZPass = GL_ZERO; }
                    else if (word == "Incr") { pass->mStencilOpZPass = GL_INCR; }
                    else if (word == "Decr") { pass->mStencilOpZPass = GL_DECR; }
                    else if (word == "Invert") { pass->mStencilOpZPass = GL_INVERT; }
                    else if (word == "Replace") { pass->mStencilOpZPass = GL_REPLACE; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Never") { pass->mStencilFunc = GL_NEVER; }
                    else if (word == "Less") { pass->mStencilFunc = GL_LESS; }
                    else if (word == "Equal") { pass->mStencilFunc = GL_EQUAL; }
                    else if (word == "Greater") { pass->mStencilFunc = GL_GREATER; }
                    else if (word == "NotEqual") { pass->mStencilFunc = GL_NOTEQUAL; }
                    else if (word == "Gequal") { pass->mStencilFunc = GL_GEQUAL; }
                    else if (word == "Always") { pass->mStencilFunc = GL_ALWAYS; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word; pass->mStencilMask = std::stoi(word);
                    ss >> word; pass->mStencilRef  = std::stoi(word);
                }
                else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
            }
            else if (tools::IsEqualSkipSpace(line, "VShader Beg")) { ParseShader(is, "VShader End", vBuffer); }
            else if (tools::IsEqualSkipSpace(line, "FShader Beg")) { ParseShader(is, "FShader End", fBuffer); }
            else
            {
                vBuffer.append(line);
                vBuffer.append("\n");
                fBuffer.append(line);
                fBuffer.append("\n");
            }
        }
        ASSERT_LOG(tools::IsEqualSkipSpace(line, endFlag), "EndFlag Error: {0}", endFlag);
    };

    //  解析Include
    const auto ParseInclude = [](const std::string & word)
    {
        auto pos = word.find_last_of(' ');
        ASSERT_LOG(pos != std::string::npos, word.c_str());
        auto url = word.substr(pos + 1);

        std::ifstream is(url);
        ASSERT_LOG(is, url.c_str());

        std::string data;
        std::string line;
        while (std::getline(is, line))
        {
            data.append(line);
            data.append("\n");
        }
        is.close();
        return data;
    };

    //  替换Include
    const auto OpenProgramFile = [&](const std::string & url)
    {
        std::string line, data;
        std::ifstream is(url);
        ASSERT_LOG(is, "URL Error: {0}", url);
        while (std::getline(is, line))
        {
            if (tools::IsEqualSkipSpace(line, "#include"))
            {
                data.append(ParseInclude(line));
                data.append("\n");
            }
            else
            {
                data.append(line);
                data.append("\n");
            }
        }
        is.close();

        std::stringstream ss;
        ss.str(data);
        return std::move(ss);
    };

    //  解析GL Program数据
    auto is = OpenProgramFile(url);
    std::string line;
    std::string vShader, fShader;
    while (std::getline(is, line))
    {
        if (tools::IsEqualSkipSpace(line, "Common Beg"))
        {
            ParsePass(is, "Common End", vShader, fShader, nullptr);
        }
        else if (tools::IsEqualSkipSpace(line, "Pass Beg"))
        {
            Pass pass;
            std::string vBuffer(vShader);
            std::string fBuffer(fShader);
            ParsePass(is, "Pass End", vBuffer, fBuffer, &pass);
            AddPass(pass, vBuffer.c_str(), vBuffer.size(), fBuffer.c_str(), fBuffer.size());
        }
    }
    return true;
}

uint GLProgram::GetUse()
{
    return _use;
}

uint GLProgram::GetPassCount()
{
    return (uint)_passs.size();
}

void GLProgram::UsePass(uint i)
{
    const auto & pass = _passs.at(i);
    //  开启混合
    if (pass.mBlendSrc != 0 && pass.mBlendDst != 0)
    {
        glEnable(GL_BLEND);
        glBlendFunc(pass.mBlendSrc, pass.mBlendDst);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    //  模板测试
    if (pass.mStencilOpFail != 0 && pass.mStencilOpZFail != 0 && pass.mStencilOpZPass != 0)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(pass.mStencilFunc, pass.mStencilRef, pass.mStencilMask);
        glStencilOp(pass.mStencilOpFail, pass.mStencilOpZFail, pass.mStencilOpZPass);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
    glUseProgram(pass.mID);
    _use = pass.mID;
}

void GLProgram::AddPass(const Pass & pass, const char * vString, uint vLength, const char * fString, uint fLength)
{
    _passs.push_back(pass);
    _passs.back().mID = glCreateProgram();

    if (vLength != 0)
    {
        uint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vString, (iint*)&vLength);
        glCompileShader(vs);
        AssertPass(vs, "Pass VShader Error.");
        glAttachShader(_passs.back().mID, vs);
        glDeleteShader(vs);
    }

    if (fLength != 0)
    {
        uint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fString, (iint*)&fLength);
        glCompileShader(fs);
        AssertPass(fs, "Pass FShader Error.");
        glAttachShader(_passs.back().mID, fs);
        glDeleteShader(fs);
    }

    iint ret;
    glLinkProgram(_passs.back().mID);
#ifdef ENABLE_DEBUG
    glGetProgramiv(_passs.back().mID, GL_LINK_STATUS, &ret);
    GLsizei size = 0;
    GLchar error[256] = { 0 };
    glGetProgramInfoLog(_passs.back().mID, sizeof(error), &size, error);
    ASSERT_LOG(ret != 0, "Pass GLProgram Error. {0}", error);
#endif
}

// ---
//  GLMaterial
// ---
GLMaterial::GLMaterial() : _mesh(nullptr), _program(nullptr)
{ }

bool GLMaterial::Init(const std::string & url)
{
    auto json = mmc::JsonValue::FromFile(url);
    ASSERT_LOG(json, "URL: {0}", url);
    if (json->IsHashKey("mesh"))
    {
        auto ptr = Global::Ref().mRawSys->Import(json->At("mesh")->ToString());
        SetMesh(CastPtr<GLMesh>(ptr));
    }
    if (json->IsHashKey("program"))
    {
        auto ptr = Global::Ref().mRawSys->Import(json->At("program")->ToString());
        SetProgram(CastPtr<GLProgram>(ptr));
    }
    if (json->IsHashKey("textures"))
    {
        for (auto value : json->At("textures"))
        {
            auto ptr = Global::Ref().mRawSys->Import(value.mValue->At("val")->ToString());
            SetTexture(value.mValue->At("key")->ToString(), CastPtr<GLTexture>(ptr));
        }
    }
    return true;
}

const SharePtr<GLMesh> & GLMaterial::GetMesh()
{
    return _mesh;
}

void GLMaterial::SetMesh(const SharePtr<GLMesh> & mesh)
{
    _mesh = mesh;
}

const SharePtr<GLProgram>& GLMaterial::GetProgram()
{
    return _program;
}

void GLMaterial::SetProgram(const SharePtr<GLProgram> & program)
{
    _program = program;
}

const std::vector<GLMaterial::Texture> & GLMaterial::GetTextures()
{
    return _textures;
}

void GLMaterial::SetTexture(const std::string & key, const SharePtr<GLTexture> & tex)
{
    auto it = std::find(_textures.begin(), _textures.end(), key);
    if (it != _textures.end()) { it->mKey = key;it->mTex = tex; }
    else { _textures.emplace_back(key, tex); }
}

