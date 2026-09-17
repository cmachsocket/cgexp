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
//  几何变换（★ 这里是你的战场 ★）
//
//  老周："下面 5 个函数现在全都返回单位矩阵——所以机器人被压成了一坨。
//         每写对一个，屏幕上就会发生一次肉眼可见的变化。开工！"
//
//  提示：先调用 mat4::identity() 拿到单位矩阵，再用 r.at(行, 列) 填数。
// ============================================================================

// ====== TODO(明宇) 工单 SD-101-a：平移矩阵 ======
// 把点移动 (tx, ty, tz)：
//
//      | 1  0  0  tx |
//      | 0  1  0  ty |
//      | 0  0  1  tz |
//      | 0  0  0  1  |
//
// 想一想：为什么 3×3 矩阵做不到平移，非要用 4×4 + 齐次坐标？
inline mat4 translate(float tx, float ty, float tz) {
    mat4 r = mat4::identity();
    (void)tx; (void)ty; (void)tz; // TODO: 删掉这行，填入平移分量
    return r;
}

inline mat4 translate(const vec3& t) { return translate(t.x, t.y, t.z); }

// ====== TODO(明宇) 工单 SD-101-b：缩放矩阵 ======
// 沿三个轴分别缩放 (sx, sy, sz)：
//
//      | sx 0  0  0 |
//      | 0  sy 0  0 |
//      | 0  0  sz 0 |
//      | 0  0  0  1 |
inline mat4 scale(float sx, float sy, float sz) {
    mat4 r = mat4::identity();
    (void)sx; (void)sy; (void)sz; // TODO: 删掉这行，填入缩放分量
    return r;
}

// ====== TODO(明宇) 工单 SD-101-c：绕 X 轴旋转 ======
// 旋转 angle 弧度（右手法则：+角度使 +Y 转向 +Z）：
//
//      | 1  0     0    0 |
//      | 0  cos  -sin  0 |
//      | 0  sin   cos  0 |
//      | 0  0     0    1 |
inline mat4 rotateX(float angle) {
    mat4 r = mat4::identity();
    (void)angle; // TODO: c = cos(angle), s = sin(angle)，按上面的矩阵填 4 个数
    return r;
}

// ====== TODO(明宇) 工单 SD-101-d：绕 Y 轴旋转 ======
// 旋转 angle 弧度（右手法则：+角度使 +Z 转向 +X）：
//
//      |  cos  0  sin  0 |
//      |  0    1  0    0 |
//      | -sin  0  cos  0 |
//      |  0    0  0    1 |
//
// 注意：-sin 的位置和 rotateX/rotateZ 不一样！想想为什么（提示：轮换 X→Y→Z→X）
inline mat4 rotateY(float angle) {
    mat4 r = mat4::identity();
    (void)angle; // TODO
    return r;
}

// ====== TODO(明宇) 工单 SD-101-e：绕 Z 轴旋转 ======
// 旋转 angle 弧度（右手法则：+角度使 +X 转向 +Y）：
//
//      | cos  -sin  0  0 |
//      | sin   cos  0  0 |
//      | 0     0    1  0 |
//      | 0     0    0  1 |
inline mat4 rotateZ(float angle) {
    mat4 r = mat4::identity();
    (void)angle; // TODO
    return r;
}

// ============================================================================
//  观察变换（引擎组临时支援版——能用，但别只是用）
//
//  老周："这两个函数是引擎组借你用的。第 3 周他们就要收回去，
//         到时候你得自己写出一模一样的——最好现在就开始琢磨。"
// ============================================================================

// 观察矩阵：把世界变换到"相机坐标系"
//   eye    相机位置
//   center 相机看向的点
//   up     世界上方向（通常 (0,1,0)）
inline mat4 lookAt(const vec3& eye, const vec3& center, const vec3& up) {
    vec3 f = normalize(center - eye);   // 前方向（相机看向 -Z，所以取负号在下面）
    vec3 s = normalize(cross(f, up));   // 右方向
    vec3 u = cross(s, f);               // 修正后的上方向

    mat4 r = mat4::identity();
    r.at(0, 0) = s.x;  r.at(0, 1) = s.y;  r.at(0, 2) = s.z;
    r.at(1, 0) = u.x;  r.at(1, 1) = u.y;  r.at(1, 2) = u.z;
    r.at(2, 0) = -f.x; r.at(2, 1) = -f.y; r.at(2, 2) = -f.z;
    r.at(0, 3) = -dot(s, eye);
    r.at(1, 3) = -dot(u, eye);
    r.at(2, 3) = dot(f, eye);
    return r;
}

// 透视投影矩阵：近大远小
//   fovy   垂直视场角（弧度）
//   aspect 宽高比
//   zNear/zFar 近/远裁剪面距离（均为正数）
inline mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
    float t = 1.0f / std::tan(fovy * 0.5f);
    mat4 r; // 全零
    r.at(0, 0) = t / aspect;
    r.at(1, 1) = t;
    r.at(2, 2) = (zFar + zNear) / (zNear - zFar);
    r.at(2, 3) = 2.0f * zFar * zNear / (zNear - zFar);
    r.at(3, 2) = -1.0f;
    return r;
}

} // namespace mgl
