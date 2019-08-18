#include "ui_class.h"
#include "../ui_state/ui_state.h"

std::vector<UIClass*>& UIClass::GetChildren()
{
    return _children;
}

void UIClass::AddChild(UIClass * child)
{
    _children.push_back(child);
}

void UIClass::DelChild(UIClass * child)
{
    auto it = std::find(_children.begin(), _children.end(), child);
    if (it != _children.end()) { delete *it; _children.erase(it); }
}

void UIClass::ClearChild()
{
    while (!_children.empty())
    {
        delete _children.back();
        _children.pop_back();
    }
}

void UIClass::Update(float dt)
{
    OnUpdate(dt);
    std::for_each(_children.begin(), _children.end(),
                  std::bind(&UIClass::Update,
                  std::placeholders::_1, dt));
}

void UIClass::Draw(float dt)
{
    OnDraw(dt);
    std::for_each(_children.begin(), _children.end(), 
                  std::bind(&UIClass::OnDraw, 
                  std::placeholders::_1, dt));
}

void UIClassDDContainer::OnUpdate(float dt)
{
}

void UIClassDDContainer::OnDraw(float dt)
{
    auto & state = GetState<UIStateDDContainer>();
    ImGui::SetNextWindowSize(ImVec2(state.mMove.z, state.mMove.w));
    ImGui::SetNextWindowPos(ImVec2(state.mMove.x, state.mMove.y));
    ImGui::Begin(state.mName.c_str());

    ImGui::End();
}

void UIClassButton::OnUpdate(float dt)
{
}

void UIClassButton::OnDraw(float dt)
{
}
