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

// ====== TODO(明宇) 工单 SD-403：AABB 相交检测 ======
// 一维热身：区间 [a1, a2] 与 [b1, b2] 重叠 ⟺ a1 <= b2 且 a2 >= b1。
// 三维推广：X、Y、Z 三个轴的区间"全部"重叠才算相交（一票否决制）。
// 整个函数一条 return 语句、6 个比较就能写完。
//
// 现在恒返回 false —— 所以小宇是个穿墙幽灵，策划正在骂人。
inline bool aabbIntersect(const AABB& a, const AABB& b) {
    (void)a; (void)b; // TODO: 删掉这行，写出 6 个比较的合取
    return false;
}
