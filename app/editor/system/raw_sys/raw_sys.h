#pragma once

#include "raw.h"

class RawSys {
public:
    SharePtr<Raw> Import(const std::string & url);
    void          Delete(const std::string & url);
    void          Clear();

    template <class T>
    SharePtr<T> Get(const std::string & url)
    {
        auto it = _resources.find(url);
        ASSERT_LOG(it != _resources.end(), "{0}", url);
        return (T *)it->second;
    }
private:
    std::map<std::string, SharePtr<Raw>> _resources;
};