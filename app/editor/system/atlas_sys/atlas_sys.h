#pragma once

#include "../../include.h"

class AtlasSys {
public:
    struct Image {
        glm::vec4   mQuat;
        std::string mKey;
        std::uint32_t mID;
    };

public:
    void Delete(const std::string & url);
    bool Import(const std::string & url);
    bool IsHasKey(const std::string & key) const;
    const Image & Get(const std::string & key) const;

private:
    bool ImportImage(const std::string & url);
    bool ImportAtlas(const std::string & url);
    void DeleteImage(const std::string & url);
    void DeleteAtlas(const std::string & url);

private:
    std::map<std::string, Image> _resources;
};