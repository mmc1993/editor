#include "raw_mgr.h"

Raw * RawMgr::Import(const std::string & url)
{
    auto it = _resources.find(url);
    CHECK_RET(it == _resources.end(), it->second);

    Raw * raw = nullptr;
    if (auto suffix = string_tool::QueryFileSuffix(url); suffix == ".png" || suffix == ".jpg")
    {
        raw = new RawBitmap();
    }
    auto ret = raw->Init(url);
    ASSERT_LOG(ret, "{0}", url);
    if (ret) { _resources.insert(std::make_pair(url, raw)); }
    else { SAFE_DELETE(raw); }
    return raw;
}

void RawMgr::Delete(const std::string & url)
{ 
    auto it = _resources.find(url);
    if (it != _resources.end())
    {
        delete it->second;
        _resources.erase(it);
    }
}

void RawMgr::Clear()
{
    for (auto res : _resources)
    {
        delete res.second;
    }
    _resources.clear();
}
