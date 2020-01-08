#pragma once

namespace glm {
    template <class T>
    T lerp(const T & a, const T & b, float v)
    {
        return a + v * (b - a);
    }

    template <class T>
    float length_sqrt(const T & vec)
    {
        return glm::dot(vec, vec);
    }
}

namespace tools {
    //  是否包含点
    inline bool IsContains(const glm::vec4 & rect, const glm::vec2 & point)
    {
        return rect.x <= point.x
            && rect.y <= point.y
            && rect.x + rect.z >= point.x
            && rect.y + rect.w >= point.y;
    }
    
    //  圆是否包含点
    inline bool IsContains(const glm::vec2 & center, const float radius, const glm::vec2 & point)
    {
        auto diff = point - center;
        return glm::dot(diff, diff) <= radius * radius;
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
    inline std::pair<float, int> PointToRectEdge(const glm::vec4 & rect, const glm::vec2 & point)
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

    inline bool Equal(const glm::vec2 & v0, const glm::vec2 & v1)
    {
        return Equal(v0.x, v1.x)
            && Equal(v0.y, v1.y);
    }

    //  点是否在线段上
    inline bool IsOnSegment(const glm::vec2 & p, const glm::vec2 & a, const glm::vec2 & b)
    {
        return p.x >= std::min(a.x, b.x)
            && p.x <= std::max(a.x, b.x)
            && p.y >= std::min(a.y, b.y)
            && p.y <= std::max(a.y, b.y)
            && glm::cross(p - a, b - p) == 0.0f;
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
        auto cross = glm::cross(b - a, d - c);
        if (cross != 0.0)
        {
            *crossA = glm::cross(d - c, a - c) / cross;
            *crossB = glm::cross(b - a, a - c) / cross;
            return true;
        }
        return false;
    }

    //  线段相交
    inline bool IsCrossSegment(const glm::vec2 & a, const glm::vec2 & b, const glm::vec2 & c, const glm::vec2 & d)
    {
        auto ab = b - a;
        auto cd = d - c;
        return glm::cross(ab, c - a) * glm::cross(ab, d - a) <= 0
            && glm::cross(cd, a - c) * glm::cross(cd, b - c) <= 0;
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

    //  线段与多边形相交
    inline bool IsCrossSegment(
        const glm::vec2 & a,
        const glm::vec2 & b,
        const std::vector<glm::vec2> & points,
        const std::function<bool(uint, uint, float, float)> & callback)
    {
        bool ret = false;
        auto crossA = 0.0f;
        auto crossB = 0.0f;
        auto size = points.size();
        for (auto i = 0; i != size; ++i)
        {
            auto j = (i + 1) % size;
            auto & c = points.at(i);
            auto & d = points.at(j);
            if (IsCrossSegment(a, b, c, d, &crossA, &crossB) && (ret = true))
            {
                if (!callback(i, j, crossA, crossB)) {break;}
            }
        }
        return ret;
    }

    //  线段与多边形相交-仅相交
    inline bool IsCrossSegment(const glm::vec2 & a, const glm::vec2 & b, const std::vector<glm::vec2> & points)
    {
        return IsCrossSegment(a, b, points, [] (uint _aIndex, uint _bIndex, float _crossA, float _crossB)
            {
                return false;
            });
    }

    //  线段与多边形相交-第一交点
    inline bool IsCrossSegment(
        const glm::vec2 & a,
        const glm::vec2 & b,
        const std::vector<glm::vec2> & points,
        uint  * aIndex, uint  * bIndex,
        float * crossA, float * crossB)
    {
        return IsCrossSegment(a, b, points, [&aIndex, &bIndex, &crossA, &crossB] (uint _aIndex, uint _bIndex, float _crossA, float _crossB)
            {
                *aIndex = _aIndex; *bIndex = _bIndex;
                *crossA = _crossA; *crossB = _crossB;
                return false;
            });
    }

    //  线段与多边形相交-全部交点
    inline bool IsCrossSegment(
        const glm::vec2 & a,
        const glm::vec2 & b,
        const std::vector<glm::vec2> & points,
        std::vector<std::tuple<uint, uint, float, float>> * output)
    {
        return IsCrossSegment(a, b, points, [&output] (uint _aIndex, uint _bIndex, float _crossA, float _crossB)
            {
                output->emplace_back(_aIndex, _bIndex,
                                     _crossB, _crossA);
                return true;
            });
    }

    //  多边形与多边形相交
    inline bool IsCrossSegment(const std::vector<glm::vec2> & points0, const std::vector<glm::vec2> & points1)
    {
        auto size = points0.size();
        for (auto i = 0; i != size; ++i)
        {
            auto j = (i + 1) % size;
            auto & a = points0.at(i);
            auto & b = points0.at(j);
            if (IsCrossSegment(a, b, points1))
            {
                return true;
            }
        }
        return false;
    }

    //  线段与多边形相交(穿过)
    inline bool IsCrossSegmentPast(const glm::vec2 & a, const glm::vec2 & b, const std::vector<glm::vec2> & points)
    {
        auto crossA = 0.0f, crossB = 0.0f;
        for (auto i = 0; i != points.size(); ++i)
        {
            auto & c = points.at(i                      );
            auto & d = points.at((i + 1) % points.size());
            if (IsCrossSegment(a, b, c, d, &crossA, &crossB)
                && crossA != 0.0f && crossA != 1.0f)
            {
                return true;
            }
        }
        return false;
    }

    //  点是否在多边形内
    inline bool IsContains(const std::vector<glm::vec2> & points, const glm::vec2 & a)
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

    //  点是否在凸多边形内
    inline bool IsContainsConvex(const std::vector<glm::vec2> & points, const glm::vec2 & p)
    {
        ASSERT_LOG(points.size() > 2, "");
        for (auto i = 0; i != points.size(); ++i)
        {
            auto & a = points.at(i                      );
            auto & b = points.at((i + 1) % points.size());
            auto & c = points.at((i + 2) % points.size());
            if (glm::cross(b - a, p - a) * glm::cross(c - b, p - c) <= 0)
            {
                return false;
            }
        }
        return true;
    }

    //  点到线段最短距离
    inline glm::vec2 PointToSegment(const glm::vec2 & p, const glm::vec2 & a, const glm::vec2 & b)
    {
        auto ab = b - a, ap = p - a, bp = p - b;
        if      (glm::dot(ap, ab) <= 0) { return -ap; }
        else if (glm::dot(bp, ab) >= 0) { return -bp; }
        else
        {
            auto l = 1 / glm::length(ab);
            auto s = l * glm::dot(ap,ab);
            return l * s * ab - ap;
        }
    }

    //  计算顶点集中心
    inline glm::vec2 CalePointsCenter(const std::vector<glm::vec2> & points)
    {
        glm::vec2 center(0.0f,0.0f);
        for (auto & point : points)
        {
            center += point;
        }
        return center / (float)points.size();
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
                auto a = points.at(i) - points.at(prev);
                auto b = points.at(next) - points.at(i);
                auto z = glm::cross(a, b);
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
    inline void StripConvexPoints(const std::vector<glm::vec2> & points, std::vector<std::vector<glm::vec2>> & output)
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
                    if (crossB >=0 && crossB <= 1 &&
                        crossA > 1 && crossA < minC)
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
            if (glm::cross(b - a, c - b) * normal < 0)
            {
                std::vector<glm::vec2> binary[2];
                auto [point, endA, endB] = CheckStripPoint(points,  i);
                BinaryPoints(points, endA, point, endB, point, binary);
                StripConvexPoints(binary[0], output);
                StripConvexPoints(binary[1], output);
                return;
            }
        }
        output.push_back(points);
    }

    //  切割凸多边形
    inline std::vector<std::vector<glm::vec2>> StripConvexPoints(const std::vector<glm::vec2> & points)
    {
        std::vector<std::vector<glm::vec2>> result;
        StripConvexPoints(points, result);
        return std::move(result);
    }

    //  切割三角形
    inline std::vector<glm::vec2> StripTrianglePoints(const std::vector<glm::vec2> & points)
    {
        std::vector<glm::vec2> result;
        for (auto i = 1; i != points.size() - 1; ++i)
        {
            result.emplace_back(points.at(0));
            result.emplace_back(points.at(i));
            result.emplace_back(points.at(i + 1));
        }
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

    //  判断多边形是否存在闭环
    inline bool IsExistClosePath(const std::vector<glm::vec2> & points)
    {
        if (points.size() <= 4)
        {
            return false;
        }
        for (auto i0 = 2; i0 != points.size(); ++i0)
        {
            auto i1 = (i0 + 1) % points.size();
            for (auto j0 = 0; j0 != i0-1; ++j0)
            {
                auto j1 = (j0 + 1) % points.size();
                if (i1 != j0 && IsCrossSegment(
                    points.at(i0), points.at(i1), 
                    points.at(j0), points.at(j1)))
                {
                    return true;
                }
            }
        }
        return false;
    }

    //  生成圆角
    inline std::vector<glm::vec2> GenRounding(const glm::vec2 & dir0, const glm::vec2 & dir1, float radius, float smooth)
    {
        std::vector<glm::vec2> result;
        auto v0 = glm::normalize(dir0) * smooth;
        auto v1 = glm::normalize(dir1) * smooth;
        auto count = glm::length(v1 - v0);
        if ((uint)count >= 2)
        {
            auto step = (v1 - v0) / count;
            for (auto i = 1; i != (uint)count; ++i)
            {
                result.emplace_back(glm::normalize(v0 += step) * radius);
            }
        }
        return std::move(result);
    }

    //  生成多边形外环
    inline std::vector<glm::vec2> GenOuterRing(const std::vector<glm::vec2> & points, float border, float smooth = 1)
    {
        glm::vec3 zAxis(0, 0, 1);
        std::vector<glm::vec2> result;
        ASSERT_LOG(!IsExistClosePath(points), "");
        auto order = CalePointsOrder(points);
        for (auto i = 0; i != points.size(); ++i)
        {
            auto & a = points.at(i);
            auto & b = points.at((i + 1) % points.size());
            auto & c = points.at((i + 2) % points.size());
            auto ab0 = glm::vec2(glm::normalize(glm::cross(glm::vec3(b - a, 0), zAxis)) * border);
            auto bc0 = glm::vec2(glm::normalize(glm::cross(glm::vec3(c - b, 0), zAxis)) * border);
            if (order < 0) { ab0 = -ab0;  bc0 = -bc0; }

            result.push_back(a);
            result.push_back(a + ab0);

            result.push_back(b);
            result.push_back(b + ab0);

            for (auto & p : GenRounding(ab0, bc0, border, smooth))
            {
                result.push_back(b + p);
            }
            result.push_back(a);
        }
        return std::move(result);
    }

    //  生成凸包
    inline std::vector<glm::vec2> GenConvexPoints(std::vector<glm::vec2> points)
    {
        if (points.size() < 3) { return points; }

        const auto min = *std::min_element(points.begin(), points.end(), 
            [] (const auto & a, const auto & b) { return a.y < b.y; });

        std::sort(points.begin(), points.end(), 
            [&min] (const auto & a, const auto & b)
            { 
                auto mina = a - min;
                auto minb = b - min;
                return std::atan2(mina.y, mina.x) < std::atan2(minb.y, minb.x);
            });

        std::vector<glm::vec2> result;
        result.push_back(points.at(0));
        result.push_back(points.at(1));
        result.push_back(points.at(2));
        for (auto i = 3; i != points.size(); ++i)
        {
            auto a = std::next(result.rbegin(), 1);
            auto b = std::next(result.rbegin(), 0);
            while (0 > glm::cross(*b - *a, points.at(i) - *b))
            {
                result.pop_back();
                a = std::next(result.rbegin(), 1);
                b = std::next(result.rbegin(), 0);
            }
            result.push_back(points.at(i));
        }
        return std::move(result);
    }

    //  随机数
    inline float Random(float min, float max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return std::uniform_real_distribution<float>(min, max)(gen);
    }
}