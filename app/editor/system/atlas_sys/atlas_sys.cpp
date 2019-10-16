#include "atlas_sys.h"
#include "../raw_sys/raw_sys.h"

void AtlasSys::Delete(const std::string & url)
{
    auto suffix = tools::GetFileSuffix(url);
    if (suffix == ".png" || suffix == ".jpg")
    {
        DeleteImage(url);
    }
    else if (suffix == ".json")
    {
        DeleteAtlas(url);
    }
}

bool AtlasSys::Import(const std::string & url)
{
    auto suffix = tools::GetFileSuffix(url);
    if (suffix == ".png" || suffix == ".jpg")
    {
        return ImportImage(url);
    }
    else if (suffix == ".json")
    {
        return ImportAtlas(url);
    }
    return false;
}

bool AtlasSys::IsHasKey(const std::string & key) const
{
    auto it = _resources.find(key);
    return it != _resources.end();
}

const AtlasSys::Image & AtlasSys::Get(const std::string & key) const
{
    auto it = _resources.find(key);
    ASSERT_LOG(it != _resources.end(), "{0}", key);
    return it->second;
}

bool AtlasSys::ImportImage(const std::string & url)
{
    if (!IsHasKey(url))
    {
        auto ptr = Global::Ref().mRawSys->Import(url);
        ASSERT_LOG(ptr != nullptr, url.c_str());
        auto raw = CastPtr<GLTexture>(ptr);

        Image image;
        image.mKey = url;
        image.mID = raw->GetID();
        image.mQuat.x = 0;
        image.mQuat.y = 0;
        image.mQuat.z = (float)raw->GetW();
        image.mQuat.w = (float)raw->GetH();
        _resources.insert(std::make_pair(image.mKey, image));
    }
    return true;
}

bool AtlasSys::ImportAtlas(const std::string & url)
{
    auto json = mmc::Json::FromFile(url);
    ASSERT_LOG(json != nullptr, url.c_str());
    auto dir = tools::GetFileFolder(url);
    dir += json->At("meta", "image")->ToString();

    auto ptr = Global::Ref().mRawSys->Import(dir);
    ASSERT_LOG(ptr != nullptr, url.c_str());
    auto raw = CastPtr<GLTexture>(ptr);

    auto w = json->At("meta", "size", "w")->ToNumber();
    auto h = json->At("meta", "size", "h")->ToNumber();
    for (auto val : json->At("frames"))
    {
        if (!IsHasKey(val.mKey))
        {
            Image image;
            image.mKey = val.mKey;
            image.mID = raw->GetID();
            image.mQuat.x = val.mVal->At("frame")->At("x")->ToNumber() / w;
            image.mQuat.y = val.mVal->At("frame")->At("y")->ToNumber() / h;
            image.mQuat.z = (val.mVal->At("frame")->At("x")->ToNumber() + val.mVal->At("frame")->At("w")->ToNumber()) / w;
            image.mQuat.w = (val.mVal->At("frame")->At("y")->ToNumber() + val.mVal->At("frame")->At("h")->ToNumber()) / h;
            _resources.insert(std::make_pair(image.mKey, image));
        }
    }
    return true;
}

void AtlasSys::DeleteImage(const std::string & url)
{
    _resources.erase(url);
    Global::Ref().mRawSys->Delete(url);
}

void AtlasSys::DeleteAtlas(const std::string & url)
{
    auto json = mmc::Json::FromFile(url);
    ASSERT_LOG(json != nullptr, url.c_str());
    auto dir = tools::GetFileFolder(url);
    dir += json->At("meta", "image")->ToString();
    Global::Ref().mRawSys->Delete(dir);
    for (auto val : json->At("frames"))
    { _resources.erase(val.mKey); }
}
