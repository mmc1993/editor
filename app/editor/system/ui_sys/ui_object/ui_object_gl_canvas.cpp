#include "ui_object.h"
#include "../../raw_sys/raw.h"
#include "../../raw_sys/raw_sys.h"
#include "../../editor_sys/editor_sys.h"
#include "../../editor_sys/component/gl_object.h"
#include "../../editor_sys/component/component.h"

UIObjectGLCanvas::UIObjectGLCanvas() : UIObject(UITypeEnum::kGLCanvas, new UIStateGLCanvas())
{
    auto state = GetState<UIStateGLCanvas>();
    state->mGLProgramSolidFill = Global::Ref().mRawSys->Get<GLProgram>(tools::GL_PROGRAM_SOLID_FILL);
}

void UIObjectGLCanvas::HandlePostCommands()
{
    auto state = GetState<UIStateGLCanvas>();
    tools::RenderTargetAttachment(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->mRenderTextures[0]);
    tools::RenderTargetAttachment(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->mRenderTextures[1]);
    glBlitFramebuffer(0, 0, (iint)state->Move.z, (iint)state->Move.w, 0, 0, (iint)state->Move.z, (iint)state->Move.w, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    for (auto & command : state->mPostCommands)
    {
        for (auto i = 0; i != command.mProgram->GetPassCount(); ++i)
        {
            if (command.mType == interface::PostCommand::kSwap)
            {
                std::swap(state->mRenderTextures[0], state->mRenderTextures[1]);
            }
            command.mProgram->UsePass(i);
            tools::RenderTargetAttachment(
                GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, state->mRenderTextures[0]);
            command.mProgram->BindUniformTex2D("uniform_screen", state->mRenderTextures[1], 0);
            command.Call();
            Post(command.mProgram, command.mTransform);
            glBindVertexArray(command.mMesh->GetVAO());
            glDrawElements(GL_TRIANGLES, command.mMesh->GetECount(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
}

void UIObjectGLCanvas::HandleFowardCommands()
{
    auto state = GetState<UIStateGLCanvas>();
    tools::RenderTargetAttachment(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_2D, state->mRenderTextures[0]);
    for (auto & command : state->mFowardCommands)
    {
        for (auto i = 0; i != command.mProgram->GetPassCount(); ++i)
        {
            auto texNum = 0;
            command.mProgram->UsePass(i);
            for (auto & texture : command.mTextures)
            {
                command.mProgram->BindUniformTex2D(
                    texture.first.c_str(), texture.second->GetID(), texNum++);
            }
            command.Call();
            Post(command.mProgram, command.mTransform);
            glBindVertexArray(command.mMesh->GetVAO());
            glDrawElements(GL_TRIANGLES, command.mMesh->GetECount(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
}

void UIObjectGLCanvas::CollCommands()
{
    GetState<UIStateGLCanvas>()->mRoot->Update(this, 0.0f);
}

void UIObjectGLCanvas::CallCommands()
{
    auto state = GetState<UIStateGLCanvas>();

    tools::RenderTargetBind(state->mRenderTarget, GL_FRAMEBUFFER);
    tools::RenderTargetAttachment(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_2D, state->mRenderTextures[0]);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    iint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, (iint)state->Move.z, (iint)state->Move.w);

    if (!state->mFowardCommands.empty())
    {
        HandleFowardCommands();
        state->mFowardCommands.clear();
    }
    ASSERT_LOG(glGetError() == 0, "");
    if (!state->mPostCommands.empty())
    {
        HandlePostCommands();
        state->mPostCommands.clear();
    }
    ASSERT_LOG(glGetError() == 0, "");
    if (!state->mOperation.mSelectObjects.empty())
    {
        DrawTrackPoint();
    }
    DrawSelectRect();

    glViewport(0, 0, viewport[2], viewport[3]);
    tools::RenderTargetBind(0, GL_FRAMEBUFFER);
}

void UIObjectGLCanvas::DrawTrackPoint()
{
    auto state = GetState<UIStateGLCanvas>();
    tools::RenderTargetAttachment(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, state->mRenderTextures[0]);
    glLineWidth((float)VAL_TrackPointLine);
    glPointSize((float)VAL_TrackPointSize);
    for (auto & object : state->mOperation.mSelectObjects)
    {
        uint seed = VAL_TrackPointColor;
        for (auto i0 = 0; i0 != object->GetComponents().size(); ++i0)
        {
            std::vector<GLMesh::Vertex> points;
            glm::vec4 color = tools::GetRandomColor(seed);
            auto & component = object->GetComponents().at(i0);
            auto & trackPoints = component->GetTrackPoints();
            for (auto i1 = 0; i1 != trackPoints.size(); ++i1)
            {points.emplace_back(trackPoints.at(i1), color);}

            auto & mesh = GetMeshBuffer(i0);
            mesh->Update(points, {});

            state->mGLProgramSolidFill->UsePass(0);
            Post(  state->mGLProgramSolidFill,
                    object->GetWorldMatrix());
            glBindVertexArray(mesh->GetVAO());
            glDrawArrays(GL_LINE_LOOP, 0, mesh->GetVCount());
            if (HasOpMode(UIStateGLCanvas::Operation::kEdit))
            {
                glBindVertexArray(mesh->GetVAO());
                glDrawArrays(GL_POINTS, 0, mesh->GetVCount());
            }
            glBindVertexArray(0);
        }
    }
}

void UIObjectGLCanvas::DrawSelectRect()
{
    if (HasOpMode(UIStateGLCanvas::Operation::kSelect))
    {
        auto state = GetState<UIStateGLCanvas>();
        auto rect = state->mOperation.mSelectRect;
        if (rect.x > rect.z) std::swap(rect.x, rect.z);
        if (rect.y > rect.w) std::swap(rect.y, rect.w);

        auto min = ProjectWorld(glm::vec2(rect.x, rect.y));
        auto max = ProjectWorld(glm::vec2(rect.z, rect.w));

        //  填充
        std::vector<GLMesh::Vertex> points;
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

        glBindVertexArray(mesh0->GetVAO());
        glDrawArrays(GL_TRIANGLES, 0, mesh0->GetVCount());

        glBindVertexArray(mesh1->GetVAO());
        glDrawArrays(GL_LINE_LOOP, 0, mesh1->GetVCount());

        glBindVertexArray(0);
        glDisable(GL_BLEND);
    }
}

void UIObjectGLCanvas::Post(const interface::PostCommand & cmd)
{
    GetState<UIStateGLCanvas>()->mPostCommands.push_back(cmd);
}

void UIObjectGLCanvas::Post(const interface::FowardCommand & cmd)
{
    GetState<UIStateGLCanvas>()->mFowardCommands.push_back(cmd);
}

void UIObjectGLCanvas::Post(const SharePtr<GLProgram> & program, const glm::mat4 & transform)
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

void UIObjectGLCanvas::OptSelected(const SharePtr<GLObject> & object, bool selected)
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

void UIObjectGLCanvas::OptSelectedClear()
{
    auto state = GetState<UIStateGLCanvas>();
    state->mOperation.mSelectObjects.clear();
    state->mOperation.mActiveObject= nullptr;
    state->mOperation.mActiveComponent = nullptr;
    state->mOperation.mOpMode = UIStateGLCanvas::Operation::kDrag;
}

interface::MatrixStack & UIObjectGLCanvas::GetMatrixStack()
{
    return GetState<UIStateGLCanvas>()->mMatrixStack;
}

void UIObjectGLCanvas::BindRoot(const SharePtr<GLObject>& root)
{
    GetState<UIStateGLCanvas>()->mRoot = root;
}

bool UIObjectGLCanvas::OnEnter()
{
    if (Global::Ref().mEditorSys->IsOpenProject())
    {
        auto state = GetState<UIStateGLCanvas>();
        ASSERT_LOG(state->mRoot != nullptr, "");
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
        ImGui::Image((ImTextureID)state->mRenderTextures[0], 
                      ImVec2(state->Move.z, state->Move.w),
                      ImVec2(0, 1), ImVec2(1, 0));
    }
}

void UIObjectGLCanvas::OnApplyLayout()
{
    auto state = GetState<UIStateGLCanvas>();
    if (state->Move_.z != state->Move.z || state->Move_.w != state->Move.w)
    {
        glBindTexture(GL_TEXTURE_2D, state->mRenderTextures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (iint)state->Move.z, (iint)state->Move.w, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, state->mRenderTextures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (iint)state->Move.z, (iint)state->Move.w, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void UIObjectGLCanvas::OnResetLayout()
{
    OnApplyLayout();
}

inline bool UIObjectGLCanvas::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param)
{
    switch (e)
    {
    case UIEventEnum::kKey:
        OnEventKey((const UIEvent::Key &)param);
        return true;
    case UIEventEnum::kMenu:
        OnEventMenu((const UIEvent::Menu &)param);
        return true;
    case UIEventEnum::kMouse:
        OnEventMouse((const UIEvent::Mouse &)param);
        return true;
    }
    return false;
}

void UIObjectGLCanvas::OnEventKey(const UIEvent::Key & param)
{
}

void UIObjectGLCanvas::OnEventMenu(const UIEvent::Menu & param)
{
}

void UIObjectGLCanvas::OnEventMouse(const UIEvent::Mouse & param)
{
    auto state = GetState<UIStateGLCanvas>();
    //  按下中间拖动舞台
    if (param.mAct == 1 && param.mKey == 2)
    {
        state->mOperation.mCoord.x -= param.mDelta.x;
        state->mOperation.mCoord.y += param.mDelta.y;
    }
    //  滚动鼠标缩放舞台
    if (param.mWheel != 0)
    {
        state->mOperation.mScale = std::clamp(state->mOperation.mScale + (0.2f * param.mWheel), 0.2f, 3.0f);
    }
    //  单击左键选择模式
    if (param.mAct == 3 && param.mKey == 0)
    {
        state->mOperation.mSelectRect.x = param.mMouse.x;
        state->mOperation.mSelectRect.y = param.mMouse.y;
        state->mOperation.mSelectRect.z = param.mMouse.x;
        state->mOperation.mSelectRect.w = param.mMouse.y;
        ModifyOpMode(UIStateGLCanvas::Operation::kSelect, true);
    }
    //  按下左键选择模式
    if (param.mAct == 1 && param.mKey == 0 && HasOpMode(UIStateGLCanvas::Operation::kSelect))
    {
        state->mOperation.mSelectRect.z = param.mMouse.x;
        state->mOperation.mSelectRect.w = param.mMouse.y;
    }
    //  抬起左键结束选择
    if (param.mAct == 2 && param.mKey == 0 && HasOpMode(UIStateGLCanvas::Operation::kSelect))
    {
        ModifyOpMode(UIStateGLCanvas::Operation::kSelect, false);
    }
}

bool UIObjectGLCanvas::HasOpMode(UIStateGLCanvas::Operation::OpModeEnum mode)
{
    auto state = GetState<UIStateGLCanvas>();
    return mode & state->mOperation.mOpMode;
}

glm::mat4 UIObjectGLCanvas::GetMatView()
{
    auto state = GetState<UIStateGLCanvas>();
    auto view = glm::lookAt(state->mOperation.mCoord,
                            state->mOperation.mCoord - glm::vec3(0, 0, 1),glm::vec3(0, 1, 0));
    return glm::scale(view, glm::vec3(state->mOperation.mScale, state->mOperation.mScale, 1));
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

glm::vec2 UIObjectGLCanvas::ProjectWorld(const glm::vec2 & pt)
{
    glm::vec4 port;
    glGetFloatv(GL_VIEWPORT, &port.x);
    const auto & coord = ToLocalCoord(pt);
    auto result = glm::unProject(
        glm::vec3(coord.x, port.w - coord.y, 0),
        glm::mat4(1), GetMatProj(), port);
    return  glm::inverse(GetMatView()) * glm::vec4(result, 1);
}

SharePtr<GLMesh> & UIObjectGLCanvas::GetMeshBuffer(size_t idx)
{
    auto state = GetState<UIStateGLCanvas>();
    if (idx == state->mMeshBuffer.size())
    {
        auto mesh = std::create_ptr<GLMesh>();
        mesh->Init({}, {}, GLMesh::Vertex::kV 
                         | GLMesh::Vertex::kC);
        state->mMeshBuffer.push_back(mesh);
    }
    return state->mMeshBuffer.at(idx);
}

void UIObjectGLCanvas::ModifyOpMode(UIStateGLCanvas::Operation::OpModeEnum op, bool add)
{
    auto state = GetState<UIStateGLCanvas>();
    if (add) state->mOperation.mOpMode |=  op;
    else     state->mOperation.mOpMode &= ~op;
}