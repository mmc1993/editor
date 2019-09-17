#pragma once

#include "include.h"

class UISys {
public:
    UISys();
    ~UISys();
    UIObject * GetRoot();
    void SetRoot(UIObject * object);
    void SetRoot(const std::string & url);

    void Update(float dt);

private:
    UIObject * _layout;
};
