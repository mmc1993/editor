#pragma once

#include "../include.h"

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
class UIClass;
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

// ---
//  默认值定义
// ---
constexpr auto LAYOUT_STRETCH_BORDER = 5.0f;      //  Layout 可拖动边距

// ---
//  字符串转其他类型
// ---
inline bool     s2b(const std::string & s) { return s == "ok"; }
inline int      s2i(const std::string & s) { return std::stoi(s); }
inline float    s2f(const std::string & s) { return std::stof(s); }

inline glm::vec2 s2v2(const std::string & s)
{
    auto arr = string_tool::Split(s, " ");
    ASSERT_LOG(arr.size() == 2, "Arr Length: {0}!", arr.size());
    return glm::vec2(s2f(arr.at(0)), s2f(arr.at(1)));
}

inline glm::vec3 s2v3(const std::string & s)
{
    auto arr = string_tool::Split(s, " ");
    ASSERT_LOG(arr.size() == 3, "Arr Length: {0}!", arr.size());
    return glm::vec3(s2f(arr.at(0)), s2f(arr.at(1)), s2f(arr.at(2)));
}

inline glm::vec4 s2v4(const std::string & s)
{
    auto arr = string_tool::Split(s, " ");
    ASSERT_LOG(arr.size() == 4, "Arr Length: {0}!", arr.size());
    return glm::vec4(s2f(arr.at(0)), s2f(arr.at(1)), s2f(arr.at(2)), s2f(arr.at(3)));
}

//  字符串 转 数字数组
inline std::vector<int> s2vi(const std::string & s)
{
    std::vector<int> ret;
    auto arr = string_tool::Split(s, " ");
    std::transform(arr.begin(), arr.end(), std::back_inserter(ret), 
        [] (const std::string & s) { return std::stoi(s); });
    return ret;
}

//  字符串 转 字符串数组
inline std::vector<std::string> s2vs(const std::string & s)
{
    return string_tool::Split(s, ",");
}

// ---
//  自定义数据 Get/Set
// ---
using CustomData = std::map<std::string, std::any>;

//  解析数据
template <class T>
void __ParseUIData(CustomData & data, const std::string & key, const std::string & val)
{
    if constexpr (std::is_same_v<T, int>)           { data.emplace(key, s2i(val)); }
    else if constexpr (std::is_same_v<T, bool>)     { data.emplace(key, s2b(val)); }
    else if constexpr (std::is_same_v<T, float>)    { data.emplace(key, s2f(val)); }
    else if constexpr (std::is_same_v<T, std::string>)  { data.emplace(key, (val)); }
    else if constexpr (std::is_same_v<T, glm::vec2>)    { data.emplace(key, s2v2(val)); }
    else if constexpr (std::is_same_v<T, glm::vec3>)    { data.emplace(key, s2v3(val)); }
    else if constexpr (std::is_same_v<T, glm::vec4>)    { data.emplace(key, s2v4(val)); }
    else if constexpr (std::is_same_v<T, std::vector<int>>) { data.emplace(key, s2vi(val)); }
    else if constexpr (std::is_same_v<T, std::vector<std::string>>) { data.emplace(key, s2vs(val)); }
    else { static_assert(false); }
}

//  解析数据
#define __REG_PARSE_UI_DATA(data, key, val, T, K)     if (#K == key) __ParseUIData<T>(data, key, val)

//  获取数据
#define __REG_GET_UI_DATA(T, K, def)                                       \
    inline const T & __GetData##K(const CustomData & data)                 \
    {                                                                      \
        static T tmp = def;                                                \
        auto it = data.find(#K);                                           \
        if (it == data.end()) return tmp;                                  \
        return std::any_cast<const T &>(it->second);                       \
    }

//  设置数据
#define __REG_SET_UI_DATA(T, K)                                            \
    inline void __SetData##K(CustomData & data, const T & val)             \
    {                                                                      \
        data[#K] = val;                                                    \
    }

__REG_GET_UI_DATA(int, Align, 0) //  对齐
__REG_SET_UI_DATA(int, Align)

__REG_GET_UI_DATA(float, BorderNumber, 1) //  边框宽度
__REG_SET_UI_DATA(float, BorderNumber)

__REG_GET_UI_DATA(bool, IsMulti, false) //  多行
__REG_SET_UI_DATA(bool, IsMulti)
__REG_GET_UI_DATA(bool, IsWindow, false) //  窗口
__REG_SET_UI_DATA(bool, IsWindow)
__REG_GET_UI_DATA(bool, IsButton, false) //  按钮
__REG_SET_UI_DATA(bool, IsButton)
__REG_GET_UI_DATA(bool, IsVisible, true) //  可见
__REG_SET_UI_DATA(bool, IsVisible)
__REG_GET_UI_DATA(bool, IsShowNav, false) //  有导航栏
__REG_SET_UI_DATA(bool, IsShowNav)
__REG_GET_UI_DATA(bool, IsEditBox, false) //  编辑框
__REG_SET_UI_DATA(bool, IsEditBox)
__REG_GET_UI_DATA(bool, IsCanMove, false) //  可以移动
__REG_SET_UI_DATA(bool, IsCanMove)

__REG_GET_UI_DATA(bool, IsShowBorder, true)  //  显示边框
__REG_SET_UI_DATA(bool, IsShowBorder)
__REG_GET_UI_DATA(bool, IsCanStretch, false) //  可以拉伸
__REG_SET_UI_DATA(bool, IsCanStretch)
__REG_GET_UI_DATA(bool, IsFullScreen, false) //  铺满全屏
__REG_SET_UI_DATA(bool, IsFullScreen)
__REG_GET_UI_DATA(bool, IsShowMenuBar, false) //  菜单栏
__REG_SET_UI_DATA(bool, IsShowMenuBar)
__REG_GET_UI_DATA(bool, IsShowTitleBar, false) //  有标题栏
__REG_SET_UI_DATA(bool, IsShowTitleBar)
__REG_GET_UI_DATA(bool, IsShowScrollBar, false) //  有滚动条
__REG_SET_UI_DATA(bool, IsShowScrollBar)

__REG_GET_UI_DATA(std::string, Name, std::string()) //  名字
__REG_SET_UI_DATA(std::string, Name)
__REG_GET_UI_DATA(std::string, Tips, std::string()) //  提示
__REG_SET_UI_DATA(std::string, Tips)
__REG_GET_UI_DATA(std::string, LSkin, std::string()) //  皮肤
__REG_SET_UI_DATA(std::string, LSkin)

__REG_GET_UI_DATA(glm::vec4, Move, glm::vec4()) //  方位
__REG_SET_UI_DATA(glm::vec4, Move)
__REG_GET_UI_DATA(glm::vec4, _Move, glm::vec4()) //  方位
__REG_SET_UI_DATA(glm::vec4, _Move)
__REG_GET_UI_DATA(glm::vec4, Color0, glm::vec4()) //  颜色
__REG_SET_UI_DATA(glm::vec4, Color0)

__REG_GET_UI_DATA(glm::vec2, StretchMin, glm::vec2(LAYOUT_STRETCH_BORDER * 3, LAYOUT_STRETCH_BORDER * 3)) //  可拉动最小宽度
__REG_SET_UI_DATA(glm::vec2, StretchMin)

__REG_GET_UI_DATA(std::vector<std::string>, MenuBar, std::vector<std::string>()) //  菜单栏
__REG_SET_UI_DATA(std::vector<std::string>, MenuBar)

inline void ParseUIData(CustomData & data, const std::string & key, const std::string & val)
{
    __REG_PARSE_UI_DATA(data, key, val, int, Align);
    __REG_PARSE_UI_DATA(data, key, val, float, BorderNumber);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsMulti);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsWindow);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsButton);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsVisible);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowNav);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsEditBox);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsCanMove);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsCanStretch);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsFullScreen);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowBorder);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowMenuBar);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowTitleBar);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowScrollBar);
    __REG_PARSE_UI_DATA(data, key, val, std::string, Name);
    __REG_PARSE_UI_DATA(data, key, val, std::string, Tips);
    __REG_PARSE_UI_DATA(data, key, val, std::string, LSkin);
    __REG_PARSE_UI_DATA(data, key, val, glm::vec2, StretchMin);
    __REG_PARSE_UI_DATA(data, key, val, glm::vec4, Move);
    __REG_PARSE_UI_DATA(data, key, val, glm::vec4, Color0);
    __REG_PARSE_UI_DATA(data, key, val, std::vector<std::string>, MenuBar);
}

#define GetUIData(data, K)         __GetData##K(data)
#define SetUIData(data, K, val)    __SetData##K(data, val)

namespace std {
    inline std::string to_string(void * val)
    {
        return std::to_string((size_t)val);
    }
}

//  转化为ImGuiID
template <class T>
inline std::string ToImGuiID(const T & v)
{
    return "##" + std::to_string(v);
}
