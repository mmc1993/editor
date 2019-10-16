#pragma once

#include "../include.h"

//  ������ǩ:
//      __Property
//      __Children

class UIParser {
public:
    static SharePtr<UIObject> CreateObject(const int type);
    static SharePtr<UIObject> Parse(const std::string & url);
    static SharePtr<UIObject> Parse(const mmc::Json::Pointer json);
    static void Parse__Property(const mmc::Json::Pointer json, const SharePtr<UIObject> & object);
    static void Parse__Children(const mmc::Json::Pointer json, const SharePtr<UIObject> & object);
};
