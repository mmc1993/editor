#pragma once

#include "../include.h"

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
    static UIObject * Parse(const std::string & url);
    static UIObject * Parse(const mmc::JsonValue::Value json);
    static void Parse__Property(const mmc::JsonValue::Value json, UIObject * object);
    static void Parse__Children(const mmc::JsonValue::Value json, UIObject * object);
    static UIObject * CreateObject(const int type);
};
