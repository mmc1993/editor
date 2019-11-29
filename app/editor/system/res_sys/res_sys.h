#pragma once

#include "res.h"

class ResSys {
public:
    //  删除资源
    void OptDeleteRes(uint id);
    void OptDeleteRes(Res * res);
    //  移动资源
    void OptModifyRes(uint id, const std::string & url);
    void OptModifyRes(Res * res, const std::string & url);
    //  设置资源类型
    void OptSetResType(uint id, uint type);
    void OptSetResType(Res * res, uint type);

    //  检索
    void Retrieve();
    //  返回指定ID资源
    Res * GetRes(uint id);
    //  返回指定类型资源
    std::vector<Res *> GetResByType(Res::TypeEnum type);
    std::vector<Res *> GetResByType(std::initializer_list<Res::TypeEnum> types);
    //  返回指定匹配资源
    std::vector<Res *> GetResByWord(const std::string & word);

private:
    void DeleteRes(Res * res);

private:
    std::map<uint, Res *> _resources;
};