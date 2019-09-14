#pragma once

#include "base.h"

class UISys {
public:
    UISys();
    ~UISys();
    UIClass * GetRoot();
    void SetRoot(UIClass * uiClass);
    void SetRoot(const std::string & url);

    void Update(float dt);

private:
    UIClass * _layout;
};
