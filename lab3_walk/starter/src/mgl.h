// ============================================================================
//  mgl.h — 明宇图形库（MingYu Graphics Library）
//
//  老周："引擎组不许你用 GLM。矩阵这东西，自己推一遍才算真的会。
//         这个文件就是你的地盘，出了 bug 也全是你的。"
//
//  约定（与 OpenGL 一致）：
//    * 右手坐标系：+X 向右，+Y 向上，+Z 指向屏幕外
//    * 矩阵为 4×4 列主序（column-major）：m[列*4 + 行]
//    * 顶点是列向量，变换写作  v' = M · v
//    * 组合顺序：M = M_平移 · M_旋转 · M_缩放  （先缩放，再旋转，最后平移）
// ============================================================================
#pragma once

#include <cmath>

namespace mgl {

constexpr float PI = 3.14159265358979f;

inline float radians(float deg) { return deg * PI / 180.0f; }

// ----------------------------------------------------------------------------
// 三维向量
// ----------------------------------------------------------------------------
struct vec3 {
    float x = 0, y = 0, z = 0;

    vec3() = default;
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    vec3 operator+(const vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    vec3 operator-(const vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    vec3 operator-() const { return {-x, -y, -z}; }
    vec3& operator+=(const vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

inline float dot(const vec3& a, const vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 cross(const vec3& a, const vec3& b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

inline float length(const vec3& v) { return std::sqrt(dot(v, v)); }

inline vec3 normalize(const vec3& v) {
    float len = length(v);
    return len > 1e-8f ? v * (1.0f / len) : vec3{0, 0, 0};
}

// 线性插值：t=0 得 a，t=1 得 b
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// ----------------------------------------------------------------------------
// 4×4 矩阵（列主序）：m[列*4 + 行]
// ----------------------------------------------------------------------------
struct mat4 {
    float m[16] = {};

    // 单位矩阵：对角线为 1，其余为 0——"什么都不做"的变换
    static mat4 identity() {
        mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    float& at(int row, int col) { return m[col * 4 + row]; }
    float at(int row, int col) const { return m[col * 4 + row]; }

    const float* data() const { return m; }

    // 矩阵乘法：C = A · B（先施加 B，再施加 A）
    mat4 operator*(const mat4& b) const {
        mat4 r;
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                    sum += at(row, k) * b.at(k, col);
                r.at(row, col) = sum;
            }
        return r;
    }

    // 矩阵 × 向量（w=1 的点）：v' = M · v
    vec3 transformPoint(const vec3& v) const {
        return {at(0,0)*v.x + at(0,1)*v.y + at(0,2)*v.z + at(0,3),
                at(1,0)*v.x + at(1,1)*v.y + at(1,2)*v.z + at(1,3),
                at(2,0)*v.x + at(2,1)*v.y + at(2,2)*v.z + at(2,3)};
    }
};

// ============================================================================
//  几何变换（工单 SD-101：第 1 周由明宇实现 ✔ 已完成）
// ============================================================================

// 平移矩阵：把点移动 (tx, ty, tz)
//
//      | 1  0  0  tx |
//      | 0  1  0  ty |
//      | 0  0  1  tz |
//      | 0  0  0  1  |
inline mat4 translate(float tx, float ty, float tz) {
    mat4 r = mat4::identity();
    r.at(0, 3) = tx;
    r.at(1, 3) = ty;
    r.at(2, 3) = tz;
    return r;
}

inline mat4 translate(const vec3& t) { return translate(t.x, t.y, t.z); }

// 缩放矩阵：沿三个轴分别缩放 (sx, sy, sz)
//
//      | sx 0  0  0 |
//      | 0  sy 0  0 |
//      | 0  0  sz 0 |
//      | 0  0  0  1 |
inline mat4 scale(float sx, float sy, float sz) {
    mat4 r = mat4::identity();
    r.at(0, 0) = sx;
    r.at(1, 1) = sy;
    r.at(2, 2) = sz;
    return r;
}

// 绕 X 轴旋转 angle 弧度（右手法则：+角度使 +Y 转向 +Z）
//
//      | 1  0     0    0 |
//      | 0  cos  -sin  0 |
//      | 0  sin   cos  0 |
//      | 0  0     0    1 |
inline mat4 rotateX(float angle) {
    mat4 r = mat4::identity();
    float c = std::cos(angle), s = std::sin(angle);
    r.at(1, 1) = c;  r.at(1, 2) = -s;
    r.at(2, 1) = s;  r.at(2, 2) = c;
    return r;
}

// 绕 Y 轴旋转 angle 弧度（右手法则：+角度使 +Z 转向 +X）
//
//      |  cos  0  sin  0 |
//      |  0    1  0    0 |
//      | -sin  0  cos  0 |
//      |  0    0  0    1 |
inline mat4 rotateY(float angle) {
    mat4 r = mat4::identity();
    float c = std::cos(angle), s = std::sin(angle);
    r.at(0, 0) = c;   r.at(0, 2) = s;
    r.at(2, 0) = -s;  r.at(2, 2) = c;
    return r;
}

// 绕 Z 轴旋转 angle 弧度（右手法则：+角度使 +X 转向 +Y）
//
//      | cos  -sin  0  0 |
//      | sin   cos  0  0 |
//      | 0     0    1  0 |
//      | 0     0    0  1 |
inline mat4 rotateZ(float angle) {
    mat4 r = mat4::identity();
    float c = std::cos(angle), s = std::sin(angle);
    r.at(0, 0) = c;  r.at(0, 1) = -s;
    r.at(1, 0) = s;  r.at(1, 1) = c;
    return r;
}

// ============================================================================
//  观察变换（★ 第 3 周主战场 ★）
//
//  老周（转发引擎组邮件）："如通知所述，lookAt / perspective 临时支援版
//  已于本周一收回。收到工单的同学请尽快自行实现，否则你的世界会
//  永远卡在原点动弹不得。——引擎组"
//
//  （明宇内心：说好的支援呢？？）
// ============================================================================

// ====== TODO(明宇) 工单 SD-302-a：观察矩阵 lookAt ======
// 把世界变换到"相机坐标系"：相机在原点、看向 -Z、+Y 朝上。
//   eye    相机位置
//   center 相机看向的点
//   up     世界上方向（通常 (0,1,0)）
//
// 推导分两步（讲义 3.2 节有完整过程）：
//
//   第 1 步：构造相机的三根轴（都要归一化）
//     f = normalize(center - eye)      前方向
//     s = normalize(cross(f, up))      右方向（前 × 上）
//     u = cross(s, f)                  真正的上方向（右 × 前，已正交无需归一化）
//
//   第 2 步：填矩阵 = 旋转（把 s/u/-f 转到 x/y/z 轴）+ 平移（把 eye 挪到原点）
//
//     | s.x   s.y   s.z   -dot(s, eye) |
//     | u.x   u.y   u.z   -dot(u, eye) |
//     | -f.x  -f.y  -f.z   dot(f, eye) |
//     | 0     0     0      1           |
//
// 为什么第三行是 -f？因为 OpenGL 相机看向自己的 -Z 方向。
inline mat4 lookAt(const vec3& eye, const vec3& center, const vec3& up) {
    mat4 r = mat4::identity();
    (void)eye; (void)center; (void)up; // TODO: 删掉这行，按上面两步实现
    return r;
}

// ====== TODO(明宇) 工单 SD-302-b：透视投影矩阵 perspective ======
// 实现"近大远小"：把视锥体压成标准立方体（NDC）。
//   fovy   垂直视场角（弧度）
//   aspect 宽高比 = 宽/高
//   zNear/zFar 近/远裁剪面距离（均为正数）
//
// 记 t = 1 / tan(fovy / 2)，矩阵为（注意：从全零矩阵开始，不是单位矩阵！）：
//
//     | t/aspect  0    0                            0                          |
//     | 0         t    0                            0                          |
//     | 0         0    (zFar+zNear)/(zNear-zFar)    2·zFar·zNear/(zNear-zFar)  |
//     | 0         0    -1                           0                          |
//
// 最下面那个 -1 是整个透视效果的灵魂：它把 -z（深度）塞进 w，
// 硬件做完 xyz÷w 后，远处的东西就自然变小了。讲义 3.4 节有完整推导。
inline mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
    mat4 r; // 全零矩阵（注意这里故意不是 identity！）
    (void)fovy; (void)aspect; (void)zNear; (void)zFar; // TODO: 按上表填 5 个数
    return r;
}

} // namespace mgl
