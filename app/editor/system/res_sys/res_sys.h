#pragma once

#include "res.h"

class ResSys {
public:
    //  删除资源
    bool OptDeleteRes(uint res);
    bool OptDeleteRes(Res * res);
    //  移动资源
    bool OptModifyRes(uint res, const std::string & url);
    bool OptModifyRes(Res * res, const std::string & url);
    //  设置资源类型
    bool OptSetResType(uint res, uint type);
    bool OptSetResType(Res * res, uint type);

    //  检索
    void Retrieve();
    //  返回指定ID资源
    Res * GetRes(uint id);
    //  返回指定类型资源
    std::vector<Res *> GetResByType(Res::TypeEnum type);
    //  返回指定匹配资源
    std::vector<Res *> GetResByWord(const std::string & word);

private:
    std::map<uint, Res *> _resources;
};