#pragma once

#include "raw.h"

class RawSys {
public:
    template <class T>
    SharePtr<T> Get(const std::string & url)
    {
        auto ptr = Import(url);
        ASSERT_LOG(ptr != nullptr, url.c_str());
        return CastPtr<T>(ptr);
    }

    void Clear();
private:
    SharePtr<Raw> Import(const std::string & url);

private:
    std::map<std::string, WeakPtr<Raw>> _resources;
};