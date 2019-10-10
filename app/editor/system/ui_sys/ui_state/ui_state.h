#pragma once

#include "../include.h"

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
    bool IsShowBorder       = true;     //  显示边框
    bool IsCanStretch       = true;     //  可拉伸
    bool IsFullScreen       = false;    //  全屏
    bool IsCanDragMove      = false;    //  可拖放移动
    bool IsCanDragFree      = false;    //  可拖放释放
    bool IsShowMenuBar      = false;    //  显示菜单
    bool IsShowTitleBar     = false;    //  显示标题
    bool IsShowScrollBar    = false;    //  显示滚动条

    float BorderNumber      = 1;        //  边框宽度

    std::string Name;                   //  名字
    std::string Tips;                   //  说明
    std::string LSkin;                  //  皮肤

    glm::vec2 StretchMin;               //  最小尺寸
    glm::vec4 Move;                     //  方位
    glm::vec4 Move_;                    //  上一帧方位

    std::vector<std::string> MenuBar;   //  菜单栏

    std::any UserData;                  //  用户数据
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

    //  拉伸数据
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

class GLMesh;
class GLObject;
class GLProgram;
class GLTexture;
class GLMaterial;

class UIStateGLCanvas : public UIState {
public:
    //  矩阵栈
    enum class MatrixTypeEnum {
        kModel,
        kView,
        kProj,
        Length,
    };

    struct Command {
        std::function<void(const Command&)> mCallback;
        void Call() {if (mCallback) mCallback(*this);}
    };

    struct PreCommand : public Command {
        SharePtr<GLMaterial> mMaterial;     //  材质
        glm::mat4 mTransform;               //  矩阵
    };

    struct PostCommand : public Command {
        enum TypeEnum {
            kOverlay,       //  叠加
            kSwap,          //  交换
        };
        SharePtr<GLProgram> mProgram;       //  着色器
        SharePtr<GLMesh> mMesh;             //  网格
        glm::mat4 mTransform;               //  矩阵
        TypeEnum mType;
    };

public:
    GLuint mRenderTarget;
    GLuint mRenderTextures[2];
    SharePtr<GLObject>  mRoot;
    std::vector<SharePtr<PreCommand>> mPreCommands;
    std::vector<SharePtr<PostCommand>> mPostCommands;
    std::stack<glm::mat4> mMatrixStack[(size_t)MatrixTypeEnum::Length];

public:
    UIStateGLCanvas();
    ~UIStateGLCanvas();
};
