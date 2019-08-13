#pragma once

#include "../base.h"

class UIParser {
public:
    UIParser()
    { }

    virtual ~UIParser()
    { }

    virtual UIState * Parse(const std::string & url) = 0;
};

class UIParserPanel : public UIParser {
public:
    virtual UIState * Parse(const std::string & url) override;
};

class UIParserButton : public UIParser {
    virtual UIState * Parse(const std::string & url) override;
};