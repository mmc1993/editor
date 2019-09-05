#pragma once

#include "../base.h"

//  0. 单独解析节点
//  1. 父级部署子级
//  解析标签:
//      __Property
//      __Children
//          MyGUI
//          Widget
//          Property
//          UserData

class UIParser {
public:
    static UIClass * Parse(const std::string & url);
    static UIClass * Parse(const mmc::JsonValue::Value json);
    static void Parse__Property(const mmc::JsonValue::Value json, UIClass * object);
    static void Parse__Children(const mmc::JsonValue::Value json, UIClass * object);
    static UIClass * CreateObject(const int type);
};
