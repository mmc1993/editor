#include "ui_sys.h"
#include "ui_menu.h"
#include "ui_parse/ui_parse.h"
#include "ui_state/ui_state.h"
#include "ui_class/ui_class.h"

UISys::UISys(): _layout(nullptr)
{ }

UISys::~UISys()
{
    SetRoot(nullptr);
}

UIClass * UISys::GetRoot()
{
    return _layout;
}

void UISys::SetRoot(UIClass * uiClass)
{
    SAFE_DELETE(_layout);
    _layout = uiClass;
}

void UISys::SetRoot(const std::string & url)
{
     SetRoot(UIParser::Parse(url));
     _layout->ResetLayout();
}

void UISys::Update(float dt)
{
    _layout->Render(dt);
}

