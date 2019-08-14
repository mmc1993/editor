#pragma once

#include "../base.h"

class UIParser {
public:
    static UIClass * Parse(const TiXmlElement * xml);
    static UIClass * Parse(const TiXmlElement * xml, UIClass * parent);

    static void ParseAttr(const TiXmlElement * xml, UIState * state);
    static void ParseProp(const TiXmlElement * xml, UIState * state);
    static void ParseUser(const TiXmlElement * xml, UIState * state);
};

class UIParserPanel {
public:
    static UIClass * Parse(const TiXmlElement * xml);
};

class UIParserButton {
public:
    static UIClass * Parse(const TiXmlElement * xml);
};