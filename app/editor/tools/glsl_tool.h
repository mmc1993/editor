#pragma once

namespace tools {
    template <class T>
    constexpr uint UBOTypeSize()
    {
        if constexpr (std::is_same<std::remove_cv_t<T>, glm::mat4>::value)
        {
            return 64;
        }
        else if constexpr (std::is_same<std::remove_cv_t<T>, glm::vec4>::value ||
                           std::is_same<std::remove_cv_t<T>, glm::vec3>::value)
        {
            return 16;
        }
        else if constexpr (std::is_same<std::remove_cv_t<T>, float>::value ||
                           std::is_same<std::remove_cv_t<T>, iint>::value ||
                           std::is_same<std::remove_cv_t<T>, uint>::value ||
                           std::is_same<std::remove_cv_t<T>, int>::value)
        {
            return 4;
        }
        else
        {
            static_assert(false);
        }
    }

    template <class T>
    constexpr uint UBOTypeAlig()
    {
        return sizeof(T) > 16 ? 16
            : sizeof(T) > 8 ? 16
            : sizeof(T) > 4 ? 8
            : 4;
    }

    //  返回 std140 布局, 对齐后的偏移量
    template <class T>
    constexpr uint UBOOffsetBase(uint base)
    {
        constexpr auto alig = UBOTypeAlig<T>();
        base = (base + alig - 1) / alig * alig;
        return base;
    }

    //  返回 std 140 布局, 填充后的偏移量
    template <class T>
    constexpr uint UBOOffsetFill(uint base)
    {
        constexpr auto size = UBOTypeSize<T>();
        return UBOOffsetBase<T>(base) + size;
    }

    template <class T>
    constexpr uint UBOAddData(uint base, const T & v)
    {
        base = UBOOffsetBase<T>(base);
        glBufferSubData(GL_UNIFORM_BUFFER, base, sizeof(T), &v);
        return UBOOffsetFill<T>(base);
    }

    //  FBO
    inline void RenderTargetBind(uint rt, uint type)
    {
        ASSERT_LOG((rt != 0) || glCheckFramebufferStatus(type) == GL_FRAMEBUFFER_COMPLETE, "");
        glBindFramebuffer(type, rt);
    }

    inline void RenderTargetAttachment(uint type, uint attachment, uint textureType, uint texture)
    {
        glFramebufferTexture2D(type, attachment, textureType, texture, 0);
    }

    //  预置着色器
    const char * const GL_PROGRAM_SOLID_FILL = "res/program/solid_fill.program";
    const char * const GL_PROGRAM_LIGHTNING  = "res/program/lightning.program";
    const char * const GL_PROGRAM_TILEMAP    = "res/program/tilemap.program";
    const char * const GL_PROGRAM_SEGMENT    = "res/program/segment.program";
    const char * const GL_PROGRAM_SPRITE     = "res/program/sprite.program";
    const char * const GL_PROGRAM_LIGHT      = "res/program/light.program";

}