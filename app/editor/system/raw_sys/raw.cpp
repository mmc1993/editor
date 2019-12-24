#include "raw.h"
#include "raw_sys.h"

// ---
//  RawMesh
// ---
RawMesh::RawMesh() : mVAO(0), mVBO(0), mEBO(0), mVCount(0), mECount(0)
{ }

RawMesh::~RawMesh()
{
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mEBO);
    glDeleteVertexArrays(1, &mVAO);
}

bool RawMesh::Init(const std::string & url)
{
    //  _TODO
    //  暂时用不上
    return true;
}

void RawMesh::Init(const std::vector<Vertex> & points, const std::vector<uint> & indexs, uint enabled, uint vUsage, uint eUsage)
{
    Init(points.data(), (uint)points.size(), indexs.data(), (uint)indexs.size(), enabled, vUsage, eUsage);
}

void RawMesh::Init(const Vertex * points, const uint pointsLength, const uint * indexs, const uint indexsLength, uint enabled, uint vUsage, uint eUsage)
{
    mVCount = pointsLength;
    mECount = indexsLength;

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mVCount, points, vUsage);

    glGenBuffers(1, &mEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mECount, indexs, eUsage);

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

void RawMesh::Update(const std::vector<Vertex> & points, const std::vector<uint> & indexs, uint vUsage, uint eUsage)
{
    mVCount = points.size();
    mECount = indexs.size();
    glBindBuffer(GL_ARRAY_BUFFER,         mVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ARRAY_BUFFER,         sizeof(Vertex) * points.size(), points.data(), vUsage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)   * indexs.size(), indexs.data(), eUsage);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RawMesh::Draw(uint primitive)
{
    glBindVertexArray(GetVAO());
    if (GetECount() == 0)
    {
        glDrawArrays(primitive, 0, GetVCount());
    }
    else
    {
        glDrawElements(primitive, GetECount(), GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
}

// ---
//  RawImage
// ---
RawImage::RawImage()
    : mFormat(0)
    , mID(0)
    , mW(0)
    , mH(0)
{ }

RawImage::~RawImage()
{
    glDeleteTextures(1, &mID);
}

void RawImage::ModifyWH(uint w, uint h)
{
    mW = w; mH = h;
    glBindTexture(GL_TEXTURE_2D, mID);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RawImage::InitNull(uint fmt)
{
    mFormat          = fmt;
    glGenTextures(1, &mID);
    SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool RawImage::Init(const std::string & url)
{
    stbi_set_flip_vertically_on_load(true);

    int w, h, format;
    if (auto data = stbi_load(url.c_str(), &w, &h, &format, 0))
    {
        mW = (uint)w;
        mH = (uint)h;
        switch (format)
        {
        case 1: mFormat = GL_RED; break;
        case 3: mFormat = GL_RGB; break;
        case 4: mFormat = GL_RGBA; break;
        }
        glGenTextures(1, &mID);
        glBindTexture(GL_TEXTURE_2D, mID);
        glTexImage2D( GL_TEXTURE_2D, 0, mFormat, mW, mH, 0, mFormat, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);

        SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        stbi_image_free(data);
    }
    return mID != 0;
}

void RawImage::SetParam(int key, int val)
{
    glBindTexture(GL_TEXTURE_2D, mID);
    glTexParameteri(GL_TEXTURE_2D, key, val);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// ---
//  RawTexture
// ---
RawTexture::RawTexture() : mOffset(0.0f)
{ }

RawTexture::~RawTexture()
{ }

bool RawTexture::Init(const std::string & url)
{
    auto ret = InitFromImage(url) || InitFromAtlas(url);
    ASSERT_LOG(ret, url.c_str());
    return ret;
}

bool RawTexture::InitFromImage(const std::string & url)
{
    if (!tools::IsFileExists(url)) { return false; }
    mRefImg = std::create_ptr<RawImage>();
    if (mRefImg->Init(url))
    {
        mOffset.x = 0.0f;
        mOffset.y = 0.0f;
        mOffset.z = 1.0f;
        mOffset.w = 1.0f;
        return true;
    }
    return false;
}

bool RawTexture::InitFromAtlas(const std::string & url)
{
    ASSERT_LOG(tools::GetFileSuffix(url) == ".png", url.c_str());

    std::string         path;
    mmc::Json::Pointer atlas;
    auto folder  = tools::GetFileFolder(url);
    auto dirname = tools::GetFolderName(url);
    path.append(folder);
    path.append(dirname);
    auto name = path + ".atlas";
    for (auto i = 0; tools::IsFileExists(name); ++i)
    {
        auto json = mmc::Json::FromFile(name);
        if (json == nullptr) { break; }
        if (json->HasKey("frames",url))
        { atlas = json; break; }
        name = SFormat("{0}{1}.atlas", path, i);
    }

    if (atlas != nullptr) 
    {
        name = path + atlas->At("meta","image")->ToString();
        mRefImg = Global::Ref().mRawSys->Get<RawImage>(name);
        mOffset.x = atlas->At("frames", url, "frame", "x")->ToNumber() / mRefImg->mW;
        mOffset.y = atlas->At("frames", url, "frame", "y")->ToNumber() / mRefImg->mH;
        mOffset.z = atlas->At("frames", url, "frame", "w")->ToNumber() / mRefImg->mW + mOffset.x;
        mOffset.w = atlas->At("frames", url, "frame", "h")->ToNumber() / mRefImg->mH + mOffset.y;
    }

    return mRefImg != nullptr;
}

// ---
//  RawMap
// ---
RawMap::RawMap()
{ }

RawMap::~RawMap()
{ }

bool RawMap::Init(const std::string & url)
{
    auto tmx = mmc::Json::FromFile(url);
    ASSERT_LOG(tmx != nullptr, url.c_str());

    auto folder = tools::GetFileFolder(url);
    for (auto & value : tmx->At("tilesets"))
    {
        auto atlasURL = folder + value.mVal->At("source")->ToString();
        auto baseIndex = (uint)value.mVal->At("firstgid")->ToNumber();
        auto atlasJson = mmc::Json::FromFile(atlasURL);
        ASSERT_LOG(atlasJson != nullptr, atlasURL.c_str());

        Atlas atlas;
        auto image = tools::GetFileFolder(atlasURL) + atlasJson->At("image")->ToString();
        atlas.mTexture  = Global::Ref().mRawSys->Get<RawTexture>(image);
        atlas.mOffset   = (uint)atlasJson->At("margin"      )->ToNumber();
        atlas.mSpace    = (uint)atlasJson->At("spacing"     )->ToNumber();
        atlas.mCol      = (uint)atlasJson->At("columns"     )->ToNumber();
        atlas.mRow      = (uint)atlasJson->At("tilecount"   )->ToNumber() / atlas.mCol;
        atlas.mBase  = baseIndex;
        mAtlass.push_back(atlas);
    }

    mMap.mSizeW = (uint)tmx->At("width"     )->ToNumber();
    mMap.mSizeH = (uint)tmx->At("height"    )->ToNumber();
    mMap.mTileW = (uint)tmx->At("tilewidth" )->ToNumber();
    mMap.mTileH = (uint)tmx->At("tileheight")->ToNumber();
    mMap.mPixelW = mMap.mSizeW * mMap.mTileW;
    mMap.mPixelH = mMap.mSizeH * mMap.mTileH;
    for (auto & layer : tmx->At("layers"))
    {
        UpdateVertexs(layer.mVal->At("data"));
    }
    return true;
}

void RawMap::UpdateVertexs(const mmc::Json::Pointer & data)
{ 
    for (auto i = 0; i != data->GetCount(); ++i)
    {
        if (auto index = (uint)data->At(i)->ToNumber(); index!= 0)
        {
            auto [uv, atlasPos] = GetTileInfo(index);

            glm::vec4 quad;
            quad.x =                                     (float)(i % mMap.mSizeW * mMap.mTileW);
            quad.y = (mMap.mSizeH  - 1) *  mMap.mTileH - (float)(i / mMap.mSizeH * mMap.mTileH);
            quad.z = quad.x + mMap.mTileW + 0.5f;
            quad.w = quad.y + mMap.mTileH + 0.5f;

            mPoints.emplace_back(glm::vec2(quad.x, quad.y), glm::vec4((float)atlasPos), glm::vec2(uv.x, uv.y));
            mPoints.emplace_back(glm::vec2(quad.z, quad.y), glm::vec4((float)atlasPos), glm::vec2(uv.z, uv.y));
            mPoints.emplace_back(glm::vec2(quad.z, quad.w), glm::vec4((float)atlasPos), glm::vec2(uv.z, uv.w));
            mPoints.emplace_back(glm::vec2(quad.x, quad.y), glm::vec4((float)atlasPos), glm::vec2(uv.x, uv.y));
            mPoints.emplace_back(glm::vec2(quad.z, quad.w), glm::vec4((float)atlasPos), glm::vec2(uv.z, uv.w));
            mPoints.emplace_back(glm::vec2(quad.x, quad.w), glm::vec4((float)atlasPos), glm::vec2(uv.x, uv.w));
        }
    }
}

std::tuple<glm::vec4, uint> RawMap::GetTileInfo(uint idx)
{
    uint atlasPos = 0;
    for (auto i = 0; mAtlass.size() != i; atlasPos = i++)
    {
        if (idx < mAtlass.at(i).mBase) { break; }
    }
    glm::vec4 quad;
    auto & atlas = mAtlass.at(atlasPos);
    auto x =                  (idx - atlas.mBase) % atlas.mCol;
    auto y = atlas.mRow - 1 - (idx - atlas.mBase) / atlas.mCol;
    quad.x = (float)(x * mMap.mTileW + x * atlas.mSpace + atlas.mOffset) / atlas.mTexture->GetW();
    quad.y = (float)(y * mMap.mTileH + y * atlas.mSpace + atlas.mOffset) / atlas.mTexture->GetH();
    quad.z = quad.x + (float)mMap.mTileW / atlas.mTexture->GetW();
    quad.w = quad.y + (float)mMap.mTileH / atlas.mTexture->GetH();
    return { quad, atlasPos, };
}

// ---
//  RawFont
// ---
RawFont::RawFont()
{ }

RawFont::~RawFont()
{ }

const SharePtr<RawTexture> & RawFont::RefTexture()
{
    return mTexture;
}

const RawFont::Char & RawFont::RefWord(char word)
{
    return RefWord((uint)word);
}

const RawFont::Char & RawFont::RefWord(uint code)
{
    return mInfo.mChars.at(code);
}

std::vector<uint> RawFont::RefWord(const std::string & text)
{
    std::vector<uint> result;
    for (auto & word : text)
    {
        result.emplace_back(RefWord(word).mID);
    }
    return std::move(result);
}

bool RawFont::Init(const std::string & url)
{
    std::ifstream is(url);
    std::string line;
    std::string word;

    is >> word;
    ASSERT_LOG(word == "info", line.c_str());
    std::getline(is, line);
    line = std::lstrip(line, ' ');
    Parse(tools::Split(line, " "), &mInfo.mInfo);

    is >> word;
    ASSERT_LOG(word == "common", line.c_str());
    std::getline(is, line);
    line = std::lstrip(line, ' ');
    Parse(tools::Split(line, " "), &mInfo.mCommon);

    is >> word;
    ASSERT_LOG(word == "page", line.c_str());
    std::getline(is, line);
    line = std::lstrip(line, ' ');
    Parse(tools::Split(line, " "), &mInfo.mPage);

    auto texurl = tools::GetFileFolder(url) + mInfo.mPage.at("file");
    mTexture = Global::Ref().mRawSys->Get<RawTexture>(texurl);
    mTexture->GetImage()->SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    mTexture->GetImage()->SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::getline(is, line);
    auto texW = std::stof(mInfo.mCommon.at("scaleW"));
    auto texH = std::stof(mInfo.mCommon.at("scaleH"));
    while (is >> word && word == "char")
    {
        //  id=:3, x=:3, y=:3, width=:6, height=:7, xoffset=:7, yoffset=:7, xadvance=:8
        Char value;
        is >> word; value.mID = std::stoi(word.substr(3));
        is >> word; value.mUV.x = std::stof(word.substr(2)) / texW;
        is >> word; value.mUV.y = std::stof(word.substr(2)) / texH;
        is >> word; value.mUV.z = std::stof(word.substr(6)) / texW + value.mUV.x;
        is >> word; value.mUV.w = std::stof(word.substr(7)) / texH + value.mUV.y;

        //  翻转UV
        std::swap(value.mUV.y, value.mUV.w);
        value.mUV.y = 1.0f - value.mUV.y;
        value.mUV.w = 1.0f - value.mUV.w;

        mInfo.mChars.emplace(value.mID, value);

        std::getline(is, line);
    }
    is.close();
    return true;
}

void RawFont::Parse(const std::vector<std::string> & pairs, std::map<std::string, std::string> * output)
{
    for (const auto & pair : pairs)
    {
        if (auto ret = tools::Split(pair, "="); ret.at(1).front() != '\"' &&
            ret.at(1).back()  != '\"')
        {
            output->emplace(ret.at(0), ret.at(1));
        }
        else
        {
            output->emplace(ret.at(0), std::string(ret.at(1).begin() + 1, ret.at(1).end() - 1));
        }
    }
}

// ---
//  RawProgram
// ---
RawProgram::RawProgram() : mUse(GL_INVALID_INDEX)
{ }

RawProgram::~RawProgram()
{
    for (auto & pass : _passs)
    {
        glDeleteProgram(pass.mID);
    }
}

bool RawProgram::Init(const std::string & url)
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
        RawProgram::Pass * pass)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (!tools::IsPrint(line)) { continue; }

            if (tools::IsEqualSkipSpace(line, endFlag))
            {
                break;
            }

            if (tools::IsEqualSkipSpace(line, "BlendFunc") || 
                tools::IsEqualSkipSpace(line, "StencilTest"))
            {
                ASSERT_LOG(pass != nullptr, "解析Pass属性错误: {0}, {1}", endFlag, line);
                std::stringstream ss;
                std::string word;
                ss.str(line);
                ss >> word;

                if (word == "StencilTest")
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

uint RawProgram::GetUse()
{
    return mUse;
}

uint RawProgram::GetPassCount()
{
    return (uint)_passs.size();
}

void RawProgram::UsePass(uint i)
{
    const auto & pass = _passs.at(i);
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
    mUse = pass.mID;
}

void RawProgram::AddPass(const Pass & pass, const char * vString, uint vLength, const char * fString, uint fLength)
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
    ASSERT_LOG(ret != 0, "Pass Error. {0}", error);
#endif
}

// ---
//  RawMaterial
// ---
RawMaterial::RawMaterial() : mMesh(nullptr), mProgram(nullptr)
{ }

bool RawMaterial::Init(const std::string & url)
{
    auto json = mmc::Json::FromFile(url);
    ASSERT_LOG(json, "URL: {0}", url);
    if (json->HasKey("mesh"))
    {
        SetMesh(Global::Ref().mRawSys->Get<RawMesh>(json->At("mesh")->ToString()));
    }
    if (json->HasKey("program"))
    {
        SetProgram(Global::Ref().mRawSys->Get<RawProgram>(json->At("program")->ToString()));
    }
    if (json->HasKey("textures"))
    {
        for (auto value : json->At("textures"))
        {
            SetTexture(value.mVal->At("key")->ToString(), Global::Ref().mRawSys->Get<RawTexture>(value.mVal->At("val")->ToString()));
        }
    }
    return true;
}

const SharePtr<RawMesh> & RawMaterial::GetMesh()
{
    return mMesh;
}

void RawMaterial::SetMesh(const SharePtr<RawMesh> & mesh)
{
    mMesh = mesh;
}

const SharePtr<RawProgram>& RawMaterial::GetProgram()
{
    return mProgram;
}

void RawMaterial::SetProgram(const SharePtr<RawProgram> & program)
{
    mProgram = program;
}

const std::vector<RawMaterial::Texture> & RawMaterial::GetTextures()
{
    return mTextures;
}

void RawMaterial::SetTexture(const std::string & key, const SharePtr<RawTexture> & tex)
{
    auto it = std::find(mTextures.begin(), mTextures.end(), key);
    if (it != mTextures.end()) { it->mKey = key;it->mTex = tex; }
    else { mTextures.emplace_back(key, tex); }
}

