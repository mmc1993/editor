#pragma once

#include "../include.h"
#include "../../interface/render.h"
#include "../../interface/serializer.h"

class UIState {
public:
    virtual ~UIState()
    { }

    bool FromStringParse(const std::string & key, const std::string & val);

    int Align               = 0;        //  对齐方式
    
    bool IsMulti            = false;    //  多行
    bool IsSelect           = false;    //  选中
    bool IsWindow           = false;    //  窗口
    bool IsButton           = false;    //  按钮
    bool IsShowNav          = false;    //  导航
    bool IsEditBox          = false;    //  编辑
    bool IsCanMove          = false;    //  可移动
    bool IsSameline         = false;    //  不换行
    bool IsShowBorder       = true;     //  显示边框
    bool IsCanStretch       = true;     //  可拉伸
    bool IsFullScreen       = false;    //  全屏
    bool IsEditReturn       = false;    //  编辑按回车
    bool IsCanDragMove      = false;    //  可拖放移动
    bool IsCanDragFree      = false;    //  可拖放释放
    bool IsShowMenuBar      = false;    //  显示菜单
    bool IsShowTitleBar     = false;    //  显示标题
    bool IsShowScrollBar    = false;    //  显示滚动条

    float BorderNumber      = 1;        //  边框宽度

    std::string Name;                   //  名字
    std::string Tips;                   //  说明
    std::string LSkin;                  //  皮肤
    std::string LSkin_;                  // 皮肤

    glm::vec2 StretchMin;               //  最小尺寸
    glm::vec4 Move;                     //  方位
    glm::vec4 Move_;                    //  方位
    glm::vec4 Color = glm::vec4(1);     //  颜色

    std::vector<std::string> MenuBar;   //  菜单栏

    //  无需解析
    bool IsFirstRender      = true;     //  是否第一次渲染
};

class UIStateLayout : public UIState {
public:
    //  拖放信息
    struct Drag {
        DirectEnum mDirect;
        glm::vec2 mBegWorld;
        glm::vec2 mEndWorld;
        UIObject * mDragObj;
        UIObject * mFreeObj;
    } mDrag;

    //  链接信息
    struct Join {
        std::vector<UIObject *>           mOut;
        std::pair<UIObject *, DirectEnum> mIn;
        Join() {}
    } mJoin[(size_t)DirectEnum::Length];

    //  鼠标焦点
    struct MouseFocus {
        WeakPtr<UIObject> mFocus;
        WeakPtr<UIObject> mDown;
        int               mKey;
    } mMouseFocus;

    //  拉伸焦点
    struct StretchFocus {
        UIObject *  mObject;
        DirectEnum  mDirect;
        StretchFocus() : mObject(nullptr) {}
    } mStretchFocus;

    UIStateLayout();
};

class UIStateTreeBox : public UIState {
public:
    UIStateTreeBox();
};

class UIStateTextBox : public UIState {
public:
    std::string mBuffer;

    UIStateTextBox();
};

class UIStateImageBox : public UIState {
public:
    SharePtr<GLTexture> mSkinTex;

    UIStateImageBox();
};

class UIStateComboBox : public UIState {
public:
    std::string mSelected;

    UIStateComboBox();
};

class UIStateUICanvas : public UIState {
public:
    UIStateUICanvas();
};

class UIStateGLCanvas : public UIState {
public:
    struct TargetCommand {
        std::vector<interface::PostCommand>     mPostCommands;      //  后期
        std::vector<interface::FowardCommand>   mFowardCommands;    //  正向
        SharePtr<GLImage>                       mRenderTextures[2];
    };

public:
    GLuint mRenderTarget;
    SharePtr<GLImage> mRenderTextures[2];
    interface::MatrixStack    mMatrixStack;
    std::stack<TargetCommand>  mCommandStack;
    std::vector<TargetCommand> mCommandArray;

    //  编辑功能相关
    struct Operation {
        //  操作模式
        enum OpModeEnum {
            kDrag = 0x1,        //  拖拽
            kEdit = 0x2,        //  编辑
            kSelect = 0x4,      //  选择
        };

        //  激活的对象
        SharePtr<GLObject>  mEditObject;
        SharePtr<Component> mEditComponent;
        size_t              mEditTrackPoint;

        glm::vec4                       mSelectRect;
        std::vector<SharePtr<GLObject>> mSelectObjects;

        size_t      mOpMode;
        float       mViewScale;
        glm::vec3   mViewCoord;

        //  初始化
        Operation(): mOpMode(0), mViewScale(1)
        { }
    } mOperation;

    std::vector<SharePtr<GLMesh>> mMeshBuffer;
    SharePtr<GLProgram>   mGLProgramSolidFill;

public:
    UIStateGLCanvas();
    ~UIStateGLCanvas();
};