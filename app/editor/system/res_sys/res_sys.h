#pragma once

#include "res.h"

class ResSys {
public:
    //  删除资源
    bool OptDeleteRes();
    //  移动资源
    bool OptModifyRes();
    //  设置资源类型
    bool OptSetResType(uint id, uint type);
    bool OptSetResType(Res * res, uint type);

    //  检索
    void Retrieve();
    //  返回指定ID资源
    const Res * GetRes(uint id);
    //  返回指定类型资源
    std::vector<const Res *> GetResByType(Res::TypeEnum tag);
    //  返回指定匹配资源
    std::vector<const Res *> GetResByWord(const std::string & word);

private:
    std::map<uint, Res *> _resources;
};