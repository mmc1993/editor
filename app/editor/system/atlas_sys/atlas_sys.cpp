#include "atlas_sys.h"
#include "../raw_sys/raw_sys.h"

//  TODO
//      图片资源允许被重复加载, 
//      当重复加载时, 会返回上一次加载的结果
//      当重复删除时, 只有第一次删除有效, 后续删除操作都被忽略
//  这是一个不太好的设计, 因为会导致资源混乱.
//  但是, 如果资源引用计数由上层管理, 会牺牲易用性
//  所以这里考虑后续使用share_ptr管理资源, 从而避免以上问题.

void AtlasSys::Delete(const std::string & url)
{
    auto suffix = tools::QueryFileSuffix(url);
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
    auto suffix = tools::QueryFileSuffix(url);
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
        auto raw = (RawBitmap *)Global::Ref().mRawSys->Import(url);
        ASSERT_LOG(raw != nullptr, url.c_str());
        CHECK_RET(raw != nullptr, false);

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
    auto json = mmc::JsonValue::FromFile(url);
    ASSERT_LOG(json != nullptr, url.c_str());
    auto dir = tools::QueryFileDir(url);
    dir += json->At("meta", "image")->ToString();

    auto raw = (RawBitmap *)Global::Ref().mRawSys->Import(dir);
    ASSERT_LOG(raw, url.c_str());

    auto w = json->At("meta", "size", "w")->ToFloat();
    auto h = json->At("meta", "size", "h")->ToFloat();
    for (auto val : json->At("frames"))
    {
        if (!IsHasKey(val.mKey))
        {
            Image image;
            image.mKey = val.mKey;
            image.mID = raw->GetID();
            image.mQuat.x = val.mValue->At("frame")->At("x")->ToFloat() / w;
            image.mQuat.y = val.mValue->At("frame")->At("y")->ToFloat() / h;
            image.mQuat.z = (val.mValue->At("frame")->At("x")->ToFloat() + val.mValue->At("frame")->At("w")->ToFloat()) / w;
            image.mQuat.w = (val.mValue->At("frame")->At("y")->ToFloat() + val.mValue->At("frame")->At("h")->ToFloat()) / h;
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
    auto json = mmc::JsonValue::FromFile(url);
    ASSERT_LOG(json != nullptr, url.c_str());
    auto dir = tools::QueryFileDir(url);
    dir += json->At("meta", "image")->ToString();
    Global::Ref().mRawSys->Delete(dir);
    for (auto val : json->At("frames"))
    { _resources.erase(val.mKey); }
}
