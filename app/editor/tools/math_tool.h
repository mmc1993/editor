#pragma once

namespace math_tool {
    inline bool IsContain(const glm::vec4 & rect, const glm::vec2 & point)
    {
        return rect.x <= point.x
            && rect.y <= point.y
            && rect.x + rect.z >= point.x
            && rect.y + rect.w >= point.y;
    }

    inline int IsHitEdge(const glm::vec4 & rect, const glm::vec2 & point, float border = 0)
    {
        if (std::abs(point.y - rect.y) <= border) return 0;
        if (std::abs(point.y - (rect.y + rect.w)) <= border) return 1;
        if (std::abs(point.x - rect.x) <= border) return 2;
        if (std::abs(point.x - (rect.x + rect.z)) <= border) return 3;
        return -1;
    }
}