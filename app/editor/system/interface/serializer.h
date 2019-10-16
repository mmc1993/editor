#pragma once

#include "../../include.h"

namespace interface {

//  提供序列化操作抽象类
class Serializer {
public:
    //  字符串值基础类型
    enum class StringValueTypeEnum {
        kErr,           //  无效
        kInt,           //  整数
        kBool,          //  布尔
        kFloat,         //  浮点
        kString,        //  字符串
        kVector2,       //  向量2
        kVector3,       //  向量3
        kVector4,       //  向量4
        kIntList,       //  整数列表
        kFloatList,     //  浮点列表
        kStringList,    //  字符串列表
        kColor4,        //  颜色
        kAsset,         //  资源
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