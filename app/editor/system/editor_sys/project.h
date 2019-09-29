#pragma once

#include "../../include.h"

class GLObject;

class Project {
public:
    bool IsOpen() const { return _root != nullptr; }
    const std::string & GetURL() const { return _url; }
    const std::string & GetDIR() const { return tools::GetFileDir(_url); }
    void New(const std::string & url);
    bool Load(const std::string & url);
    void Save(const std::string & url);
    const SharePtr<GLObject> & GetRoot() { return _root; }

private:
    SharePtr<GLObject> _root;
    std::string         _url;
};