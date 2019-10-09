#include "raw_sys.h"

SharePtr<Raw> RawSys::Import(const std::string & url)
{
    auto it = _resources.find(url);
    CHECK_RET(it == _resources.end(), it->second);

    SharePtr<Raw> raw;
    auto suffix = tools::GetFileSuffix(url);
    if (suffix == ".obj")
    {
        raw = std::create_ptr<GLMesh>();
    }
    else if (suffix == ".png" || suffix == ".jpg")
    {
        raw = std::create_ptr<GLTexture>();
    }
    auto ret = raw->Init(url);
    ASSERT_LOG(ret, "{0}", url);
    if (ret)
    {
        _resources.insert(std::make_pair(url, raw));
    }
    return raw;
}

void RawSys::Delete(const std::string & url)
{ 
    auto it = _resources.find(url);
    if (it != _resources.end())
    {
        _resources.erase(it);
    }
}

void RawSys::Clear()
{
    _resources.clear();
}
