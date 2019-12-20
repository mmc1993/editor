#pragma once

#include "../../include.h"

class Raw {
public:
    virtual ~Raw()
    { }

    virtual bool Init(const std::string & url) = 0;

protected:
    Raw()
    { }
};

// ---
//  RawMesh
// ---
class RawMesh : public Raw {
public:
    struct Vertex {
        glm::vec2 v;
        glm::vec4 c;
        glm::vec2 uv;

        enum EnableEnum {
            kV  = 0x1,
            kC  = 0x2,
            kUV = 0x4,
        };

        Vertex()
        { }

        Vertex(const glm::vec2 & _v) : v(_v)
        { }

        Vertex(const glm::vec2 & _v, const glm::vec4 & _c) : v(_v), c(_c)
        { }

        Vertex(const glm::vec2 & _v, const glm::vec2 & _uv) : v(_v), uv(_uv)
        { }

        Vertex(const glm::vec2 & _v, const glm::vec4 & _c, const glm::vec2 & _uv) : v(_v), c(_c), uv(_uv)
        { }

        static uint SizeOf(uint enabled)
        {
            uint size = 0;
            if (kV & enabled) { size += sizeof(Vertex::v); }
            if (kC & enabled) { size += sizeof(Vertex::c); }
            if (kUV & enabled) { size += sizeof(Vertex::uv); }
            return size;
        }
    };

    RawMesh();

    ~RawMesh();

    virtual bool Init(const std::string & url) override;

    //  完全填充
    void Init(
        const std::vector<Vertex> & points, 
        const std::vector<uint> &   indexs, 
        uint enabled, uint vUsage = GL_STATIC_DRAW, uint eUsage = GL_STATIC_DRAW);

    void Init(
        const Vertex * points, const uint pointsLength, 
        const uint *   indexs, const uint indexsLength, 
        uint enabled, uint vUsage = GL_STATIC_DRAW, uint eUsage = GL_STATIC_DRAW);

    void Update(
        const std::vector<Vertex> & points, 
        const std::vector<uint>   & indexs,
        uint vUsage = GL_STATIC_DRAW, 
        uint eUsage = GL_STATIC_DRAW);

    void Draw(uint primitive);
    uint GetVAO() const { return mVAO; }
    uint GetVBO() const { return mVBO; }
    uint GetEBO() const { return mEBO; }
    uint GetVCount() const { return mVCount; }
    uint GetECount() const { return mECount; }

private:
    uint mVAO, mVBO, mEBO;
    uint mVCount, mECount;
};

// ---
//  RawImage
// ---
class RawImage : public Raw {
public:
    RawImage();
    ~RawImage();
    void SetParam(int key, int val);
    void ModifyWH(uint w, uint h);
    void InitNull(uint fmt);
    virtual bool Init(const std::string & url) override;

public:
    uint mFormat, mID, mW, mH;
};

// ---
//  RawTexture
// ---
class RawTexture : public Raw {
public:
    struct Filter {
        uint mMin;
        uint mMag;
        uint mWrapS;
        uint mWrapT;
        Filter()
            : mMin(GL_LINEAR)
            , mMag(GL_LINEAR)
            , mWrapS(GL_CLAMP_TO_EDGE)
            , mWrapT(GL_CLAMP_TO_EDGE)
        { }
    };

public:
    RawTexture();
    ~RawTexture();
    uint GetID() { return mRefImg->mID; }
    uint GetImageW() { return mRefImg->mW; }
    uint GetImageH() { return mRefImg->mH; }
    uint GetImageFormat() { return mRefImg->mFormat; }
    const glm::vec4 & GetOffset() { return  _offset; }
    SharePtr<RawImage> & GetRefImage() { return mRefImg; }
    uint GetW() { return (uint)(GetImageW() * (_offset.w - _offset.x)); }
    uint GetH() { return (uint)(GetImageH() * (_offset.z - _offset.y)); }

    virtual bool Init(const std::string & url) override;

private:
    bool InitFromImage(const std::string & url);
    bool InitFromAtlas(const std::string & url);

private:
    SharePtr<RawImage> mRefImg;
    glm::vec4         _offset;
};

// ---
//  RawMap
// ---
class RawMap : public Raw {
public:
    struct Map {
        uint mPixelW;
        uint mPixelH;
        uint mSizeW;
        uint mSizeH;
        uint mTileW;
        uint mTileH;
        uint mRow;
        uint mCol;
    };

    struct Atlas {
        uint mBase;
        uint mSpace;
        uint mOffset;
        uint mRow, mCol;
        SharePtr<RawTexture> mTexture;
    };

public:
    RawMap();
    ~RawMap();
    virtual bool Init(const std::string & url) override;
    const Map &                          GetMap() { return mMap; }
    const std::vector<Atlas> &           GetAtlass() { return mAtlass; }
    const std::vector<RawMesh::Vertex> & GetPoints() { return mPoints; }

private:
    void UpdateVertexs(const mmc::Json::Pointer & data);
    std::tuple<glm::vec4, uint>   GetTileInfo(uint idx);

private:
    Map                          mMap;
    std::vector<Atlas>           mAtlass;
    std::vector<RawMesh::Vertex> mPoints;
};

// ---
//  RawFont
// ---
class RawFont : public Raw {
public:
    struct Char {
        uint      mID; //  字Unicode编码
        glm::vec4 mUV;
    };

    //  字体信息
    struct FontInfo {
        std::map<uint, Char> mChars;
        std::map<std::string, std::string> mInfo;
        std::map<std::string, std::string> mPage;
        std::map<std::string, std::string> mCommon;
    };

public:
    RawFont();
    ~RawFont();
    const SharePtr<RawTexture> & RefTexture();
    const Char & RefWord(char word);
    const Char & RefWord(uint code);
    //const Word & RefWord(const char * word, uint count);
    std::vector<uint> RefWord(const std::string & text);
    virtual bool Init(const std::string & url) override;

private:
    void Parse(const std::vector<std::string> & pairs, std::map<std::string, std::string> * output);

private:
    FontInfo            _info;
    SharePtr<RawTexture> mTexture;
};

// ---
//  RawProgram
// ---
class RawProgram : public Raw {
public:
    struct Value {
        enum TypeEnum {
            kNumber,
            kTexture,
            kMatrix3,
            kMatrix4,
            kVector2,
            kVector3,
            kVector4,
        };

        bool operator=(Value && value)
        {
            mKey = std::move(value.mKey);
            mVal = std::move(value.mVal);
        }

        bool operator=(const Value & value)
        {
            mKey = value.mKey;
            mVal = value.mVal;
        }

        template <class T>
        Value(const std::string & key, const T & val) : mKey(key), mVal(val)
        {
            if constexpr (std::is_arithmetic_v<T>)
                mType = kNumber;
            else if constexpr (std::is_same_v<T, RawTexture *>)
                mType = kTexture;
            else if constexpr (std::is_same_v<T, glm::mat3>)
                mType = kMatrix3;
            else if constexpr (std::is_same_v<T, glm::mat4>)
                mType = kMatrix4;
            else if constexpr (std::is_same_v<T, glm::vec2>)
                mType = kVector2;
            else if constexpr (std::is_same_v<T, glm::vec3>)
                mType = kVector3;
            else if constexpr (std::is_same_v<T, glm::vec4>)
                mType = kVector4;
        }

        std::string mKey;
        std::any    mVal;
        TypeEnum    mType;
    };

public:
    struct Pass {
        iint        mBlendSrc;          //  混合
        iint        mBlendDst;          //  混合
        iint        mStencilOpFail;	    //	模板测试失败
        iint        mStencilOpZFail;	//	深度测试失败
        iint        mStencilOpZPass;	//	深度测试通过
        iint		mStencilFunc;	    //	模板测试函数
        iint        mStencilMask;       //  模板测试值
        iint        mStencilRef;        //  模板测试值
        uint        mID;                //  ID

        Pass() { memset(this, 0, sizeof(Pass)); }
    };
public:
    RawProgram();
    ~RawProgram();

    virtual bool Init(const std::string & url) override;

    uint GetUse();
    uint GetPassCount();
    void UsePass(uint i);
    void AddPass(const Pass & pass, const char * vString, uint vLength, const char * fString, uint fLength);
    void BindUniformNumber(const char * const key, double val) const { glUniform1f(glGetUniformLocation(mUse, key), static_cast<float>(val)); }
    void BindUniformVector(const char * const key, const glm::vec2 & val) const { glUniform2f(glGetUniformLocation(mUse, key), val.x, val.y); }
    void BindUniformVector(const char * const key, const glm::vec3 & val) const { glUniform3f(glGetUniformLocation(mUse, key), val.x, val.y, val.z); }
    void BindUniformVector(const char * const key, const glm::vec4 & val) const { glUniform4f(glGetUniformLocation(mUse, key), val.x, val.y, val.z, val.w); }
    void BindUniformMatrix(const char * const key, const glm::mat3 & val) const { glUniformMatrix3fv(glGetUniformLocation(mUse, key), 1, GL_FALSE, &val[0][0]); }
    void BindUniformMatrix(const char * const key, const glm::mat4 & val) const { glUniformMatrix4fv(glGetUniformLocation(mUse, key), 1, GL_FALSE, &val[0][0]); }
    void BindUniformTex2D(const char * const key, const uint val, iint pos) const { glActiveTexture(GL_TEXTURE0 + pos); glBindTexture(GL_TEXTURE_2D,  val); glUniform1i(glGetUniformLocation(mUse, key), pos); }

private:
    void AssertPass(uint shaderID, const std::string & errorTxt) const
    {
        iint ret;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &ret);
        if (ret == 0)
        {
            char err[256] = { 0 };
            glGetShaderInfoLog(shaderID, sizeof(err), nullptr, err);
            ASSERT_LOG(false, "Shader Error. {0}, Code: {1}, Text: {2}", errorTxt, ret, err);
        }
    }

private:
    std::vector<Pass> _passs;
    uint              mUse;
};

// ---
//  RawMaterial
// ---
class RawMaterial : public Raw {
public:
    struct Texture {
        std::string          mKey;
        SharePtr<RawTexture> mTex;

        Texture(
            const std::string & key = std::string(), 
            const SharePtr<RawTexture> & tex = nullptr)
            : mKey(key), mTex(tex)
        { }

        bool operator==(const std::string & key) const
        {
            return mKey == key;
        }
    };

public:
    RawMaterial();

    virtual bool Init(const std::string & url) override;

    const SharePtr<RawMesh> & GetMesh();
    void SetMesh(const SharePtr<RawMesh> & mesh);

    const SharePtr<RawProgram> & GetProgram();
    void SetProgram(const SharePtr<RawProgram> & program);

    const std::vector<Texture> & GetTextures();
    void SetTexture(const std::string & key, const SharePtr<RawTexture> & tex);

private:
    SharePtr<RawMesh>    mMesh;
    SharePtr<RawProgram> mProgram;
    std::vector<Texture> mTextures;
};