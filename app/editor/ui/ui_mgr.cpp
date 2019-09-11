#include "ui_mgr.h"
#include "ui_menu.h"
#include "ui_parse/ui_parse.h"
#include "ui_state/ui_state.h"
#include "ui_class/ui_class.h"

UIMgr::UIMgr(): _layout(nullptr)
{ }

UIMgr::~UIMgr()
{
    SetRoot(nullptr);
}

UIClass * UIMgr::GetRoot()
{
    return _layout;
}

void UIMgr::SetRoot(UIClass * uiClass)
{
    SAFE_DELETE(_layout);
    _layout = uiClass;
}

void UIMgr::SetRoot(const std::string & url)
{
     SetRoot(UIParser::Parse(url));
     _layout->ResetLayout();
}

void UIMgr::Update(float dt)
{
    _layout->Render(dt);
}

