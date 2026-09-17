// ============================================================================
//  camera.h — 《CUBE RUNNER》第 3 周：镜头组的活儿现在归你了
//
//  工单 SD-302：实现 mgl::lookAt 与 mgl::perspective（去 mgl.h 里写！
//               引擎组的临时支援版已经收回，那两个函数现在是空壳。）
//  工单 SD-303：实现第三人称跟随相机 FollowCamera::update
//
//  第三人称相机的本质是"球面坐标"：
//    相机吊在一根看不见的自拍杆上，杆的另一端绑在角色头顶。
//    yaw（水平角）与 pitch（俯仰角）决定杆的方向，distance 是杆长：
//
//      offset.x = d · cos(pitch) · sin(yaw)
//      offset.y = d · sin(pitch)
//      offset.z = d · cos(pitch) · cos(yaw)
//      eye = target + offset          （相机在角色斜后上方）
// ============================================================================
#pragma once

#include "mgl.h"

class FollowCamera {
public:
    float yaw = 0.0f;                     // 水平环绕角
    float pitch = mgl::radians(20.0f);    // 俯仰角（正 = 从上往下看）
    float distance = 5.0f;                // 自拍杆长度

    mgl::vec3 eye;    // 更新后：相机位置
    mgl::vec3 target; // 更新后：注视点

    // mouseDX/mouseDY: 本帧鼠标位移；focus: 角色脚底位置
    void update(const mgl::vec3& focus, float mouseDX, float mouseDY) {
        using namespace mgl;

        // 鼠标 → 角度（灵敏度 0.003 弧度/像素）
        yaw -= mouseDX * 0.003f;
        pitch += mouseDY * 0.003f;

        // 俯仰角限位：不许翻到角色头顶正上方或钻到地底
        const float maxPitch = radians(65.0f), minPitch = radians(-15.0f);
        if (pitch > maxPitch) pitch = maxPitch;
        if (pitch < minPitch) pitch = minPitch;

        // 注视点：角色胸口（脚底往上 1.1 米），镜头不该盯着脚
        target = focus + vec3{0.0f, 1.1f, 0.0f};

        // 球面坐标 → 直角坐标（见文件头的公式）
        vec3 offset{distance * std::cos(pitch) * std::sin(yaw),
                    distance * std::sin(pitch),
                    distance * std::cos(pitch) * std::cos(yaw)};
        eye = target + offset;

        // 防穿地
        if (eye.y < 0.2f) eye.y = 0.2f;
    }

    mgl::mat4 viewMatrix() const {
        return mgl::lookAt(eye, target, {0.0f, 1.0f, 0.0f});
    }

    // 玩家"往前"的方向 = 相机到角色的水平朝向（Player 移动要用）
    // 相机在角色后方 yaw 处，所以前进方向是 yaw + π
    float forwardYaw() const { return yaw + mgl::PI; }
};

// ----------------------------------------------------------------------------
// 自由飞行相机（调试神器，按 C 切换）。
// 引擎组写好送你的——对照它理解 lookAt 的 eye/center/up 三件套。
// ----------------------------------------------------------------------------
class FreeCamera {
public:
    mgl::vec3 position{0.0f, 5.0f, 12.0f};
    float yaw = mgl::PI; // 初始朝 -Z（面向镇中心）
    float pitch = mgl::radians(-15.0f);

    void update(float mouseDX, float mouseDY,
                bool w, bool s, bool a, bool d, bool up, bool down, float dt) {
        using namespace mgl;
        yaw -= mouseDX * 0.003f;
        pitch -= mouseDY * 0.003f;
        const float lim = radians(89.0f);
        if (pitch > lim) pitch = lim;
        if (pitch < -lim) pitch = -lim;

        vec3 front{std::cos(pitch) * std::sin(yaw), std::sin(pitch),
                   std::cos(pitch) * std::cos(yaw)};
        vec3 right{std::sin(yaw - PI / 2.0f), 0.0f, std::cos(yaw - PI / 2.0f)};

        const float speed = 8.0f;
        if (w) position += front * (speed * dt);
        if (s) position += front * (-speed * dt);
        if (d) position += right * (speed * dt);
        if (a) position += right * (-speed * dt);
        if (up) position += vec3{0, 1, 0} * (speed * dt);
        if (down) position += vec3{0, 1, 0} * (-speed * dt);
    }

    mgl::mat4 viewMatrix() const {
        using namespace mgl;
        vec3 front{std::cos(pitch) * std::sin(yaw), std::sin(pitch),
                   std::cos(pitch) * std::cos(yaw)};
        return lookAt(position, position + front, {0.0f, 1.0f, 0.0f});
    }
};
