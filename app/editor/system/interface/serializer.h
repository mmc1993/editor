#pragma once

#include "../../include.h"

namespace interface {

//  提供序列化操作抽象类
class Serializer {
public:
    //  字符串值基础类型
    enum class StringValueTypeEnum {
        kERR,           //  无效
        kINT,           //  整数
        kBOOL,          //  布尔
        kFLOAT,         //  浮点
        kSTRING,        //  字符串
        kVEC2,          //  向量2
        kVEC3,          //  向量3
        kVEC4,          //  向量4
        kINT_LIST,      //  整数列表
        kFLOAT_LIST,    //  浮点列表
        kSTRING_LIST,   //  字符串列表
    };

    static bool FromStringParse(
        StringValueTypeEnum type, 
        const std::string & val, 
        const std::string & sep, 
        void * out);

public:
    virtual void EncodeBinary(std::ofstream & os) = 0;
    virtual void DecodeBinary(std::ifstream & is) = 0;
};

#define FROM_STRING_PARSE_VARIABLE(type, key, val, sep, K, out) if (key == K) return interface::Serializer::FromStringParse(type, val, sep, &out)

}