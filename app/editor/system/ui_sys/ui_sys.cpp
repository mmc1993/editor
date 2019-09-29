#include "ui_sys.h"
#include "ui_parse/ui_parse.h"
#include "ui_object/ui_object.h"

UISys::UISys()
{ }

UISys::~UISys()
{ }

void UISys::Update(float dt)
{
    _root->Render(dt);
}

const SharePtr<UIObject> & UISys::GetRoot()
{
    return _root;
}

void UISys::SetRoot(const std::string & url)
{
    SetRoot(UIParser::Parse(url));
    _root->ResetLayout();
}

void UISys::SetRoot(const SharePtr<UIObject> & object)
{
    _root = object;
    _root->ResetLayout();
}
