#pragma once

namespace math_tool {
    inline bool IsContain(const glm::vec4 & rect, const glm::vec2 & point)
    {
        return rect.x <= point.x
            && rect.y <= point.y
            && rect.x + rect.z >= point.x
            && rect.y + rect.w >= point.y;
    }

    inline int IsContain(const glm::vec4 & rect, const glm::vec2 & point, float border = 0)
    {
        //  1, 2, 3, 4 => ио, об, вС, ср
        if (point.y - rect.y <= border) return 1;
        if (point.y - (rect.y + rect.w) >= -border) return 2;
        if (point.x - rect.x <= border) return 3;
        if (point.x - (rect.x + rect.z) >= -border) return 4;
        return 0;
    }
}