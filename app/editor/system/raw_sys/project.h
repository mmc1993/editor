#pragma once

#include "../../include.h"
#include "gl_object.h"
#include "component.h"
#include "res.h"
#include "raw.h"

class Project {
public:
    Project();

    //  项目
    bool IsOpen() const { return _object != nullptr; }
    const std::string & GetURL() const { return _url; }
    const std::string & GetDIR() const { return tools::GetFileFolder(_url); }
    
    void New(const std::string & url);
    void Load(const std::string & url);
    void Save(const std::string & url);

    //  对象
    const SharePtr<GLObject> & GetObject()        { return _object; }
    const SharePtr<GLObject> & GetObject(uint id) { return _objects.at(id); }

    SharePtr<GLObject> NewObject();
    void DeleteObject(const uint & id);
    void DeleteObject(const SharePtr<GLObject> & object);

    //  资源
    bool DeleteRes(uint id);
    bool DeleteRes(Res * res);
    bool ModifyRes(uint id, const std::string & url);
    bool ModifyRes(Res * res, const std::string & url);
    bool SetResType(uint id, uint type);
    bool SetResType(Res * res, uint type);

    void Retrieve();    //  检索
    Res * GetRes(uint id);
    std::vector<Res *> GetResByType(const Res::TypeEnum & type);
    std::vector<Res *> GetResByType(const std::initializer_list<Res::TypeEnum> & types);
    std::vector<Res *> GetResByWord(const std::string & word);

private:
    std::map<uint, Res *>               _resources;     //  资源
    std::map<uint, SharePtr<GLObject>>  _objects;       //  对象

    SharePtr<GLObject> _object;
    std::string        _url;       //  项目路径
    uint               _gid;       //  全局ID
};