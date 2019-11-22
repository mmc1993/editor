#include "raw_sys.h"

SharePtr<Raw> RawSys::Import(const std::string & url)
{
    auto it = _resources.find(url);
    if (it != _resources.end() && !it->second.expired())
    {
        return it->second.lock();
    }

    SharePtr<Raw> raw;
    auto suffix = tools::GetFileSuffix(url);
    if (suffix == ".fnt")
    {
        raw = std::create_ptr<GLFont>();
    }
    else if (suffix == ".obj")
    {
        raw = std::create_ptr<GLMesh>();
    }
    else if (suffix == ".png")
    {
        raw = std::create_ptr<GLTexture>();
    }
    else if (suffix == ".program")
    {
        raw = std::create_ptr<GLProgram>();
    }
    else if (suffix == ".material")
    {
        raw = std::create_ptr<GLMaterial>();
    }
    if (raw->Init(url))
    {
        _resources[url] = raw;
    }
    return raw;
}

void RawSys::Clear()
{
    for (auto it = _resources.begin(); it != _resources.end();)
    {
        if (it->second.expired())
        {
            it = _resources.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
