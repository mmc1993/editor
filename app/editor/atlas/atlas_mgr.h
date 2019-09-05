#pragma once

#include "../include.h"

class AtlasMgr {
public:
    struct Image {
        glm::vec4   mQuat;
        std::string mKey;
    };

public:
    bool Import(const std::string & url);

    const Image & Get(const std::string & key)
    {
        auto it = _resources.find(key);
        ASSERT_LOG(it != _resources.end(), "{0}", key);
        return it->second;
    }

private:
    bool ImportImage(const std::string & url);
    bool ImportAtlas(const std::string & url);

private:
    std::map<std::string, Image> _resources;
};