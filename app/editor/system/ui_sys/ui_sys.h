#pragma once

#include "include.h"

class UISys {
public:
    UISys();
    ~UISys();
    void Update(float dt);
    void OpenWindow(const std::string & url);
    void FreeWindow(const SharePtr<UIObject> & ptr);
    const std::vector<SharePtr<UIObject>> & GetWindows();

private:
    std::vector<SharePtr<UIObject>> _windows;
};
