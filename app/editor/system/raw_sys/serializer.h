#pragma once

#include "../../include.h"

class Project;

//  提供序列化操作抽象类
class Serializer {
public:
    virtual void EncodeBinary(std::ostream & os, Project * project) = 0;
    virtual void DecodeBinary(std::istream & is, Project * project) = 0;
};
