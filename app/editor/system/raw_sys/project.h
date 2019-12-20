#pragma once

#include "../../include.h"
#include "gl_object.h"
#include "component.h"
#include "res.h"
#include "raw.h"

class Project {
public:
    Project();
    ~Project();

    //  项目
    bool IsOpen() const { return _object != nullptr; }
    const std::string & GetURL() const { return _url; }
    const std::string & GetDIR() const { return tools::GetFileFolder(_url); }
    
    void New(const std::string & url);
    void Load(const std::string & url);
    void Save(const std::string & url);

    //  对象
    bool IsExistObject(uint id) { return _objects.count(id) != 0; }
    const SharePtr<GLObject> & GetObject()        { return _object; }
    const SharePtr<GLObject> & GetObject(uint id) { return _objects.at(id); }

    SharePtr<GLObject> NewObject();
    void DeleteObject(const uint & id);
    void DeleteObject(const SharePtr<GLObject> & object);

    //  资源
    bool DeleteRes(Res * res);
    bool RenameRes(Res * res, const std::string & url);
    bool SetResType(Res * res, uint type);

    void Retrieve();    //  检索
    Res * GetRes(uint id);
    Res * GetResFromPGID(uint PGID);
    std::vector<Res *> GetResByType(const std::vector<Res::TypeEnum> & types);

private:
    std::string GetResURL(const std::string & url)
    { 
        return url + ".res";
    }

    Res * NewRes();

private:
    std::map<uint, Res *>               _resources;     //  资源
    std::map<uint, SharePtr<GLObject>>  _objects;       //  对象

    SharePtr<GLObject> _object;
    std::string        _url;       //  项目路径
    uint               _gid;       //  全局ID
};