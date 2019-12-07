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

void UISys::OpenWindow(const std::string & url, const std::any & param)
{
    auto window = _windows.emplace_back(UIParser::Parse(url));
    window->ResetLayout(  );
    window->WakeInit(param);
}

void UISys::FreeWindow(const SharePtr<UIObject> & window)
{
    auto it = std::remove(_windows.begin(), _windows.end(), window);
    ASSERT_LOG(it != _windows.end(), "");
    _windows.erase(it);
}

bool UISys::CheckOpen(const SharePtr<UIObject> & window)
{ 
    return _windows.end() != std::find(_windows.begin(), _windows.end(), window);
}

const std::vector<SharePtr<UIObject>> & UISys::GetWindows()
{
    return _windows;
}

