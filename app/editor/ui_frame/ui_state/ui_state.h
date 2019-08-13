#pragma once

#include "../base.h"

class UIState {
public:
    static bool OpEqual(const UIState * state, const std::string & name)
    {
        return state->mName == name;
    }

    //  UI皮肤
    struct Skin {
        enum class TypeEnum {
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
        std::string mTexture;
        glm::vec4 mUV[(size_t)TypeEnum::LENGTH];
    };

    //  对齐方式
    struct Alignment {
        enum class TypeEnum {
            kDEFAULT,               //  绝对坐标
            kSTRETCH,               //  拉伸
            kCENTER,                //  居中
            kLEFT_TOP,              //  左上停靠
            kLEFT_BOTTOM,           //  左下停靠
            kLEFT_VSTRETCH,         //  左停靠, 垂直拉伸
            kLEFT_VCENTER,          //  左停靠, 垂直居中
            kRIGHT_TOP,             //  右上停靠
            kRIGHT_BOTTOM,          //  右下停靠
            kRIGHT_VSTRETCH,        //  右停靠, 垂直拉伸
            kRIGHT_VCENTER,         //  右停靠, 垂直居中
            kHSTRETCH_TOP,          //  上停靠, 水平拉伸
            kHSTRETCH_BOTTOM,       //  下停靠, 水平拉伸
            kHSTRETCH_VSTRETCH,     //  水平拉伸, 垂直拉伸
            kHSTRETCH_VCENTER,      //  垂直居中, 水平拉伸
            kHCENTER_TOP,           //  上停靠, 水平居中
            kHCENTER_BOTTOM,        //  下停靠, 垂直居中
            kHCENTER_VSTRETCH,      //  水平居中, 垂直拉伸
            kHCENTER_VCENTER,       //  水平居中, 垂直居中
            LENGTH,
        };
        glm::vec4 mOrigin;
        TypeEnum    mType;
    };

public:
    UIState()
    { }

    virtual ~UIState()
    { }

    //  基础状态
    size_t          mID;
    Skin            mSkin;
    glm::vec2       mPos;
    glm::vec2       mSize;
    std::string     mName;
    std::string     mTips;
    std::string     mTitle;
    Alignment       mAlignment;
};

class UIStatePanel : public UIState {
public:
    struct Layout {
        struct LinkEdge {
            UIStatePanel * mPanel;
            DirectEnum      mEdge;
        };
        std::vector<UIStatePanel *> mLinks[(size_t)DirectEnum::LENGTH]; //  相连的Panel
        std::vector<LinkEdge>       mEdges[(size_t)DirectEnum::LENGTH]; //  相连的Edge
    };

    Layout mLayout;
};