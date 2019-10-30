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
//  GLMesh
// ---
class GLMesh : public Raw {
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

    GLMesh();

    ~GLMesh();

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
    uint GetVAO() const { return _vao; }
    uint GetVBO() const { return _vbo; }
    uint GetEBO() const { return _ebo; }
    uint GetVCount() const { return _vCount; }
    uint GetECount() const { return _eCount; }

private:
    uint _vao, _vbo, _ebo;
    uint _vCount, _eCount;
};

// ---
//  GLImage
// ---
class GLImage: Raw {
public:
    GLImage();
    ~GLImage();
    void SetParam(int key, int val);
    virtual bool Init(const std::string & url) override;

public:
    uint mFormat, mID, mW, mH;
};

// ---
//  GLTexture
// ---
class GLTexture : public Raw {
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
    GLTexture();
    ~GLTexture();
    uint GetID() { return _refimg->mID; }
    uint GetImageW() { return _refimg->mW; }
    uint GetImageH() { return _refimg->mH; }
    uint GetImageFormat() { return _refimg->mFormat; }
    const glm::vec4 & GetOffset() { return  _offset; }
    uint GetW() { return (uint)(GetImageW() * (_offset.w - _offset.x)); }
    uint GetH() { return (uint)(GetImageH() * (_offset.z - _offset.y)); }

    virtual bool Init(const std::string & url) override;

private:
    bool InitFromImage(const std::string & url);
    bool InitFromAtlas(const std::string & url);

private:
    SharePtr<GLImage> _refimg;
    glm::vec4         _offset;
};


// ---
//  GLProgram
// ---
class GLProgram : public Raw {
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
            else if constexpr (std::is_same_v<T, GLTexture *>)
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
    GLProgram();
    ~GLProgram();

    virtual bool Init(const std::string & url) override;

    uint GetUse();
    uint GetPassCount();
    void UsePass(uint i);
    void AddPass(const Pass & pass, const char * vString, uint vLength, const char * fString, uint fLength);
    void BindUniformNumber(const char * const key, double val) const { glUniform1f(glGetUniformLocation(_use, key), static_cast<float>(val)); }
    void BindUniformVector(const char * const key, const glm::vec2 & val) const { glUniform2f(glGetUniformLocation(_use, key), val.x, val.y); }
    void BindUniformVector(const char * const key, const glm::vec3 & val) const { glUniform3f(glGetUniformLocation(_use, key), val.x, val.y, val.z); }
    void BindUniformVector(const char * const key, const glm::vec4 & val) const { glUniform4f(glGetUniformLocation(_use, key), val.x, val.y, val.z, val.w); }
    void BindUniformMatrix(const char * const key, const glm::mat3 & val) const { glUniformMatrix3fv(glGetUniformLocation(_use, key), 1, GL_FALSE, &val[0][0]); }
    void BindUniformMatrix(const char * const key, const glm::mat4 & val) const { glUniformMatrix4fv(glGetUniformLocation(_use, key), 1, GL_FALSE, &val[0][0]); }
    void BindUniformTex2D(const char * const key, const uint val, iint pos) const { glActiveTexture(GL_TEXTURE0 + pos); glBindTexture(GL_TEXTURE_2D,  val); glUniform1i(glGetUniformLocation(_use, key), pos); }

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
    uint              _use;
};

// ---
//  GLMaterial
// ---
class GLMaterial : public Raw {
public:
    struct Texture {
        std::string         mKey;
        SharePtr<GLTexture> mTex;

        Texture(
            const std::string & key = std::string(), 
            const SharePtr<GLTexture> & tex = nullptr)
            : mKey(key), mTex(tex)
        { }

        bool operator==(const std::string & key) const
        {
            return mKey == key;
        }
    };

public:
    GLMaterial();

    virtual bool Init(const std::string & url) override;

    const SharePtr<GLMesh> & GetMesh();
    void SetMesh(const SharePtr<GLMesh> & mesh);

    const SharePtr<GLProgram> & GetProgram();
    void SetProgram(const SharePtr<GLProgram> & program);

    const std::vector<Texture> & GetTextures();
    void SetTexture(const std::string & key, const SharePtr<GLTexture> & tex);

private:
    SharePtr<GLMesh>      _mesh;
    SharePtr<GLProgram>   _program;
    std::vector<Texture>  _textures;
};