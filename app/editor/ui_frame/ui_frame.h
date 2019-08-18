#pragma once

#include "base.h"

class UIFrame {
public:
    UIFrame();
    ~UIFrame();
    UIClass * GetRoot();
    void SetRoot(UIClass * uiClass);
    void SetRoot(const std::string & url);

    void Update(float dt);

private:
    UIClass * _uiRoot;
};
