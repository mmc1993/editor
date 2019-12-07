#pragma once

#include "include.h"
#include "ui_menu.h"
#include "ui_state/ui_state.h"
#include "ui_parse/ui_parse.h"
#include "ui_object/ui_object.h"

class UISys {
public:
    UISys();
    ~UISys();
    void Update(float dt);
    void OpenWindow(const std::string & url,
                    const std::any & param = std::any());
    void FreeWindow(const SharePtr<UIObject> & window);
    bool CheckOpen(const SharePtr<UIObject> & window);
    const std::vector<SharePtr<UIObject>> & GetWindows();

private:
    std::vector<SharePtr<UIObject>> _windows;
};
