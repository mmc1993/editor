#pragma once

namespace math_tool {
    //  是否包含点
    inline bool IsContain(const glm::vec4 & rect, const glm::vec2 & point)
    {
        return rect.x <= point.x
            && rect.y <= point.y
            && rect.x + rect.z >= point.x
            && rect.y + rect.w >= point.y;
    }

    //  是否在边上
    inline int IsOnEdge(const glm::vec4 & rect, const glm::vec2 & point, float border = 0)
    {
        if (std::abs(point.y - rect.y) <= border) return 0;
        if (std::abs(point.y - (rect.y + rect.w)) <= border) return 1;
        if (std::abs(point.x - rect.x) <= border) return 2;
        if (std::abs(point.x - (rect.x + rect.z)) <= border) return 3;
        return -1;
    }

    //  浮点数比较
    inline bool Equal(const float f0, const float f1)
    {
        return std::abs(f1 - f0) <= FLT_EPSILON;
    }
}