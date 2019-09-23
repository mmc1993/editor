#pragma once

#include "../../include.h"

class GLObject;

class Project {
public:
    GLObject * mRoot;

public:
    Project();
    ~Project();
    const std::string & GetDir() const { return mDir; }
    const std::string & GetURL() const { return mURL; }
    bool Load(const std::string & url);
    void Save(const std::string & url);
    void Free();

private:
    std::string mURL;
    std::string mDir;
};