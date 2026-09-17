// ============================================================================
//  player.h — 《CUBE RUNNER》第 3 周：让小宇走起来
//
//  工单 SD-301：实现 Player::update —— 把 WASD 输入变成世界空间移动
//
//  设计要点（策划案 v0.3）：
//    * "相机相对移动"：按 W 是朝相机面对的方向走，不是朝世界 +Z 走。
//      玩家心里想的是"往前"，而"前"由镜头说了算——3A 大作都这么做。
//    * 角色要平滑转身面向移动方向（插值），瞬间掉头会像抽搐。
//    * 行走时四肢摆动的相位由"累计路程"驱动，而不是时间——
//      这样走快走慢步频自然匹配，停下时腿自动回正。
// ============================================================================
#pragma once

#include "mgl.h"
#include "robot.h"

struct PlayerInput {
    bool forward = false, back = false, left = false, right = false;
    float camYaw = 0.0f; // 相机水平朝向（决定"前"是哪边）
};

class Player {
public:
    Robot robot;
    float walkSpeed = 2.2f; // 米/秒

    void update(const PlayerInput& in, float dt) {
        using namespace mgl;

        // ── 第 1 步：把按键组合成"相机空间"里的期望方向 ──
        //    W=+前，S=-前，D=+右，A=-右
        float fwd = (in.forward ? 1.0f : 0.0f) - (in.back ? 1.0f : 0.0f);
        float side = (in.right ? 1.0f : 0.0f) - (in.left ? 1.0f : 0.0f);

        bool moving = (fwd != 0.0f || side != 0.0f);
        if (moving) {
            // ── 第 2 步：相机空间 → 世界空间 ──
            // 相机水平朝向 camYaw 定义了"前"向量与"右"向量：
            //    front = (sin(camYaw), 0, cos(camYaw))
            //    right = (cos(camYaw), 0, -sin(camYaw))   （front 绕 Y 转 -90°）
            vec3 front{std::sin(in.camYaw), 0.0f, std::cos(in.camYaw)};
            vec3 right{std::cos(in.camYaw), 0.0f, -std::sin(in.camYaw)};

            // 期望移动方向（归一化，斜向走不会变成 √2 倍速）
            vec3 dir = normalize(front * fwd + right * side);

            // ── 第 3 步：位移 = 方向 × 速度 × 时间 ──
            robot.position += dir * (walkSpeed * dt);

            // ── 第 4 步：平滑转身 ──
            // 目标朝向角：atan2 把方向向量变回角度（注意参数顺序 x, z）
            float targetYaw = std::atan2(dir.x, dir.z);
            robot.yaw = smoothTurn(robot.yaw, targetYaw, 12.0f * dt);

            // ── 第 5 步：行走动画（相位由累计路程驱动）──
            distance_ += walkSpeed * dt;
            float phase = distance_ * 5.5f;
            float swing = std::sin(phase);
            robot.legSwingL = swing * radians(30.0f);
            robot.legSwingR = -swing * radians(30.0f);
            robot.armSwingL = -swing * radians(24.0f); // 手臂与腿反相摆动
            robot.armSwingR = swing * radians(24.0f);
        } else {
            // 停下：四肢平滑归零（直接置零会"啪"地立正，很出戏）
            robot.legSwingL = mgl::lerp(robot.legSwingL, 0.0f, 10.0f * dt);
            robot.legSwingR = mgl::lerp(robot.legSwingR, 0.0f, 10.0f * dt);
            robot.armSwingL = mgl::lerp(robot.armSwingL, 0.0f, 10.0f * dt);
            robot.armSwingR = mgl::lerp(robot.armSwingR, 0.0f, 10.0f * dt);
        }

        // 简易边界：别走出方块镇的地面
        const float lim = 18.0f;
        if (robot.position.x > lim) robot.position.x = lim;
        if (robot.position.x < -lim) robot.position.x = -lim;
        if (robot.position.z > lim) robot.position.z = lim;
        if (robot.position.z < -lim) robot.position.z = -lim;
    }

private:
    float distance_ = 0.0f; // 累计走过的路程（驱动步伐相位）

    // 沿"最短方向"平滑转向目标角（处理 ±π 回绕：170° 转到 -170° 应只转 20°）
    static float smoothTurn(float cur, float target, float t) {
        float diff = target - cur;
        while (diff > mgl::PI) diff -= 2.0f * mgl::PI;
        while (diff < -mgl::PI) diff += 2.0f * mgl::PI;
        if (t > 1.0f) t = 1.0f;
        return cur + diff * t;
    }
};
