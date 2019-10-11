#pragma once

#include "../../include.h"

class GLMesh;
class GLObject;
class GLProgram;
class GLTexture;
class GLMaterial;

namespace Interface {
    struct RenderCommand {
        std::function<void(const RenderCommand &)> mCallback;
        void Call() { if (mCallback) mCallback(*this); }
    };

    struct FowardCommand : public RenderCommand {
        SharePtr<GLMaterial> mMaterial;     //  材质
        glm::mat4 mTransform;               //  矩阵
    };

    struct PostCommand : public RenderCommand {
        enum TypeEnum {
            kOverlay,       //  叠加
            kSwap,          //  交换
        };
        SharePtr<GLProgram> mProgram;       //  着色器
        SharePtr<GLMesh> mMesh;             //  网格
        glm::mat4 mTransform;               //  矩阵
        TypeEnum mType;
    };
}