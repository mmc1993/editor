#pragma once

#include "../include.h"
#include "../ui_event.h"
#include "../ui_state/ui_state.h"

class UIObject: public std::enable_shared_from_this<UIObject> {
public:
    UIObject(UITypeEnum type = UITypeEnum::kOther, UIState * state = new UIState())
        : _type(type)
        , _state(state)
        , _parent(nullptr)
    { }

    virtual ~UIObject()
    { }

    template <class T = UIState>
    T * GetState() { return (T *)_state.get(); }
    
    SharePtr<UIObject> GetObject(const std::initializer_list<std::string> & list);
    std::vector<SharePtr<UIObject>>   GetObjects(UITypeEnum type);
    std::vector<SharePtr<UIObject>> & GetObjects();
    void InsertObject(const SharePtr<UIObject> & object);
    void DeleteObject(const SharePtr<UIObject> & object);
    void DeleteObject(size_t index);
    void ClearObjects();
    void DeleteThis();
    SharePtr<UIObject> GetParent();
    SharePtr<UIObject> GetObject();
    UITypeEnum GetType();
    bool IsVisible();

    void ResetLayout();
    void ApplyLayout();
    void Render(float dt, bool visible = true);

    glm::vec4 CalcStretech(DirectEnum direct, const glm::vec2 & offset);
    glm::vec2 ToWorldCoord(const glm::vec2 & coord = glm::vec2(0));
    glm::vec4 ToLocalCoord(const glm::vec4 & coord);
    glm::vec2 ToLocalCoord(const glm::vec2 & coord);
    glm::vec2 ToLocalCoordFromImGUI();
    glm::vec4 ToWorldRect();

    //  绑定事件委托, 事件将被传递到委托中
    void BindDelegate(UIEvent::DelegateHandler * delegate);

protected:
    void RenderDrag();
    void UpdateSize();
    void UpdateMove();
    virtual bool OnEnter();
    virtual void OnLeave(bool ret);
    virtual void OnResetLayout();
    virtual void OnApplyLayout();

    //  事件处理
    void DispatchEventKey();
    SharePtr<UIObject> DispatchEventKey(const UIEvent::Key & param);
    void DispatchEventDrag();
    SharePtr<UIObject> DispatchEventDrag(const UIEvent::Drag & param);
    void DispatchEventMouse();
    SharePtr<UIObject> DispatchEventMouse(const UIEvent::Mouse & param);
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param);

private:
    SharePtr<UIObject> CallEventMessage(UIEventEnum e, const UIEvent::Event & param);
public:
    SharePtr<UIObject> PostEventMessage(UIEventEnum e, const UIEvent::Event & param);

private:
    bool                                _visible;       //  标记当前节点是否渲染
    UITypeEnum                          _type;
    UIObject *                          _parent;
    UniquePtr<UIState>                  _state;
    std::vector<SharePtr<UIObject>>     _children;
    UniquePtr<UIEvent::DelegateHandler> _delegate;
};

class UIObjectLayout : public UIObject {
public:
    UIObjectLayout();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual void OnResetLayout() override;
    virtual void OnApplyLayout() override;

    bool IsCanStretch(DirectEnum edge);
    bool IsCanStretch(DirectEnum edge, const glm::vec2 & offset);
    void HandleStretch();
};

class UIObjectTreeBox : public UIObject {
public:
    UIObjectTreeBox();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
};

class UIObjectTextBox : public UIObject {
public:
    UIObjectTextBox();

private:
    virtual bool OnEnter() override;
};

class UIObjectImageBox : public UIObject {
public:
    UIObjectImageBox();

private:
    virtual bool OnEnter() override;
};

class UIObjectComboBox : public UIObject {
public:
    UIObjectComboBox();

private:
    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param) override;
};

class UIObjectUICanvas : public UIObject {
public:
    UIObjectUICanvas();
};

class UIObjectGLCanvas : public UIObject {
private:
    static const auto VAL_TrackPointLine    = 2;            //  追踪点线宽
    static const auto VAL_TrackPointSize    = 10;           //  追踪点大小
    static const glm::vec4 VAL_TrackPointColors[32];        //  追踪点颜色

public:
    UIObjectGLCanvas();
    interface::MatrixStack & GetMatrixStack();
    void Post(const interface::PostCommand  & cmd);
    void Post(const interface::TargetCommand & cmd);
    void Post(const interface::FowardCommand & cmd);
    void Post(const SharePtr<GLProgram> & program, const glm::mat4 & transform);

    //  交互操作
    void OpEditObject(const SharePtr<GLObject> & object);
    void OpDragSelects(const glm::vec2 & worldBeg, 
                       const glm::vec2 & worldEnd);
    void OpSelected(const SharePtr<GLObject> & object, bool selected);
    void OpSelectedClear();

    glm::vec2 ProjectScreen(const glm::vec2 & world);
    glm::vec2 ProjectWorld(const glm::vec2 & screen);

private:
    void HandleFowardCommands(UIStateGLCanvas::TargetCommand & command);
    void HandlePostCommands(UIStateGLCanvas::TargetCommand & command);
    void CollCommands();
    void CallCommands();

    void DrawTrackPoint();
    void DrawSelectRect();

    virtual bool OnEnter() override;
    virtual void OnLeave(bool ret) override;
    virtual bool OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param) override;

    bool OnEventKey(const UIEvent::Key & param);
    bool OnEventMenu(const UIEvent::Menu & param);
    bool OnEventMouse(const UIEvent::Mouse & param);

    glm::mat4 GetMatView();
    glm::mat4 GetMatProj();
    glm::mat4 GetMatViewProj();
    SharePtr<GLMesh> & GetMeshBuffer(size_t idx);
    bool HasOpMode(UIStateGLCanvas::Operation::OpModeEnum op);
    void AddOpMode(UIStateGLCanvas::Operation::OpModeEnum op, bool add);

    const SharePtr<GLObject> & GetProjectRoot();
    bool FromRectSelectObjects(const glm::vec2 & worldMin, 
                               const glm::vec2 & worldMax);
    void FromRectSelectObjects(const SharePtr<GLObject> & object, 
                               const glm::vec2 & local0, 
                               const glm::vec2 & local1, 
                               const glm::vec2 & local2, 
                               const glm::vec2 & local3, 
                               std::vector<SharePtr<GLObject>> & output);
    SharePtr<GLObject> FromCoordSelectObject(const SharePtr<GLObject> & object, const glm::vec2 & local);
    std::tuple<iint, SharePtr<Component>, glm::vec2, uint> FromCoordSelectTrackPoint(const glm::vec2 & screen);
};