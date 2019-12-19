#include "ui_object.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../raw_sys/comp_transform.h"
#include "../../editor_sys/editor_sys.h"

const glm::vec4 UIObjectGLCanvas::VAL_TrackPointColors[32] = {
    glm::vec4{ 0.60f, 0.85f, 0.91f, 1.0f },
    glm::vec4{ 0.60f, 0.85f, 0.91f, 1.0f },
    glm::vec4{ 0.60f, 0.85f, 0.91f, 1.0f },
    glm::vec4{ 0.60f, 0.85f, 0.91f, 1.0f },
};

UIObjectGLCanvas::UIObjectGLCanvas() : UIObject(UITypeEnum::kGLCanvas, new UIStateGLCanvas())
{
    auto state = GetState<UIStateGLCanvas>();
    state->mRenderTextures[0] = std::create_ptr<RawImage>();
    state->mRenderTextures[0]->InitNull(GL_RGBA);
    state->mRenderTextures[1] = std::create_ptr<RawImage>();
    state->mRenderTextures[1]->InitNull(GL_RGBA);
    state->mGLProgramSolidFill = Global::Ref().mRawSys->Get<RawProgram>(tools::GL_PROGRAM_SOLID_FILL);
}

void UIObjectGLCanvas::HandlePostCommands(UIStateGLCanvas::TargetCommand & command)
{
    std::swap(command.mRenderTextures[0]->mID,
              command.mRenderTextures[1]->mID);
    for (auto & cmd : command.mPostCommands)
    {
        for (auto i = 0; i != cmd.mProgram->GetPassCount(); ++i)
        {
            cmd.mProgram->UsePass(i);
            tools::RenderTargetAttachment(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                          GL_TEXTURE_2D, command.mRenderTextures[0]->mID);
            cmd.mProgram->BindUniformTex2D("uniform_screen", command.mRenderTextures[1]->mID, 0);
            cmd.Call(1                       );
            Post(cmd.mProgram, cmd.mTransform);
            cmd.mMesh->Draw(GL_TRIANGLES     );
            if (cmd.mType == interface::PostCommand::kSwap)
            {
                std::swap(command.mRenderTextures[0]->mID, command.mRenderTextures[1]->mID);
            }
        }
    }
}

void UIObjectGLCanvas::HandleFowardCommands(UIStateGLCanvas::TargetCommand & command)
{
    auto state = GetState();
    for (auto & cmd : command.mFowardCommands)
    {
        for (auto i = 0; i != cmd.mProgram->GetPassCount(); ++i)
        {
            uint texNum = 0;
            cmd.mProgram->UsePass(i);
            for (auto & texture : cmd.mTextures)
            {
                cmd.mProgram->BindUniformTex2D(texture.first.c_str(), texture.second->GetID(), texNum++);
            }
            if (cmd.mEnabled & interface::FowardCommand::kClipView)
            {
                glEnable(GL_SCISSOR_TEST);
                auto matrixVP = GetMatrixStack().GetP() * GetMatrixStack().GetV();
                auto min = ProjectScreen({ cmd.mClipview.x, cmd.mClipview.y });
                auto max = ProjectScreen({ cmd.mClipview.z, cmd.mClipview.w });
                min.y = state->Move.w - min.y;
                max.y = state->Move.w - max.y;
                if (min.x > max.x) { std::swap(min.x, max.x); }
                if (min.y > max.y) { std::swap(min.y, max.y); }
                glScissor(
                    (iint)min.x, (iint)min.y,
                    (iint)max.x - (iint)min.x,
                    (iint)max.y - (iint)min.y);
            }
            Post(cmd.mProgram, cmd.mTransform);
            cmd.Call(texNum);
            cmd.mMesh->Draw(GL_TRIANGLES);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //cmd.mMesh->Draw(GL_TRIANGLES);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if (cmd.mEnabled & interface::FowardCommand::kClipView)
            {
                glDisable(GL_SCISSOR_TEST);
            }
        }
    }
}

void UIObjectGLCanvas::CollCommands()
{
    auto state = GetState<UIStateGLCanvas>();
    auto & command = state->mCommandArray.emplace_back();
    command.mRenderTextures[0] = state->mRenderTextures[0];
    command.mRenderTextures[1] = state->mRenderTextures[1];
    Global::Ref().mEditorSys->GetProject()->GetObject()->Update(this, 0.0f);
}

void UIObjectGLCanvas::CallCommands()
{
    auto state = GetState<UIStateGLCanvas>();
    ASSERT_LOG(state->mCommandStack.empty(), "");
    tools::RenderTargetBind(state->mRenderTarget, GL_FRAMEBUFFER);

    iint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, (iint)state->Move.z, (iint)state->Move.w);

    for (auto iter = state->mCommandArray.rbegin(); 
              iter != state->mCommandArray.rend(); ++iter)
    {
        auto & command = *iter;
        if (command.mRenderTextures[0]->mW != state->Move.z ||
            command.mRenderTextures[0]->mH != state->Move.w)
        {
            command.mRenderTextures[0]->ModifyWH((uint)state->Move.z, (uint)state->Move.w);
        }

        if (command.mRenderTextures[1]->mW != state->Move.z ||
            command.mRenderTextures[1]->mH != state->Move.w)
        {
            command.mRenderTextures[1]->ModifyWH((uint)state->Move.z, (uint)state->Move.w);
        }

        tools::RenderTargetAttachment(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                      GL_TEXTURE_2D, command.mRenderTextures[0]->mID);
        tools::RenderTargetAttachment(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                      GL_TEXTURE_2D, command.mRenderTextures[1]->mID);
        uint buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, buffers);
        glClearColor(0,0,0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        buffers[1]         = GL_NONE;
        glDrawBuffers(2, buffers);

        if (!command.mFowardCommands.empty())
        {
            HandleFowardCommands(command);
        }
        ASSERT_LOG(glGetError() == 0, "");
        if (!command.mPostCommands.empty())
        {
            HandlePostCommands(command);
        }
        ASSERT_LOG(glGetError() == 0, "");
    }

    state->mCommandArray.clear();

    //  绘制选择区
    if (!state->mOperation.mSelectObjects.empty())
    {
        DrawTrackPoint();
    }
    DrawSelectRect();
    glUseProgram(0);

    glViewport(0, 0, viewport[2], viewport[3]);
    tools::RenderTargetBind(0, GL_FRAMEBUFFER);
}

void UIObjectGLCanvas::DrawTrackPoint()
{
    auto state = GetState<UIStateGLCanvas>();
    glLineWidth((float)VAL_TrackPointLine);
    glPointSize((float)VAL_TrackPointSize);
    for (auto & object : state->mOperation.mSelectObjects)
    {
        for (auto i0 = 0; i0 != object->GetComponents().size(); ++i0)
        {
            if (object->GetComponents().at(i0)->HasState(Component::StateEnum::kActive))
            {
                std::vector<RawMesh::Vertex> points;
                auto & component = object->GetComponents().at(i0);
                auto & trackPoints = component->GetTrackPoints();
                for (auto i1 = 0; i1 != trackPoints.size(); ++i1)
                {
                    if (state->mOperation.mEditComponent == component && state->mOperation.mEditTrackPoint == i1)
                    {
                        points.emplace_back(trackPoints.at(i1), VAL_TrackPointColors[i0] * 0.5f);
                    }
                    else
                    {
                        points.emplace_back(trackPoints.at(i1), VAL_TrackPointColors[i0]);
                    }
                }
                
                auto & mesh = GetMeshBuffer(i0);
                mesh->Update(points, {});

                state->mGLProgramSolidFill->UsePass(0);
                Post(state->mGLProgramSolidFill, object->GetWorldMatrix());
                mesh->Draw(GL_LINE_LOOP);
                if (HasOpMode(UIStateGLCanvas::Operation::kEdit))
                {
                    mesh->Draw(GL_POINTS);
                }
            }
        }
    }
}

void UIObjectGLCanvas::DrawSelectRect()
{
    if (HasOpMode(UIStateGLCanvas::Operation::kSelect))
    {
        auto state = GetState<UIStateGLCanvas>();
        auto min = ProjectWorld(ToLocalCoord(glm::vec2(state->mOperation.mSelectRect.x, state->mOperation.mSelectRect.y)));
        auto max = ProjectWorld(ToLocalCoord(glm::vec2(state->mOperation.mSelectRect.z, state->mOperation.mSelectRect.w)));
        if (min.x > max.x) std::swap(min.x, max.x);
        if (min.y > max.y) std::swap(min.y, max.y);

        //  填充
        std::vector<RawMesh::Vertex> points;
        points.emplace_back(glm::vec2(min.x, min.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.2f));
        points.emplace_back(glm::vec2(max.x, min.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.2f));
        points.emplace_back(glm::vec2(max.x, max.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.2f));
        points.emplace_back(glm::vec2(min.x, min.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.2f));
        points.emplace_back(glm::vec2(max.x, max.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.2f));
        points.emplace_back(glm::vec2(min.x, max.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.2f));
        auto mesh0 = GetMeshBuffer(0);
        mesh0->Update(points, {});

        //  描边
        points.clear();
        points.emplace_back(glm::vec2(min.x, min.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.5f));
        points.emplace_back(glm::vec2(max.x, min.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.5f));
        points.emplace_back(glm::vec2(max.x, max.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.5f));
        points.emplace_back(glm::vec2(min.x, max.y), glm::vec4(0.7f, 0.9f, 0.1f, 0.5f));
        auto mesh1 = GetMeshBuffer(1);
        mesh1->Update(points, {});

        state->mGLProgramSolidFill->UsePass(0);
        glEnable(GL_BLEND);
        Post(state->mGLProgramSolidFill, glm::mat4(1));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mesh0->Draw(GL_TRIANGLES);
        mesh1->Draw(GL_LINE_LOOP);
        glDisable(GL_BLEND);

        FromRectSelectObjects(min, max);
    }
}

void UIObjectGLCanvas::Post(const interface::PostCommand & cmd)
{
    if (auto state = GetState<UIStateGLCanvas>(); state->mCommandStack.empty())
    {
        state->mCommandArray.front().mPostCommands.emplace_back(cmd);
    }
    else
    {
        state->mCommandStack.top().mPostCommands.emplace_back(cmd);
    }
}

void UIObjectGLCanvas::Post(const::interface::TargetCommand & cmd)
{
    auto state = GetState<UIStateGLCanvas>();
    if (cmd.mType == interface::TargetCommand::kPush)
    {
        auto & target = state->mCommandStack.emplace();
        target.mRenderTextures[0] = cmd.mTexture;
        target.mRenderTextures[1] = state->mRenderTextures[1];
    }
    else if (cmd.mType == interface::TargetCommand::kPop)
    {
        auto && top = std::move(state->mCommandStack.top());
        state->mCommandArray.emplace_back(std::move(top));
        state->mCommandStack.pop();
    }
}

void UIObjectGLCanvas::Post(const interface::FowardCommand & cmd)
{
    if (auto state = GetState<UIStateGLCanvas>(); state->mCommandStack.empty())
    {
        state->mCommandArray.front().mFowardCommands.emplace_back(cmd);
    }
    else
    {
        state->mCommandStack.top().mFowardCommands.emplace_back(cmd);
    }
}

void UIObjectGLCanvas::Post(const SharePtr<RawProgram> & program, const glm::mat4 & transform)
{
    auto state = GetState<UIStateGLCanvas>();
    const auto & matrixV = GetMatrixStack().Top(interface::MatrixStack::kView);
    const auto & matrixP = GetMatrixStack().Top(interface::MatrixStack::kProj);
    const auto & matrixMV = matrixV * transform;
    const auto & matrixVP = matrixP * matrixV;
    const auto & matrixMVP = matrixP * matrixMV;

    //  矩阵
    program->BindUniformMatrix("uniform_matrix_m", transform);
    program->BindUniformMatrix("uniform_matrix_v", matrixV);
    program->BindUniformMatrix("uniform_matrix_p", matrixP);
    program->BindUniformMatrix("uniform_matrix_mv", matrixMV);
    program->BindUniformMatrix("uniform_matrix_vp", matrixVP);
    program->BindUniformMatrix("uniform_matrix_mvp", matrixMVP);

    //  逆矩阵
    program->BindUniformMatrix("uniform_matrix_v_inv", glm::inverse(matrixV));
    program->BindUniformMatrix("uniform_matrix_p_inv", glm::inverse(matrixP));
    program->BindUniformMatrix("uniform_matrix_mv_inv", glm::inverse(matrixMV));
    program->BindUniformMatrix("uniform_matrix_vp_inv", glm::inverse(matrixVP));

    //  其他参数
    program->BindUniformNumber("uniform_game_time", glfwGetTime());
}

void UIObjectGLCanvas::OpEditObject(const SharePtr<GLObject> & object)
{
    auto state = GetState<UIStateGLCanvas>();
    if (object != nullptr)
    {
        AddOpMode(UIStateGLCanvas::Operation::kEdit, true);
        state->mOperation.mEditObject = object;
    }
    else
    {
        AddOpMode(UIStateGLCanvas::Operation::kEdit, false);
        state->mOperation.mEditObject       = nullptr;
        state->mOperation.mEditComponent    = nullptr;
        state->mOperation.mEditTrackPoint   = std::numeric_limits<uint>::max();
    }
}

void UIObjectGLCanvas::OpDragSelects(const glm::vec2 & worldBeg, const glm::vec2 & worldEnd)
{ 
    static const auto IsSkip = [] (
        const SharePtr<GLObject>              & object, 
        const std::vector<SharePtr<GLObject>> & objects)
    {
        for (auto parent = object->GetParent(); parent != nullptr; parent = parent->GetParent())
        {
            if (std::find(objects.begin(), objects.end(), parent) != objects.end())
            {
                return true;
            }
        }
        return false;
    };

    auto state = GetState<UIStateGLCanvas>();
    for (auto & object : state->mOperation.mSelectObjects)
    {
        if (!IsSkip(object, state->mOperation.mSelectObjects))
        {
            auto a = object->GetParent()->WorldToLocal(worldBeg);
            auto b = object->GetParent()->WorldToLocal(worldEnd);
            auto ab = b - a;
            object->GetTransform()->AddPosition(ab.x, ab.y);
        }
    }
}

void UIObjectGLCanvas::OpSelected(const SharePtr<GLObject> & object, bool selected)
{
    auto state = GetState<UIStateGLCanvas>();
    auto it = std::find(state->mOperation.mSelectObjects.begin(),
                        state->mOperation.mSelectObjects.end(), object);
    if (selected)
    {
        if (it == state->mOperation.mSelectObjects.end())
        {
            state->mOperation.mSelectObjects.push_back(object);
        }
    }
    else
    {
        if (it != state->mOperation.mSelectObjects.end())
        {
            state->mOperation.mSelectObjects.erase(it);
        }
    }
}

void UIObjectGLCanvas::OpSelectedClear()
{
    auto state = GetState<UIStateGLCanvas>();
    state->mOperation.mSelectObjects.clear();

    state->mOperation.mOpMode = 0;
    state->mOperation.mEditObject = nullptr;
    state->mOperation.mEditComponent = nullptr;
}

glm::vec2 UIObjectGLCanvas::ProjectScreen(const glm::vec2 & world)
{
    glm::vec4 viewPort(0, 0, GetState()->Move.z, GetState()->Move.w);
    auto coord = glm::project(glm::vec3(world, 0),
            GetMatView(), GetMatProj(), viewPort);
    return  glm::vec2(coord.x, GetState()->Move.w - coord.y);
}

glm::vec2 UIObjectGLCanvas::ProjectWorld(const glm::vec2 & screen)
{
    glm::vec3 coord(screen.x, GetState()->Move.w - screen.y, 0);
    glm::vec4 viewPort(0, 0, GetState()->Move.z, GetState()->Move.w);
    auto result = glm::unProject(coord, GetMatView(), GetMatProj(), viewPort);
    return result;
}

interface::MatrixStack & UIObjectGLCanvas::GetMatrixStack()
{
    return GetState<UIStateGLCanvas>()->mMatrixStack;
}

bool UIObjectGLCanvas::OnEnter()
{
    if (Global::Ref().mEditorSys->IsOpenProject())
    {
        auto state = GetState<UIStateGLCanvas>();
        state->mMatrixStack.Identity(interface::MatrixStack::kModel);
        state->mMatrixStack.Identity(interface::MatrixStack::kView, GetMatView());
        state->mMatrixStack.Identity(interface::MatrixStack::kProj, GetMatProj());
        CollCommands();
        CallCommands();
        state->mMatrixStack.Pop(interface::MatrixStack::kModel);
        state->mMatrixStack.Pop(interface::MatrixStack::kView);
        state->mMatrixStack.Pop(interface::MatrixStack::kProj);
        return true;
    }
    return false;
}

void UIObjectGLCanvas::OnLeave(bool ret)
{
    if (ret)
    {
        auto state = GetState<UIStateGLCanvas>();
        ImGui::Image((ImTextureID)state->mRenderTextures[0]->mID, 
                      ImVec2(state->Move.z, state->Move.w),
                      ImVec2(0, 1), ImVec2(1, 0));
    }
}

bool UIObjectGLCanvas::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param)
{
    if (Global::Ref().mEditorSys->IsOpenProject())
    {
        switch (e)
        {
        case UIEventEnum::kKey:
            return OnEventKey((const UIEvent::Key &)param);
        case UIEventEnum::kMenu:
            return OnEventMenu((const UIEvent::Menu &)param);
        case UIEventEnum::kMouse:
            return OnEventMouse((const UIEvent::Mouse &)param);
        }
    }
    return false;
}

bool UIObjectGLCanvas::OnEventKey(const UIEvent::Key & param)
{
    auto ready = false;
    auto state = GetState<UIStateGLCanvas>();
    if (param.mState == 0 && param.mAct == 2 && param.mKey == GLFW_KEY_ENTER)
    {
        if (HasOpMode(UIStateGLCanvas::Operation::kEdit))
        {
            OpEditObject(nullptr);
        }
        else if (state->mOperation.mSelectObjects.size() == 1)
        {
            OpEditObject(state->mOperation.mSelectObjects.front());
        }
        ready = true;
    }
    return ready;
}

bool UIObjectGLCanvas::OnEventMenu(const UIEvent::Menu & param)
{
    return false;
}

bool UIObjectGLCanvas::OnEventMouse(const UIEvent::Mouse & param)
{
    auto state = GetState<UIStateGLCanvas>();
    //  按下中间拖动舞台
    if (param.mAct == 1 && param.mKey == 2)
    {
        state->mOperation.mViewCoord.x -= param.mDelta.x;
        state->mOperation.mViewCoord.y += param.mDelta.y;
    }
    //  滚动鼠标缩放舞台
    if (param.mWheel != 0)
    {
        auto origin = ProjectWorld(ToLocalCoord(param.mMouse));
        auto oldS   = state->mOperation.mViewScale;
        state->mOperation.mViewScale = std::clamp(state->mOperation.mViewScale + (0.2f * param.mWheel), 0.2f, 10.0f);
        auto newS   = state->mOperation.mViewScale;
        auto target = ProjectWorld(ToLocalCoord(param.mMouse));
        state->mOperation.mViewCoord.x += (origin.x - target.x) * newS;
        state->mOperation.mViewCoord.y += (origin.y - target.y) * newS;
    }

    //  非编辑模式
    if (!HasOpMode(UIStateGLCanvas::Operation::kEdit))
    {
        //  单击左键选中对象
        if (!HasOpMode(UIStateGLCanvas::Operation::kSelect) && param.mAct == 3 && param.mKey == 0)
        {
            state->mOperation.mSelectRect.x = param.mMouse.x;
            state->mOperation.mSelectRect.y = param.mMouse.y;
            state->mOperation.mSelectRect.z = param.mMouse.x;
            state->mOperation.mSelectRect.w = param.mMouse.y;
            const auto & coord = ProjectWorld(ToLocalCoord(param.mMouse));
            if (FromRectSelectObjects(coord, coord))
            {
                AddOpMode(UIStateGLCanvas::Operation::kDrag, true);
            }
            else
            {
                AddOpMode(UIStateGLCanvas::Operation::kSelect, true);
            }
        }
        //  按下左键选择对象
        if (HasOpMode(UIStateGLCanvas::Operation::kSelect) && param.mAct == 1 && param.mKey == 0)
        {
            state->mOperation.mSelectRect.z = param.mMouse.x;
            state->mOperation.mSelectRect.w = param.mMouse.y;
        }
        //  抬起左键结束选择
        if (HasOpMode(UIStateGLCanvas::Operation::kSelect) && param.mAct == 2 && param.mKey == 0)
        {
            AddOpMode(UIStateGLCanvas::Operation::kSelect, false);
        }
        //   按住左键拖动对象
        if (HasOpMode(UIStateGLCanvas::Operation::kDrag) && param.mAct == 1 && param.mKey == 0)
        {
            auto prev = ProjectWorld(ToLocalCoord(param.mMouse - param.mDelta));
            auto curr = ProjectWorld(ToLocalCoord(param.mMouse               ));
            OpDragSelects(prev, curr);
        }
        //  抬起左键结束拖拽
        if (HasOpMode(UIStateGLCanvas::Operation::kDrag) && param.mAct == 2 && param.mKey == 0)
        {
            AddOpMode(UIStateGLCanvas::Operation::kDrag, false);
        }
    }

    //  编辑模式
    if (HasOpMode(UIStateGLCanvas::Operation::kEdit))
    {
        ASSERT_LOG(state->mOperation.mEditObject != nullptr, "");
        //  左键双击新增顶点
        if (param.mAct == 4 && param.mKey == 0)
        {
            auto[num, comp, coord, index] = FromCoordSelectTrackPoint(ToLocalCoord(param.mMouse));
            if (num == 0 || num == 2)
            {
                comp->InsertTrackPoint(index, coord);
            }
        }
        //  左键单击选中顶点
        if (param.mAct == 3 && param.mKey == 0)
        {
            auto[num, comp, coord, index] = FromCoordSelectTrackPoint(ToLocalCoord(param.mMouse));
            state->mOperation.mEditTrackPoint = num == 1? index: (uint)-1;
            state->mOperation.mEditComponent = num == 1? comp: nullptr;
        }
        //  左键按下拖动顶点
        if (param.mAct == 1 && param.mKey == 0 && state->mOperation.mEditComponent)
        {
            auto & comp = state->mOperation.mEditComponent;
            auto index = state->mOperation.mEditTrackPoint;
            ASSERT_LOG(index < comp->GetTrackPoints().size(), "");
            auto prev = comp->GetOwner()->WorldToLocal(ProjectWorld(ToLocalCoord(param.mMouse - param.mDelta)));
            auto curr = comp->GetOwner()->WorldToLocal(ProjectWorld(ToLocalCoord(param.mMouse               )));
            comp->ModifyTrackPoint(index, comp->GetTrackPoints().at(index) + (curr - prev));
        }
    }
    return true;
}

glm::mat4 UIObjectGLCanvas::GetMatView()
{
    auto state = GetState<UIStateGLCanvas>();
    auto view = glm::lookAt(state->mOperation.mViewCoord,
                            state->mOperation.mViewCoord - glm::vec3(0, 0, 1),glm::vec3(0, 1, 0));
    return glm::scale(view, glm::vec3(state->mOperation.mViewScale, state->mOperation.mViewScale, 1));
}

glm::mat4 UIObjectGLCanvas::GetMatProj()
{
    auto state = GetState<UIStateGLCanvas>();
    return glm::ortho(state->Move.z * -0.5f, state->Move.z * 0.5f, 
                      state->Move.w * -0.5f, state->Move.w * 0.5f);
}

glm::mat4 UIObjectGLCanvas::GetMatViewProj()
{
    return GetMatProj() * GetMatView();
}

SharePtr<RawMesh> & UIObjectGLCanvas::GetMeshBuffer(size_t idx)
{
    auto state = GetState<UIStateGLCanvas>();
    if (idx == state->mMeshBuffer.size())
    {
        auto mesh = std::create_ptr<RawMesh>();
        mesh->Init({}, {}, RawMesh::Vertex::kV 
                         | RawMesh::Vertex::kC);
        state->mMeshBuffer.push_back(mesh);
    }
    return state->mMeshBuffer.at(idx);
}

bool UIObjectGLCanvas::HasOpMode(UIStateGLCanvas::Operation::OpModeEnum op)
{
    return op & GetState<UIStateGLCanvas>()->mOperation.mOpMode;
}

void UIObjectGLCanvas::AddOpMode(UIStateGLCanvas::Operation::OpModeEnum op, bool add)
{
    auto state = GetState<UIStateGLCanvas>();
    if (add) state->mOperation.mOpMode |=  op;
    else     state->mOperation.mOpMode &= ~op;
}

const SharePtr<GLObject>& UIObjectGLCanvas::GetProjectRoot()
{
    ASSERT_LOG(Global::Ref().mEditorSys->IsOpenProject(), "");
    return Global::Ref().mEditorSys->GetProject()->GetObject();
}

bool UIObjectGLCanvas::FromRectSelectObjects(const glm::vec2 & worldMin, const glm::vec2 & worldMax)
{
    auto state = GetState<UIStateGLCanvas>();
    if (worldMin == worldMax)
    {
        if (auto hit = FromCoordSelectObject(GetProjectRoot(), GetProjectRoot()->ParentToLocal(worldMin)))
        {
            auto ret = std::find(state->mOperation.mSelectObjects.begin(), 
                                 state->mOperation.mSelectObjects.end(), hit);
            if (ret == state->mOperation.mSelectObjects.end())
            {
                Global::Ref().mEditorSys->OptSelectObject(hit, true);
            }
            return true;    //  命中了一个Object
        }
        else
        {
            Global::Ref().mEditorSys->OptSelectObject(hit, true);
        }
    }
    else
    {
        std::vector<SharePtr<GLObject>> output;
        auto pt0 = GetProjectRoot()->ParentToLocal(worldMin);
        auto pt1 = GetProjectRoot()->ParentToLocal(glm::vec2(worldMax.x, worldMin.y));
        auto pt2 = GetProjectRoot()->ParentToLocal(worldMax);
        auto pt3 = GetProjectRoot()->ParentToLocal(glm::vec2(worldMin.x, worldMax.y));
        FromRectSelectObjects(GetProjectRoot(), pt0, pt1, pt2, pt3, output);
        auto NoExists = [&output] (const auto & object) 
        {
            return output.end() == std::find(output.begin(), output.end(), object);
        };
        std::vector<SharePtr<GLObject>> unlocks;
        std::copy_if(state->mOperation.mSelectObjects.begin(),
                     state->mOperation.mSelectObjects.end(),
                     std::back_inserter(unlocks), NoExists);
        for (auto & object : unlocks)
        {
            Global::Ref().mEditorSys->OptSelectObject(object, false);
        }
        for (auto & object : output)
        {
            Global::Ref().mEditorSys->OptSelectObject(object, true, true);
        }
    }
    return false;
}

void UIObjectGLCanvas::FromRectSelectObjects(
    const SharePtr<GLObject> & object, 
    const glm::vec2 & local0, 
    const glm::vec2 & local1, 
    const glm::vec2 & local2, 
    const glm::vec2 & local3, 
    std::vector<SharePtr<GLObject>> & output)
{
    std::vector<glm::vec2> points(4);
    auto pred = [&points] (const auto & com)
    { 
        auto it = std::find_if(com->GetTrackPoints().begin(), com->GetTrackPoints().end(),
            [&] (const auto & point) { return tools::IsContainsConvex(points, point); });
        return com->HasState(Component::StateEnum::kActive)
            && it != com->GetTrackPoints().end();
    };

    for (auto & children : object->GetObjects())
    {
        if (children->HasState(GLObject::StateEnum::kActive))
        {
            points.at(0) = children->ParentToLocal(local0);
            points.at(1) = children->ParentToLocal(local1);
            points.at(2) = children->ParentToLocal(local2);
            points.at(3) = children->ParentToLocal(local3);
            if (!children->HasState(GLObject::StateEnum::kLocked))
            {
                auto ret = std::find_if(
                    children->GetComponents().begin(),
                    children->GetComponents().end(), pred);
                if (children->GetComponents().end() != ret)
                {
                    output.push_back(children);
                }
            }
            FromRectSelectObjects(children, points.at(0), points.at(1), points.at(2), points.at(3), output);
        }
    }
}

SharePtr<GLObject> UIObjectGLCanvas::FromCoordSelectObject(const SharePtr<GLObject> & object, const glm::vec2 & local)
{
    auto thit = local;
    auto pred = [&thit] (const auto & com)
    { 
        return com->HasState(Component::StateEnum::kActive) &&
               tools::IsContains(com->GetTrackPoints(), thit);
    };
    for (auto it = object->GetObjects().rbegin(); it != object->GetObjects().rend(); ++it)
    {
        if ((*it)->HasState(GLObject::StateEnum::kActive))
        {
            thit = (*it)->ParentToLocal(local);

            if (auto ret = FromCoordSelectObject(*it, thit))
            {
                return ret;
            }
            if (!(*it)->HasState(GLObject::StateEnum::kLocked))
            {
                auto ret = std::find_if(
                    (*it)->GetComponents().begin(), 
                    (*it)->GetComponents().end(), pred);
                if (ret != (*it)->GetComponents().end()) { return *it; }
            }
        }
    }
    return nullptr;
}

std::tuple<iint, SharePtr<Component>, glm::vec2, uint> UIObjectGLCanvas::FromCoordSelectTrackPoint(const glm::vec2 & screen)
{
    //  返回值:
    //      没有击中追踪点 -1
    //      击中追踪点中心  1
    //      击中追踪点之间  2
    //      击中追踪点附近  0
    auto state = GetState<UIStateGLCanvas>();
    ASSERT_LOG(state->mOperation.mEditObject != nullptr, "");

    const auto & comps = state->mOperation.mEditObject->GetComponents();
    for (auto it = comps.rbegin(); it != comps.rend(); ++it)
    {
        //  优先判断是否击中顶点
        for (auto i = 0u; i != (*it)->GetTrackPoints().size(); ++i)
        {
            const auto coord = ProjectScreen(state->mOperation.mEditObject->LocalToWorld((*it)->GetTrackPoints().at(i)));

            if (tools::IsCantains(coord, (float)VAL_TrackPointSize, screen))
            {
                return std::make_tuple(1, *it, state->mOperation.mEditObject->WorldToLocal(ProjectWorld(screen)), (uint)i);
            }
            if (2 * VAL_TrackPointSize * VAL_TrackPointSize >= glm::length_sqrt(screen - coord))
            {
                return std::make_tuple(0, *it, state->mOperation.mEditObject->WorldToLocal(ProjectWorld(screen)), (uint)i + 1);
            }
        }
        //  其次判断是否击中线段
        for (auto i = 0u; i != (*it)->GetTrackPoints().size(); ++i)
        {
            auto j = (i + 1) % (*it)->GetTrackPoints().size();
            const auto a = ProjectScreen(state->mOperation.mEditObject->LocalToWorld((*it)->GetTrackPoints().at(i)));
            const auto b = ProjectScreen(state->mOperation.mEditObject->LocalToWorld((*it)->GetTrackPoints().at(j)));
            if (VAL_TrackPointSize * VAL_TrackPointSize >= glm::length_sqrt(tools::PointToSegment(screen, a, b)))
            {
                return std::make_tuple(2, *it, state->mOperation.mEditObject->WorldToLocal(ProjectWorld(screen)), (uint)j);
            }
        }
    }
    return std::make_tuple(-1, nullptr, glm::vec2(), (uint)-1);
}
