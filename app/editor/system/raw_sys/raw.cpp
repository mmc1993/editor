#include "raw.h"

RawBitmap::RawBitmap(): _fmt(0), _id(0), _w(0), _h(0)
{ }

RawBitmap::~RawBitmap()
{
    glDeleteTextures(1, &_id);
}

bool RawBitmap::Init(const std::string & url)
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
    ASSERT_LOG(_id == 0, "");
    glGenTextures(1, &_id);
    ASSERT_LOG(_id != 0, "");
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
