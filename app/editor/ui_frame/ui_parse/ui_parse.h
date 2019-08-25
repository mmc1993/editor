#pragma once

#include "../base.h"

class UIParser {
public:
    static UIClass * Parse(const std::string & url);
    static UIClass * Parse(const mmc::JsonValue::Value json);
    static UIClass * Parse(const mmc::JsonValue::Value json, UIClass * parent);

    static void ParseAttr(const mmc::JsonValue::Value json, UIState * state);
    static void ParseProp(const mmc::JsonValue::Value json, UIState * state);
    static void ParseUser(const mmc::JsonValue::Value json, UIState * state);
};

class UIParserLayout {
public:
    static UIClass * Parse(const mmc::JsonValue::Value json);
};
