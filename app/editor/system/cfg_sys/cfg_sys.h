#pragma once

#include "../include.h"
#include <filesystem>

class CfgSys {
public:
    CfgSys(): _root(mmc::JsonValue::FromValue(mmc::JsonValue::Hash()))
    { }

    ~CfgSys()
    { }

    template <class ...Keys>
    mmc::JsonValue::Value At(Keys && ...keys)
    {
        return _root->At(std::forward<Keys>(keys)...);
    }

    void Init(const std::string & root)
    {
        for (auto & entry : std::experimental::filesystem::directory_iterator(root))
        {
            Import(entry.path().string());
        }
    }

    void Import(const std::string & url)
    {
        auto fname = string_tool::Replace(url, "\\", "/");
        auto value = mmc::JsonValue::FromFile(fname);
        ASSERT_LOG(value != nullptr, url.c_str());

        auto key = string_tool::QueryFileName(fname);
        _files.insert(std::make_pair(key, fname));
        _root->Insert(value, key);
    }

    void Save(const std::string & key)
    {
        ASSERT_LOG(_root->IsHashKey(key), key.c_str());
        auto buffer = std::to_string(_root->At(key));
        std::ofstream ofile(_files.at(key));
        ofile.write(buffer.c_str(), buffer.size());
        ofile.close();
    }

    void Save()
    {
        for (auto pair : _files)
        {
            Save(pair.first);
        }
    }

private:
    mmc::JsonValue::Value _root;

    std::map<std::string, std::string> _files;
};