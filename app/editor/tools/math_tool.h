#pragma once

namespace glm {
    template <class T>
    T lerp(const T & a, const T & b, float v)
    {
        return a + v * (b - a);
    }
}

namespace tools {
    //  是否包含点
    inline bool IsContain(const glm::vec4 & rect, const glm::vec2 & point)
    {
        return rect.x <= point.x
            && rect.y <= point.y
            && rect.x + rect.z >= point.x
            && rect.y + rect.w >= point.y;
    }

    //  是否在边上
    inline int IsOnRect(const glm::vec4 & rect, const glm::vec2 & point, float border = 0)
    {
        auto t = rect.y, b = rect.y + rect.w;
        auto l = rect.x, r = rect.x + rect.z;
        if (point.x >= l && point.x <= r && std::abs(point.y - rect.y) <= border) return 0;
        if (point.x >= l && point.x <= r && std::abs(point.y - (rect.y + rect.w)) <= border) return 1;
        if (point.y >= t && point.y <= b && std::abs(point.x - rect.x) <= border) return 2;
        if (point.y >= t && point.y <= b && std::abs(point.x - (rect.x + rect.z)) <= border) return 3;
        return -1;
    }

    //  点到四边形距离
    //      返回距离及最接近的边
    inline std::pair<float, int> RectInDistance(const glm::vec4 & rect, const glm::vec2 & point)
    {
        auto t = std::abs(point.y - rect.y);
        auto b = std::abs(point.y - rect.y - rect.w);
        auto l = std::abs(point.x - rect.x);
        auto r = std::abs(point.x - rect.x - rect.z);
        //  矩形水平范围内
        if (point.x >= rect.x && point.x <= rect.x + rect.z) { l = r = 0; }
        //  矩形垂直范围内
        if (point.y >= rect.y && point.y <= rect.y + rect.w) { t = b = 0; }

        auto xmin = std::min(l, r);
        auto ymin = std::min(t, b);
        //  矩形内
        if (xmin == 0 && ymin == 0) { return std::make_pair(0.0f, 0); }
        //  垂直距离
        if (xmin == 0) { return std::make_pair(ymin, ymin == t ? 0 : 1); }
        //  水平距离
        if (ymin == 0) { return std::make_pair(xmin, xmin == l ? 2 : 3); }
        //  返回远的距离
        auto dist = std::max(xmin, ymin);
        return std::make_pair(dist, dist == t ? 0
                                  : dist == b ? 1
                                  : dist == l ? 2 : 3);
    }

    //  浮点数比较
    inline bool Equal(const float f0, const float f1)
    {
        return std::abs(f1 - f0) <= FLT_EPSILON;
    }

    //  点是否在线段上
    inline bool IsOnSegment(const glm::vec2 & p, const glm::vec2 & a, const glm::vec2 & b)
    {
        return p.x >= std::min(a.x, b.x)
            && p.x <= std::max(a.x, b.x)
            && p.y >= std::min(a.y, b.y)
            && p.y <= std::max(a.y, b.y)
            && glm::cross(glm::vec3(p - a, 0), glm::vec3(b - p, 0)).z == 0.0f;
    }

    //  点是否在线段上
    inline bool IsOnSegment(const glm::vec2 & p, const std::vector<glm::vec2> & points)
    {
        auto size = points.size();
        for (auto i = 0; i != size; ++i)
        {
            auto & a = points.at(i             );
            auto & b = points.at((i + 1) % size);
            if (IsOnSegment(p, a, b)) { return true; }
        }
        return false;
    }

    //  直线交点
    inline bool IsCrossLine(const glm::vec2 & a, const glm::vec2 & b, const glm::vec2 & c, const glm::vec2 & d, float * crossA, float * crossB)
    {
        assert(crossA != nullptr);
        assert(crossB != nullptr);
        auto cross = glm::cross(glm::vec3(b - a, 0), glm::vec3(d - c, 0)).z;
        if (cross != 0.0)
        {
            *crossA = glm::cross(glm::vec3(d - c, 0), glm::vec3(a - c, 0)).z / cross;
            *crossB = glm::cross(glm::vec3(b - a, 0), glm::vec3(a - c, 0)).z / cross;
            return true;
        }
        return false;
    }

    //  线段相交
    inline bool IsCrossSegment(const glm::vec2 & a, const glm::vec2 & b, const glm::vec2 & c, const glm::vec2 & d)
    {
        auto ab = glm::vec3(b - a, 0);
        auto cd = glm::vec3(d - c, 0);
        return glm::cross(ab, glm::vec3(c - a, 0)).z * glm::cross(ab, glm::vec3(d - a, 0)).z <= 0
            && glm::cross(cd, glm::vec3(a - c, 0)).z * glm::cross(cd, glm::vec3(b - c, 0)).z <= 0;
    }

    //  线段交点
    inline bool IsCrossSegment(const glm::vec2 & a, const glm::vec2 & b, const glm::vec2 & c, const glm::vec2 & d, float * crossA, float * crossB)
    {
        if (IsCrossLine(a, b, c, d, crossA, crossB))
        {
            return *crossA >= 0.0f && *crossA <= 1.0f
                && *crossB >= 0.0f && *crossB <= 1.0f;
        }
        return false;
    }

    //  点是否在多边形内
    inline bool IsInPolygon(const glm::vec2 & a, const std::vector<glm::vec2> & points)
    {
        glm::vec2 b(std::numeric_limits<float>::max(), a.y);

        auto num  = 0;
        auto size = points.size();
        for (auto i = 0; i != size; ++i)
        {
            auto & c = points.at(i             );
            auto & d = points.at((i + 1) % size);
            if (IsOnSegment(a, c, d)) { return true; }
            if (c.y != d.y)
            {
                if (IsCrossSegment(a, b, c, d))
                {
                    if (c.y < d.y)
                    { if (c.y < a.y) ++num; }
                    else
                    { if (d.y < a.y) ++num; }
                }
            }
        }
        return (num & 1) == 1;
    }

    //  点到线段最短距离
    inline glm::vec2 PointToSegment(const glm::vec2 & p, const glm::vec2 & a, const glm::vec2 & b)
    {
        auto ab = glm::vec3(b - a, 0);
        auto ap = glm::vec3(p - a, 0);
        auto bp = glm::vec3(p - b, 0);
        if      (glm::dot(ap, ab) <= 0) { return -ap; }
        else if (glm::dot(bp, ab) >= 0) { return -bp; }
        else
        {
            auto l = glm::length(ab);
            auto s = glm::dot(ap, ab) / l;
            return s / l * ab;
        }
    }

    //  计算多边形顶点顺序
    inline float CalePointsOrder(const std::vector<glm::vec2> & points)
    {
        ASSERT_LOG(points.size() >= 3, "");
        auto min = 0;
        auto ret = 0.0f;
        for (auto i = 0; i != points.size(); ++i)
        {
            if (points.at(i).x <= points.at(min).x)
            {
                auto prev = (i + points.size() - 1) % points.size();
                auto next = (i + 1) % points.size();
                auto a = glm::vec3(points.at(i) - points.at(prev), 0);
                auto b = glm::vec3(points.at(next) - points.at(i), 0);
                auto z = glm::cross(a, b).z;
                if (z != 0) { ret = z; min = i; }
            }
        }
        return ret;
    }

    //  二分多边形
    inline void BinaryPoints(
        const std::vector<glm::vec2> & points,
        const uint endA, const glm::vec2 & a,
        const uint endB, const glm::vec2 & b,
        std::vector<glm::vec2> output[2])
    {
        output[0].push_back(a);
        if (output[0].back() != b) { output[0].push_back(b); }
        for (auto i = endB; i != endA; i = (i + 1) % points.size())
        {
            if (output[0].back() != points.at(i)) { output[0].push_back(points.at(i)); }
        }

        output[1].push_back(b);
        if (output[1].back() != a) { output[1].push_back(a); }
        for (auto i = endA; i != endB; i = (i + 1) % points.size())
        {
            if (output[1].back() != points.at(i)) { output[1].push_back(points.at(i)); }
        }
    }

    //  切割闭合路径
    inline void StripClosePoints(const std::vector<glm::vec2> & points, std::vector<std::vector<glm::vec2>> & output)
    {
        if (points.size() < 3) { return; }

        static const auto CheckStripPoint = [] (const std::vector<glm::vec2> & points, size_t i)
        {
            float crossA = 0, crossB = 0;
            auto & a = points.at(i);
            auto & b = points.at((i + 1) % points.size());
            for (size_t j = 0; j != i; ++j)
            {
                auto & c = points.at(j);
                auto & d = points.at((j + 1) % points.size());
                if (IsCrossSegment(a, b, c, d, &crossA, &crossB))
                {
                    return std::make_tuple(true,
                        glm::lerp(a, b, crossA),
                        (i + 1) % points.size(),
                        (j + 1) % points.size());
                }
            }
            return std::make_tuple(false, glm::highp_vec2(), (size_t)0, (size_t)0);
        };

        for (auto i = 2; i != points.size(); ++i)
        {
            auto[cross, point, endA, endB] = CheckStripPoint(points, i);
            if (cross)
            {
                std::vector<glm::vec2> binary[2];
                BinaryPoints(points, endB, point, endA, point, binary);
                StripClosePoints(binary[0], output);
                StripClosePoints(binary[1], output);
                return;
            }
        }
        output.push_back(points);
    }

    //  切割闭合路径
    inline std::vector<std::vector<glm::vec2>> StripClosePoints(const std::vector<glm::vec2> & points)
    {
        std::vector<std::vector<glm::vec2>> result;
        StripClosePoints(points, result);
        return std::move(result);
    }

    //  切割凹多边形
    inline void StripCullPoints(const std::vector<glm::vec2> & points, std::vector<std::vector<glm::vec2>> & output)
    {
        static const auto CheckStripPoint = [] (const std::vector<glm::vec2> & points, size_t i)
        {
            float crossA = 0, crossB = 0;
            float minC = std::numeric_limits<float>::max();
            auto minI = std::numeric_limits<size_t>::max();
            auto & a = points.at(i                      );
            auto & b = points.at((i + 1) % points.size());
            for (size_t j = 0; j != points.size(); ++j)
            {
                auto & c = points.at(j                      );
                auto & d = points.at((j + 1) % points.size());
                if (IsCrossLine(a, b, c, d, &crossA, &crossB))
                {
                    if (crossA > 1 && crossA < minC)
                    { minC = crossA; minI = j; }
                }
            }
            ASSERT_LOG(minI != std::numeric_limits<size_t>::max(), "");
            return std::make_tuple(glm::lerp(a, b, minC), (i + 1) % points.size(), (minI + 1) % points.size());
        };

        auto normal = CalePointsOrder(points);
        for (auto i = 0; i != points.size(); ++i)
        {
            auto & a = points.at(i);
            auto & b = points.at((i + 1) % points.size());
            auto & c = points.at((i + 2) % points.size());
            auto ab = glm::vec3(b - a, 0);
            auto bc = glm::vec3(c - b, 0);
            if (glm::cross(ab, bc).z * normal < 0)
            {
                std::vector<glm::vec2> binary[2];
                auto [point, endA, endB] = CheckStripPoint(points,  i);
                BinaryPoints(points, endA, point, endB, point, binary);
                StripCullPoints(binary[0], output);
                StripCullPoints(binary[1], output);
                return;
            }
        }
        output.push_back(points);
    }

    //  切割凸多边形
    inline std::vector<std::vector<glm::vec2>> StripCullPoints(const std::vector<glm::vec2> & points)
    {
        std::vector<std::vector<glm::vec2>> result;
        StripCullPoints(points, result);
        return std::move(result);
    }

    //  顶点去重
    inline std::vector<glm::vec2> UniquePoints(const std::vector<glm::vec2> & points)
    {
        auto result = points;
        std::sort(result.begin(), result.end(), [](auto & a, auto & b) { return a == b; });
        auto it = std::unique(result.begin(), result.end());
        result.erase(it, result.end());
        return std::move(result);
    }
}