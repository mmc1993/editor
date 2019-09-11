#pragma once

#include "base.h"

class UIMgr {
public:
    UIMgr();
    ~UIMgr();
    UIClass * GetRoot();
    void SetRoot(UIClass * uiClass);
    void SetRoot(const std::string & url);

    void Update(float dt);

private:
    UIClass * _layout;
};
