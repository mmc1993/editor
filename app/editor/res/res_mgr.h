#pragma once

#include "res.h"

class ResMgr {
public:
    bool Import(const std::string & url);
    void Delete(const std::string & url);

    template <class T>
    T * Ref(const std::string & url)
    {
        auto it = _resources.find(url);
        ASSERT_LOG(it != _resources.end(), "{0}", url);
        return (T *)it->second;
    }
private:
    std::map<std::string, Res *> _resources;
};