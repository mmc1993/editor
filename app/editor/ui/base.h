#pragma once

#include "../include.h"
#include "imgui.h"

class UIState;
class UIClass;
class UIMgr;

enum class DirectEnum {
    kU, kD, kL, kR, LENGTH
};

//  控件列表
enum class UITypeEnum {
    kTREE,                  //  树形
    kIMAGE,                 //  图片
    kLAYOUT,                //  布局
    kTEXTBOX,               //  文本框
    kCOMBOBOX,              //  下拉框
    kUICONVAS,              //  UI画布
    kGLCONVAS,              //  GL画布
};

//  控件状态
enum class UIStateEnum {
    kDISABLED,              //  禁用
    kNORMAL,                //  正常
    kOVER,                  //  悬浮
    kPRESSED,               //  按下
    kSELECTED_DISABLED,     //  选中禁止
    kSELECTED_NORMAL,       //  选中正常
    kSELECTED_OVER,         //  选中悬浮
    kSELECTED_PRESSED,      //  选中按下
    LENGTH,
};

//  控件对齐
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

constexpr auto LAYOUT_DRAG_PADDING = 5.0f;      //  layout 可拖动边距

//  字符串转其他类型
inline bool     S2B(const std::string & s) { return s == "ok"; }
inline int      S2I(const std::string & s) { return std::stoi(s); }
inline float    S2F(const std::string & s) { return std::stof(s); }

inline glm::vec2 S2V2(const std::string & s)
{
    auto arr = string_tool::Split(s, " ");
    ASSERT_LOG(arr.size() == 2, "Arr Length: {0}!", arr.size());
    return glm::vec2(S2F(arr.at(0)), S2F(arr.at(1)));
}

inline glm::vec3 S2V3(const std::string & s)
{
    auto arr = string_tool::Split(s, " ");
    ASSERT_LOG(arr.size() == 3, "Arr Length: {0}!", arr.size());
    return glm::vec3(S2F(arr.at(0)), S2F(arr.at(1)), S2F(arr.at(2)));
}

inline glm::vec4 S2V4(const std::string & s)
{
    auto arr = string_tool::Split(s, " ");
    ASSERT_LOG(arr.size() == 4, "Arr Length: {0}!", arr.size());
    return glm::vec4(S2F(arr.at(0)), S2F(arr.at(1)), S2F(arr.at(2)), S2F(arr.at(3)));
}

//  自定义数据 Get/Set
using CustomData = std::map<std::string, std::any>;

//  解析数据
template <class T>
void __ParseUIData(CustomData & data, const std::string & key, const std::string & val)
{
    if constexpr (std::is_same_v<T, int>)           { data.emplace(key, S2I(val)); }
    else if constexpr (std::is_same_v<T, bool>)     { data.emplace(key, S2B(val)); }
    else if constexpr (std::is_same_v<T, float>)    { data.emplace(key, S2F(val)); }
    else if constexpr (std::is_same_v<T, std::string>)  { data.emplace(key, (val)); }
    else if constexpr (std::is_same_v<T, glm::vec2>)    { data.emplace(key, S2V2(val)); }
    else if constexpr (std::is_same_v<T, glm::vec3>)    { data.emplace(key, S2V3(val)); }
    else if constexpr (std::is_same_v<T, glm::vec4>)    { data.emplace(key, S2V4(val)); }
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
__REG_GET_UI_DATA(bool, IsVisible, true) //  可见
__REG_SET_UI_DATA(bool, IsVisible)
__REG_GET_UI_DATA(bool, IsShowNav, false) //  有导航栏
__REG_SET_UI_DATA(bool, IsShowNav)
__REG_GET_UI_DATA(bool, IsTextBox, false) //  文本框
__REG_SET_UI_DATA(bool, IsTextBox)
__REG_GET_UI_DATA(bool, IsEditBox, false) //  编辑框
__REG_SET_UI_DATA(bool, IsEditBox)
__REG_GET_UI_DATA(bool, IsCanMove, false) //  可以移动
__REG_SET_UI_DATA(bool, IsCanMove)
__REG_GET_UI_DATA(bool, EnabledKey, false) //  启用键盘
__REG_SET_UI_DATA(bool, EnabledKey)
__REG_GET_UI_DATA(bool, EnabledMouse, false) //  启用鼠标
__REG_SET_UI_DATA(bool, EnabledMouse)
__REG_GET_UI_DATA(bool, IsCanStretch, false) //  可以拉伸
__REG_SET_UI_DATA(bool, IsCanStretch)
__REG_GET_UI_DATA(bool, IsFullScreen, false) //  铺满全屏
__REG_SET_UI_DATA(bool, IsFullScreen)
__REG_GET_UI_DATA(bool, IsShowBorder, true) //  显示边框
__REG_SET_UI_DATA(bool, IsShowBorder)
__REG_GET_UI_DATA(bool, IsShowTitleBar, false) //  有标题栏
__REG_SET_UI_DATA(bool, IsShowTitleBar)
__REG_GET_UI_DATA(bool, IsShowCollapse, false) //  可以收缩
__REG_SET_UI_DATA(bool, IsShowCollapse)
__REG_GET_UI_DATA(bool, IsShowScrollBar, false) //  有滚动条
__REG_SET_UI_DATA(bool, IsShowScrollBar)

__REG_GET_UI_DATA(std::string, Name, std::string()) //  名字
__REG_SET_UI_DATA(std::string, Name)
__REG_GET_UI_DATA(std::string, Tips, std::string()) //  提示
__REG_SET_UI_DATA(std::string, Tips)
__REG_GET_UI_DATA(std::string, Skin, std::string()) //  皮肤
__REG_SET_UI_DATA(std::string, Skin)
__REG_GET_UI_DATA(std::string, Title, std::string()) //  标题
__REG_SET_UI_DATA(std::string, Title)

__REG_GET_UI_DATA(glm::vec4, Move, glm::vec4()) //  方位
__REG_SET_UI_DATA(glm::vec4, Move)
__REG_GET_UI_DATA(glm::vec4, Color, glm::vec4()) //  颜色
__REG_SET_UI_DATA(glm::vec4, Color)
__REG_GET_UI_DATA(glm::vec4, _Move, glm::vec4()) //  方位
__REG_SET_UI_DATA(glm::vec4, _Move)
__REG_GET_UI_DATA(glm::vec4, BgColor, glm::vec4()) //  背景色
__REG_SET_UI_DATA(glm::vec4, BgColor)

__REG_GET_UI_DATA(glm::vec2, StretchMin, glm::vec2(LAYOUT_DRAG_PADDING * 3, LAYOUT_DRAG_PADDING * 3)) //  可拉动最小宽度
__REG_SET_UI_DATA(glm::vec2, StretchMin)

inline void ParseUIData(CustomData & data, const std::string & key, const std::string & val)
{
    __REG_PARSE_UI_DATA(data, key, val, int, Align);
    __REG_PARSE_UI_DATA(data, key, val, float, BorderNumber);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsMulti);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsWindow);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsVisible);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowNav);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsTextBox);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsEditBox);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsCanMove);
    __REG_PARSE_UI_DATA(data, key, val, bool, EnabledKey);
    __REG_PARSE_UI_DATA(data, key, val, bool, EnabledMouse);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsCanStretch);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsFullScreen);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowBorder);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowTitleBar);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowCollapse);
    __REG_PARSE_UI_DATA(data, key, val, bool, IsShowScrollBar);
    __REG_PARSE_UI_DATA(data, key, val, std::string, Name);
    __REG_PARSE_UI_DATA(data, key, val, std::string, Tips);
    __REG_PARSE_UI_DATA(data, key, val, std::string, Skin);
    __REG_PARSE_UI_DATA(data, key, val, std::string, Title);
    __REG_PARSE_UI_DATA(data, key, val, glm::vec2, StretchMin);
    __REG_PARSE_UI_DATA(data, key, val, glm::vec4, Move);
    __REG_PARSE_UI_DATA(data, key, val, glm::vec4, Color);
    __REG_PARSE_UI_DATA(data, key, val, glm::vec4, BgColor);
}

#define GetUIData(data, K)         __GetData##K(data)
#define SetUIData(data, K, val)    __SetData##K(data, val)
