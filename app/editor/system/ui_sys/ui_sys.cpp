#include "ui_sys.h"

UISys::UISys()
{ }

UISys::~UISys()
{ }

void UISys::Update(float dt)
{
    //  拷贝副本, 防止在遍历过程中增删数组
    auto windows = _windows;

    for (auto & window : windows)
    {
        window->Render(dt);
    }
}

void UISys::OpenWindow(const std::string & url)
{
    auto window = _windows.emplace_back(UIParser::Parse(url));
    window->ResetLayout();
}

void UISys::FreeWindow(const SharePtr<UIObject> & ptr)
{
    auto it = std::remove(_windows.begin(), _windows.end(), ptr);
    if (it != _windows.end()) { _windows.erase(it); }
}

const std::vector<SharePtr<UIObject>> & UISys::GetWindows()
{
    return _windows;
}

