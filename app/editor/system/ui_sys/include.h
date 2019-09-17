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

class UIObject;
class UIState;
class UISys;

enum class DirectEnum {
    kU, kD, kL, kR, LENGTH
};

// ---
//  控件列表
// ---
enum class UITypeEnum {
    kOTHER,                 //  其他
    kLAYOUT,                //  布局
    kTREEBOX,               //  树形框
    kTEXTBOX,               //  文本框
    kIMAGEBOX,              //  图片框
    kCOMBOBOX,              //  下拉框
    kUICONVAS,              //  UI画布
    kGLCONVAS,              //  GL画布
};

// ---
//  控件对齐
// ---
enum class UIAlignEnum {
    kDEFAULT,               //  绝对坐标
    kCLING_T = 1 << 1,      //  靠上
    kCLING_B = 1 << 2,      //  靠下
    kCLING_L = 1 << 3,      //  靠左
    kCLING_R = 1 << 4,      //  靠右
    kCENTER_H = 1 << 5,     //  水平居中
    kCENTER_V = 1 << 6,     //  垂直居中
    kSTRETCH_H = 1 << 7,    //  水平拉伸
    kSTRETCH_V = 1 << 8,    //  垂直拉伸
    LENGTH,
};

// ---
//  文本对齐
// ---
enum class UITextAlignEnum {
    kCLING_T = 1 << 1,      //  靠上
    kCLING_B = 1 << 2,      //  靠下
    kCLING_L = 1 << 3,      //  靠左
    kCLING_R = 1 << 4,      //  靠右
    kCENTER_H = 1 << 5,     //  水平居中
    kCENTER_V = 1 << 6,     //  垂直居中
};

// ---
//  控件事件
// ---
enum class UIEventEnum {
    kKEY,               //  按键
    kMOUSE,             //  鼠标
    kEDIT_TEXT_FINISH,  //  输入完成
    kMENU,              //  菜单
};

//  转化为ImGuiID
template <class T>
inline std::string ImID(const T & v)
{
    return "##" + std::to_string(v);
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
