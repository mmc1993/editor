#pragma once

#include "../include.h"

//  解析标签:
//      __Property
//      __Children

class UIParser {
public:
    //  字符串值基础类型
    enum class StringValueTypeEnum {
        kErr,           //  无效
        kInt,           //  整数
        kBool,          //  布尔
        kFlag,          //  标志
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

public:
    static SharePtr<UIObject> CreateObject(const int type);
    static SharePtr<UIObject> Parse(const std::string & url);
    static SharePtr<UIObject> Parse(const mmc::Json::Pointer json);
    static void Parse__Property(const mmc::Json::Pointer json, const SharePtr<UIObject> & object);
    static void Parse__Children(const mmc::Json::Pointer json, const SharePtr<UIObject> & object);
    static bool FromStringParse(StringValueTypeEnum type, const std::string & val, const std::string & sep, void * out);
};
