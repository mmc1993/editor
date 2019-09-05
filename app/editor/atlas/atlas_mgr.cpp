#include "atlas_mgr.h"
#include "../global.h"
#include "../raw/raw_mgr.h"

bool AtlasMgr::Import(const std::string & url)
{
    if (auto suffix = string_tool::QueryFileSuffix(url); suffix == ".png" || suffix == ".jpg")
    {
        return ImportImage(url);
    }
    else if (suffix == ".plist")
    {
        return ImportAtlas(url);
    }
    return false;
}

bool AtlasMgr::ImportImage(const std::string & url)
{
    auto it = _resources.find(url);
    CHECK_RET(it != _resources.end(), true);

    auto raw = (RawBitmap *)Global::Ref().mRawMgr->Import(url);
    ASSERT_LOG(raw != nullptr, url.c_str());
    CHECK_RET(raw != nullptr, false);

    Image image;
    image.mKey = url;
    image.mQuat.x = 0;
    image.mQuat.y = 0;
    image.mQuat.z = raw->GetW();
    image.mQuat.w = raw->GetH();
    _resources.insert(std::make_pair(image.mKey, image));
    return true;
}

bool AtlasMgr::ImportAtlas(const std::string & url)
{
    //  MMC TODO
    return false;
}
