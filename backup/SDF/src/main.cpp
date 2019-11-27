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
        kChannelOut,    //  Í¨µÀ³¬³ö
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

    int GetCount() const
    {
        return mW * mH;
    }

    int GetIndex(int x, int y) const
    {
        return mW * y + x;
    }

    int GetValue(int x, int y, int def = 0) const
    {
        if (x < 0 || y < 0 || x >= mW || y >= mH)
        {
            return def;
        }
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

int GetDistance(const Image & image, int x, int y, int threshold)
{
    auto cur = image.GetValue(x, y);
    auto dx = image.mW * 2;
    auto dy = image.mH * 2;
    auto val = threshold;

    for (auto i = 1; i != threshold; ++i)
    {
        auto exit = true;
        auto px = x - i;
        auto py = y - i;
        for (auto i0 = 0; i0 != i * 2; ++i0)
        {
            if (GetDistanceSq(x, y, px, py) < GetDistanceSq(x, y, dx, dy))
            {
                if (auto v = image.GetValue(px, py, cur); v != cur)
                {
                    dx = px; dy = py; val = v;
                }
                exit = false;
            }
            ++px;
        }
        for (auto i1 = 0; i1 != i * 2; ++i1)
        {
            if (GetDistanceSq(x, y, px, py) < GetDistanceSq(x, y, dx, dy))
            {
                if (auto v = image.GetValue(px, py, cur); v != cur)
                {
                    dx = px; dy = py; val = v;
                }
                exit = false;
            }
            ++py;
        }
        for (auto i2 = 0; i2 != i * 2; ++i2)
        {
            if (GetDistanceSq(x, y, px, py) < GetDistanceSq(x, y, dx, dy))
            {
                if (auto v = image.GetValue(px, py, cur); v != cur)
                {
                    dx = px; dy = py; val = v;
                }
                exit = false;
            }
            --px;
        }
        for (auto i3 = 0; i3 != i * 2; ++i3)
        {
            if (GetDistanceSq(x, y, px, py) < GetDistanceSq(x, y, dx, dy))
            {
                if (auto v = image.GetValue(px, py, cur); v != cur)
                {
                    dx = px; dy = py; val = v;
                }
                exit = false;
            }
            --py;
        }
        if (exit) { break; }
    }

    if (val == threshold)
    {
        return -threshold;
    }
    else
    {
        return val == 0 
            ?  GetDistance(x, y, dx, dy) 
            : -GetDistance(x, y, dx, dy);
    }
}

char * GenSDF(const Image & image, int outW, int outH, int threshold)
{
    auto val = GetDistance(image, 53, 21, threshold);

    auto sdf = new int[image.GetCount()];
    for (int y = 0; y != image.mH; ++y)
    {
        for (int x = 0; x != image.mW; ++x)
        {
            sdf[image.GetIndex(x, y)] = GetDistance(image, x, y, threshold);
        }
        std::cout << "line: " << y << std::endl;
    }

    std::ofstream os("1.txt");
    for (int y = 0; y != image.mH; ++y)
    {
        for (int x = 0; x != image.mW; ++x)
        {
            auto val = sdf[image.GetIndex(x, y)];
            os << (val < 0 ? 0 : 1) << " ";
        }
        os << std::endl;
    }
    os.close();

    auto ret = new char[image.GetCount()];
    for (auto i = 0; i != image.GetCount(); ++i)
    {
        if (std::abs(sdf[i]) < threshold)
        {
            auto s = std::abs((float)sdf[i] / threshold);
            ret[i] = sdf[i] < 0
                ? (uchar)(127.0f * (1 - s))
                : (uchar)(127.0f * s + 128);
        }
        else
        {
            ret[i] = 0;
        }
    }
    delete[] sdf;

    return ret;
}

void GenSDF(const std::string & ifile, const std::string & ofile, int outW, int outH, int threshold)
{
    int w = 0, h = 0, fmt = 0;
    auto data = stbi_load(ifile.c_str(), &w, &h, &fmt, 0);
    if (data == nullptr) {throw Error(Error::kDataError);}

    auto sdf = GenSDF(Image(w, h, fmt, data), outW, outH, threshold);
    stbi_write_png(ofile.c_str(), outW, outH, 1, sdf, 0);
    stbi_image_free(data);
    delete[] sdf;
}

int main()
{
    try
    {
        std::string ifile = "1.png";
        std::string ofile = "1.sdf.png";
        int threshold = 48;
        int outW = 256;
        int outH = 256;
        GenSDF(ifile, ofile, outW, outH, threshold);
    }
    catch (const Error & error)
    {
        std::cout << error.What() << std::endl;
    }
    std::cin.get();
    return 0;
}