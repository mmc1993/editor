#include "res_mgr.h"

bool ResMgr::Import(const std::string & url)
{
    Res* res = nullptr;
    if (auto suffix = string_tool::QueryFileSuffix(url); suffix == ".png" || suffix == ".jpg")
    {
        res = new ResBitmap();
    }
    auto ret = res->Init(url);
    ASSERT_LOG(ret, "{0}", url);
    CHECK_RET(ret, false);
    return true;
}

void ResMgr::Delete(const std::string & url)
{ 
    auto it = _resources.find(url);
    if (it != _resources.end())
    {
        delete it->second;
        _resources.erase(it);
    }
}

void ResMgr::Clear()
{
    for (auto res : _resources)
    {
        delete res.second;
    }
    _resources.clear();
}
