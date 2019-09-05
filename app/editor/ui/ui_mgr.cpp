#include "ui_mgr.h"
#include "ui_parse/ui_parse.h"
#include "ui_state/ui_state.h"
#include "ui_class/ui_class.h"

UIMgr::UIMgr(): _uiRoot(nullptr)
{
}

UIMgr::~UIMgr()
{
    SetRoot(nullptr);
}

UIClass * UIMgr::GetRoot()
{
    return _uiRoot;
}

void UIMgr::SetRoot(UIClass * uiClass)
{
    SAFE_DELETE(_uiRoot);
    _uiRoot = uiClass;
}

void UIMgr::SetRoot(const std::string & url)
{
     SetRoot(UIParser::Parse(url));
     _uiRoot->ResetLayout();
}

void UIMgr::Update(float dt)
{
    _uiRoot->Render(dt);
}

