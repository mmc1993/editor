#pragma once

#include "../../include.h"

class CfgSys {
public:
    CfgSys(): _root(mmc::JsonValue::Hash())
    { }

    ~CfgSys()
    { }

    template <class ...Keys>
    mmc::JsonValue::Value At(Keys && ...keys)
    {
        return _root->At(std::forward<Keys>(keys)...);
    }

    void Import(const std::string & url)
    {
        auto value = mmc::JsonValue::FromFile(url);
        ASSERT_LOG(value != nullptr, url.c_str());
        _root->Insert(value, url);
    }

    void Save(const std::string & url)
    {
        ASSERT_LOG(_root->IsHashKey(url), url.c_str());
        std::ofstream ofile(url);
        auto buffer = std::to_string(_root->At(url));
        ofile.write(buffer.c_str(), buffer.size());
        ofile.close();
    }

    void Save()
    {
        for (auto pair : _root)
        {
            Save(pair.mKey);
        }
    }

private:
    mmc::JsonValue::Value _root;
};