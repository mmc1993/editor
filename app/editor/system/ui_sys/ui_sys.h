#pragma once

#include "include.h"

class UISys {
public:
    UISys();
    ~UISys();
    void Update(float dt);
    const SharePtr<UIObject> & GetRoot();
    void SetRoot(const std::string & url);
    void SetRoot(const SharePtr<UIObject> & object);

private:
    SharePtr<UIObject> _root;
};
