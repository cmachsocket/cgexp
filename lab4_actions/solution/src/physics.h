// ============================================================================
//  physics.h — 《CUBE RUNNER》第 4 周：星穹互娱"物理引擎"（自封的）
//
//  工单 SD-403：实现 aabbIntersect —— 两个轴对齐包围盒是否相交
//
//  AABB（Axis-Aligned Bounding Box，轴对齐包围盒）是最便宜的碰撞体：
//  两个 AABB 相交，当且仅当它们在 X、Y、Z 三个轴上的区间都重叠。
//  一个轴不重叠就等于隔着一堵看不见的墙——直接 false。
// ============================================================================
#pragma once

#include "mgl.h"

struct AABB {
    mgl::vec3 min; // 最小角
    mgl::vec3 max; // 最大角

    // 用中心点 + 尺寸构造（size 是完整边长，不是半边长）
    static AABB fromCenter(const mgl::vec3& center, const mgl::vec3& size) {
        mgl::vec3 h = size * 0.5f;
        return {center - h, center + h};
    }
};

// 两个 AABB 是否相交：三个轴的区间都必须重叠（分离轴定理的最简形式）
inline bool aabbIntersect(const AABB& a, const AABB& b) {
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y &&
           a.min.z <= b.max.z && a.max.z >= b.min.z;
}
