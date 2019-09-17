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

class RawBitmap : public Raw {
public:
    struct Sample {
        uint32_t minfilter;
        uint32_t magfilter;
        uint32_t wraps;
        uint32_t wrapt;
        Sample()
            : minfilter(GL_LINEAR)
            , magfilter(GL_LINEAR)
            , wraps(GL_CLAMP_TO_EDGE)
            , wrapt(GL_CLAMP_TO_EDGE)
        { }
    };

public:
    RawBitmap();
    ~RawBitmap();
    std::uint32_t GetW() const { return _w; }
    std::uint32_t GetH() const { return _h; }
    std::uint32_t GetID() const { return _id; }
    std::uint32_t GetFormat() const { return _fmt; }
    void SetParam(int key, int val) const
    {
        glBindTexture(  GL_TEXTURE_2D, _id);
        glTexParameteri(GL_TEXTURE_2D, key, val);
        glBindTexture(  GL_TEXTURE_2D, 0);
    }

    virtual bool Init(const std::string & url) override;

public:
    std::uint32_t _fmt, _id, _w, _h;
};

