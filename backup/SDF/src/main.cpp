#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "stb_image_resize.h"
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

using uchar = unsigned char;

class Error {
public:
    enum CodeEnum {
        kChannelOut,    //  通道超出
        kDataError,
    };

public:
    Error(CodeEnum code): _mCode(code)
    { }

    CodeEnum What() const { return _mCode; }

private:
    CodeEnum _mCode;
};

struct Image {
    int mW;
    int mH;
    int mFmt;
    uchar * mData;

    Image(int w, int h, int fmt, uchar * data)
        : mW(w), mH(h), mFmt(fmt), mData(data)
    { }

    int GetValue(int i) const
    {
        return mData[i * mFmt];
    }

    int GetPixelSize() const
    {
        return mW * mH;
    }

    int GetByteSize() const
    {
        return GetPixelSize() * mFmt;
    }

    int GetIndex(int x, int y) const
    {
        return mW * y + x;
    }

    int GetValue(int x, int y) const
    {
        return (int)(mData[GetIndex(x, y) * mFmt] != 0);
    }
};

int GetDistanceSq(int x0, int y0, int x1, int y1)
{
    auto v0 = x1 - x0;
    auto v1 = y1 - y0;
    return v0 * v0 + v1 * v1;
}

int GetDistance(int x0, int y0, int x1, int y1)
{
    return (int)std::sqrt(GetDistanceSq(x0, y0, x1, y1));
}

int GetDistance(const Image & image, int x, int y)
{
    auto cur = image.GetValue(x, y);
    int dx0 = image.mW * 2, dy0 = image.mH * 2, val0 = 0;
    int dx1 = image.mW * 2, dy1 = image.mH * 2, val1 = 0;

    //  上方
    for (auto y0 = y; y0 != -1; --y0)
    {
        auto isGoto = true;
        for (auto x0 = 0; x0 != image.mW; ++x0)
        {
            if (GetDistanceSq(x, y, x0, y0) < GetDistanceSq(x, y, dx0, dy0))
            {
                if (auto val = image.GetValue(x0, y0); val != cur)
                { 
                    dx0 = x0; dy0 = y0; val0 = val;
                }
                isGoto = false;
            }
        }
        if (isGoto) goto exit_for_u;
    }
exit_for_u:

    //  下方
    for (auto y0 = y + 1; y0 != image.mH; ++y0)
    {
        auto isGoto = true;
        for (auto x0 = 0; x0 != image.mW; ++x0)
        {
            if (GetDistanceSq(x, y, x0, y0) < GetDistanceSq(x, y, dx1, dy1))
            {
                if (auto val = image.GetValue(x0, y0); val != cur)
                {
                    dx1 = x0; dy1 = y0; val1 = val;
                }
                isGoto = false;
            }
        }
        if (isGoto) goto exit_for_d;
    }
exit_for_d:

    int dx = 0, dy = 0, val = 0;
    if (GetDistanceSq(x, y, dx0, dy0) < GetDistanceSq(x, y, dx1, dy1))
    {
        dx = dx0; dy = dy0; val = val0;
    }
    else
    {
        dx = dx1; dy = dy1; val = val1;
    }

    return val == 0 
        ?  GetDistance(x, y, dx, dy) 
        : -GetDistance(x, y, dx, dy);
}

char * GenSDF(const Image & image, int oW, int oH)
{
    auto val = GetDistance(image, 215, 105);

    auto max = 0;
    auto sdf = new int[image.GetPixelSize()];
    for (int y = 0; y != image.mH; ++y)
    {
        for (int x = 0; x != image.mW; ++x)
        {
            auto val = GetDistance(image, x, y);
            sdf[image.GetIndex(x, y)] = val;
            if (std::abs(val) > max)
            {
                max = std::abs(val);
            }
        }
        std::cout << "line: " << y << std::endl;
    }

    std::cout << "max: " << max << std::endl;

    auto ret = new char[image.GetPixelSize()];
    for (auto i = 0; i != image.GetPixelSize(); ++i)
    {
        if (std::abs(sdf[i]) > 50)
        {
            ret[i] = 0;
        }
        else
        {
            auto s = std::abs((float)sdf[i] / (float)50);
            ret[i] = sdf[i] < 0
                ? (uchar)(127.0f * (1 - s))
                : (uchar)(127.0f * s + 128);
        }
    }
    return ret;
}

void GenSDF(const std::string & ifile, const std::string & ofile, int oW, int oH)
{
    int w = 0, h = 0, fmt = 0;
    auto data = stbi_load(ifile.c_str(), &w, &h, &fmt, 0);

    if (data == nullptr)
    {
        stbi_image_free(data);
        throw Error(Error::kDataError);
    }

    auto sdf = GenSDF(Image(w, h, fmt, (uchar *)data), oW, oH);
    stbi_write_png(ofile.c_str(), oW, oH, 1, sdf, 0);
    stbi_image_free(data);
    delete[] sdf;
}

int main()
{
    try
    {
        std::string ifile = "1.png";
        std::string ofile = "1.sdf.png";
        int oW = 256;
        int oH = 256;
        GenSDF(ifile, ofile, oW, oH);
    }
    catch (const Error & error)
    {
        std::cout << error.What() << std::endl;
    }
    std::cin.get();
    return 0;
}