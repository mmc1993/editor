#include "ui_sys.h"

UISys::UISys()
{ }

UISys::~UISys()
{ }

void UISys::Update(float dt)
{
    //  拷贝副本, 防止在遍历过程中增删数组
    auto windows = mWindows;

    for (auto & window : windows)
    {
        window->Render(dt);
    }
}

void UISys::OpenWindow(const std::string & url, const std::any & param)
{
    auto window = mWindows.emplace_back(UIParser::Parse(url));
    window->ResetLayout(  );
    window->WakeInit(param);
}

void UISys::FreeWindow(const SharePtr<UIObject> & window)
{
    auto it = std::remove(mWindows.begin(), mWindows.end(), window);
    ASSERT_LOG(it != mWindows.end(), "");
    mWindows.erase(it);
}

bool UISys::CheckOpen(const SharePtr<UIObject> & window)
{ 
    return mWindows.end() != std::find(mWindows.begin(), mWindows.end(), window);
}

const std::vector<SharePtr<UIObject>> & UISys::GetWindows()
{
    return mWindows;
}

