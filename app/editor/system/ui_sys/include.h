#pragma once

#include "../../include.h"

//  扩展ImVec
#ifndef IM_VEC2_CLASS_EXTRA
#define IM_VEC2_CLASS_EXTRA                                 \
    ImVec2(const glm::vec2 & val): x(val.x), y(val.y)       \
    { }                                                     \
    ImVec2 & operator=(const glm::vec2 & val)               \
    {                                                       \
        x = val.x;                                          \
        y = val.y;                                          \
        return *this;                                       \
    }                                                       \
    operator glm::vec2() const                              \
    {                                                       \
        return glm::vec2(x, y);                             \
    }                                                       
#endif

#ifndef IM_VEC4_CLASS_EXTRA
#define IM_VEC4_CLASS_EXTRA                                 \
    ImVec4(const glm::vec3 & val)                           \
        : x(val.x), y(val.y)                                \
        , z(val.z), w(0)                                    \
    { }                                                     \
    ImVec4(const glm::vec4 & val)                           \
        : x(val.x), y(val.y)                                \
        , z(val.z), w(val.w)                                \
    { }                                                     \
    ImVec4 & operator=(const glm::vec3 & val)               \
    {                                                       \
        x = val.x;                                          \
        y = val.y;                                          \
        z = val.z;                                          \
        return *this;                                       \
    }                                                       \
    ImVec4 & operator=(const glm::vec4 & val)               \
    {                                                       \
        x = val.x;                                          \
        y = val.y;                                          \
        z = val.z;                                          \
        w = val.w;                                          \
        return *this;                                       \
    }                                                       \
    operator glm::vec3() const                              \
    {                                                       \
        return glm::vec3(x, y, z);                          \
    }                                                       \
    operator glm::vec4() const                              \
    {                                                       \
        return glm::vec4(x, y, z, w);                       \
    }                                                       
#endif

#include "imgui.h"

class UIState;
class UIParser;
class UIObject;

enum class DirectEnum {
    kU, kD, kL, kR, Length, kNone,
};

// ---
//  控件列表
// ---
enum class UITypeEnum {
    kOther,                 //  其他
    kLayout,                //  布局
    kTreeBox,               //  树形框
    kTextBox,               //  文本框
    kImageBox,              //  图片框
    kComboBox,              //  下拉框
    kUICanvas,              //  UI画布
    kGLCanvas,              //  GL画布
};

// ---
//  控件对齐
// ---
enum class UIAlignEnum {
    kDefault,              //  绝对坐标
    kClingT = 1 << 1,      //  靠上
    kClingB = 1 << 2,      //  靠下
    kClingL = 1 << 3,      //  靠左
    kClingR = 1 << 4,      //  靠右
    kCenterH = 1 << 5,     //  水平居中
    kCenterV = 1 << 6,     //  垂直居中
    kStretchH = 1 << 7,    //  水平拉伸
    kStretchV = 1 << 8,    //  垂直拉伸
    Length,
};

// ---
//  文本对齐
// ---
enum class UITextAlignEnum {
    kClingT = 1 << 1,      //  靠上
    kClingB = 1 << 2,      //  靠下
    kClingL = 1 << 3,      //  靠左
    kClingR = 1 << 4,      //  靠右
    kCenterH = 1 << 5,     //  水平居中
    kCenterV = 1 << 6,     //  垂直居中
};

// ---
//  控件事件
// ---
enum class UIEventEnum {
    kKey,               //  按键
    kMouse,             //  鼠标
    kEdit,              //  编辑
    kMenu,              //  菜单
    kDrag,              //  拖放
    kInit,              //  构造
};

//  转化为ImGuiID
template <class T>
std::string ImID(const T & v)
{
    if constexpr (std::is_pointer_v<T> || std::is_arithmetic_v<T>)
    {
        return SFormat("##{0}", (size_t)v);
    }
    else
    {
        return SFormat("{0}##{1}", v, (size_t)&v);
    }
}

//  ImGui 工具函数
namespace imgui_tools {
    static int OnResizeBuffer(ImGuiInputTextCallbackData * data)
    {
        auto value = (std::string *)data->UserData;
        value->resize(data->BufTextLen);
        data->Buf = value->data();
        return value->capacity();
    }
}
