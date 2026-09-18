// ============================================================================
//  player.h — 《CUBE RUNNER》第 4 周：注入灵魂——跑、跳与动作状态机
//
//  工单 SD-401：动画状态机 Animator::update（Idle/Walk/Run/Jump 四态四肢驱动）
//  工单 SD-402：跳跃运动学 —— 起跳速度 + 重力积分（v += g·dt; y += v·dt）
//  工单 SD-403：AABB 碰撞（physics.h），撞墙时把位移"顶回去"
//
//  策划案 v1.0（Game Jam 交付版）：
//    * SHIFT 冲刺：速度 ×2，步频加快、摆幅加大、身体略前倾
//    * SPACE 跳跃：只有落地状态能起跳；空中保留水平操控（游戏手感！）
//    * 跳跃全程：起跳蹬腿 → 空中收腿摆臂 → 落地屈膝缓冲
// ============================================================================
#pragma once

#include "mgl.h"
#include "physics.h"
#include "robot.h"

#include <vector>

enum class ActionState { Idle, Walk, Run, Jump };

struct PlayerInput {
    bool forward = false, back = false, left = false, right = false;
    bool run = false;  // SHIFT
    bool jump = false; // SPACE（本帧刚按下）
    float camYaw = 0.0f;
};

// ----------------------------------------------------------------------------
// 动画师：根据"状态 + 相位"摆放小宇的四肢。
// 纯函数式设计：不持有状态，每帧根据输入直接算出关节角。
// ----------------------------------------------------------------------------
class Animator {
public:
    // phase: 步伐相位（弧度）；airTime: 离地时长；vy: 垂直速度
    void update(Robot& r, ActionState st, float phase, float dt,
                float vy = 0.0f) {
        using namespace mgl;
        switch (st) {
        case ActionState::Idle: {
            // 待机：四肢缓缓归零 + 轻微"呼吸"起伏
            toZero(r, dt);
            idleTime_ += dt;
            r.crouch = 0.01f + 0.01f * std::sin(idleTime_ * 2.0f);
            break;
        }
        case ActionState::Walk: {
            float swing = std::sin(phase);
            r.legSwingL = swing * radians(30.0f);
            r.legSwingR = -swing * radians(30.0f);
            r.armSwingL = -swing * radians(24.0f); // 手脚反相
            r.armSwingR = swing * radians(24.0f);
            r.armLiftL = mgl::lerp(r.armLiftL, 0.0f, 8.0f * dt);
            r.armLiftR = mgl::lerp(r.armLiftR, 0.0f, 8.0f * dt);
            r.crouch = 0.0f;
            break;
        }
        case ActionState::Run: {
            // 冲刺：摆幅更大、手臂端起来（想想百米运动员）
            float swing = std::sin(phase);
            r.legSwingL = swing * radians(48.0f);
            r.legSwingR = -swing * radians(48.0f);
            r.armSwingL = -swing * radians(42.0f);
            r.armSwingR = swing * radians(42.0f);
            r.armLiftL = mgl::lerp(r.armLiftL, radians(12.0f), 8.0f * dt);
            r.armLiftR = mgl::lerp(r.armLiftR, radians(12.0f), 8.0f * dt);
            r.crouch = 0.02f + 0.02f * std::fabs(std::cos(phase)); // 重心起伏
            break;
        }
        case ActionState::Jump: {
            // 空中姿态由垂直速度决定：上升展开、下落收紧准备落地
            float t = mgl::lerp(0.0f, 1.0f, vy > 0.0f ? 0.0f : 1.0f);
            float back0 = radians(-20.0f), fwd0 = radians(35.0f);
            r.legSwingL = mgl::lerp(fwd0, back0, t);  // 前后剪刀腿
            r.legSwingR = mgl::lerp(back0, fwd0, t);
            r.armSwingL = radians(-30.0f);
            r.armSwingR = radians(-30.0f);
            r.armLiftL = radians(35.0f); // 双臂展开保持"平衡"
            r.armLiftR = radians(35.0f);
            r.crouch = 0.0f;
            break;
        }
        }
        if (st != ActionState::Idle) idleTime_ = 0.0f;
    }

private:
    float idleTime_ = 0.0f;

    static void toZero(Robot& r, float dt) {
        float t = 10.0f * dt;
        if (t > 1.0f) t = 1.0f;
        r.legSwingL = mgl::lerp(r.legSwingL, 0.0f, t);
        r.legSwingR = mgl::lerp(r.legSwingR, 0.0f, t);
        r.armSwingL = mgl::lerp(r.armSwingL, 0.0f, t);
        r.armSwingR = mgl::lerp(r.armSwingR, 0.0f, t);
        r.armLiftL = mgl::lerp(r.armLiftL, 0.0f, t);
        r.armLiftR = mgl::lerp(r.armLiftR, 0.0f, t);
    }
};

// ----------------------------------------------------------------------------
// 玩家控制器：输入 → 状态机 → 位移 + 碰撞 → 动画
// ----------------------------------------------------------------------------
class Player {
public:
    Robot robot;
    ActionState state = ActionState::Idle;

    float walkSpeed = 2.2f;
    float runSpeed = 4.6f;
    float jumpSpeed = 4.5f;       // 起跳初速度（≈ 跳 1 米高）
    static constexpr float kGravity = -9.8f;

    void update(const PlayerInput& in, const std::vector<AABB>& colliders,
                float dt) {
        using namespace mgl;

        // ── ① 水平移动（第 3 周的成果，空中同样生效但操控减半）──
        float fwd = (in.forward ? 1.0f : 0.0f) - (in.back ? 1.0f : 0.0f);
        float side = (in.right ? 1.0f : 0.0f) - (in.left ? 1.0f : 0.0f);
        bool moving = (fwd != 0.0f || side != 0.0f);

        bool onGround = (robot.position.y <= 0.001f && vy_ <= 0.0f);
        float speed = in.run ? runSpeed : walkSpeed;
        if (!onGround) speed *= 0.55f; // 空中操控打折：跳出去就要负责

        if (moving) {
            vec3 front{std::sin(in.camYaw), 0.0f, std::cos(in.camYaw)};
            vec3 right{-std::cos(in.camYaw), 0.0f, std::sin(in.camYaw)};
            vec3 dir = normalize(front * fwd + right * side);

            // 试探移动 + AABB 碰撞：X/Z 轴分开处理，撞墙还能沿墙滑动
            tryMove({dir.x * speed * dt, 0.0f, 0.0f}, colliders);
            tryMove({0.0f, 0.0f, dir.z * speed * dt}, colliders);

            float targetYaw = std::atan2(dir.x, dir.z);
            robot.yaw = smoothTurn(robot.yaw, targetYaw, 12.0f * dt);
            distance_ += speed * dt;
        }

        // ── ② 跳跃运动学（工单 SD-402）──
        if (in.jump && onGround) {
            vy_ = jumpSpeed;   // 起跳：给一个向上的初速度
            onGround = false;
        }
        if (!onGround) {
            vy_ += kGravity * dt;            // 重力积分：v = v + g·dt
            robot.position.y += vy_ * dt;    // 位置积分：y = y + v·dt
            if (robot.position.y <= 0.0f) {  // 落地
                robot.position.y = 0.0f;
                vy_ = 0.0f;
            }
        }

        // ── ③ 状态机：一目了然的优先级判断 ──
        bool inAir = robot.position.y > 0.001f || vy_ > 0.0f;
        if (inAir)             state = ActionState::Jump;
        else if (!moving)      state = ActionState::Idle;
        else if (in.run)       state = ActionState::Run;
        else                   state = ActionState::Walk;

        // ── ④ 动画（工单 SD-401）──
        float freq = (state == ActionState::Run) ? 7.5f : 5.5f;
        animator_.update(robot, state, distance_ * freq, dt, vy_);

        // 场地边界
        const float lim = 18.0f;
        if (robot.position.x > lim) robot.position.x = lim;
        if (robot.position.x < -lim) robot.position.x = -lim;
        if (robot.position.z > lim) robot.position.z = lim;
        if (robot.position.z < -lim) robot.position.z = -lim;
    }

    const char* stateName() const {
        switch (state) {
        case ActionState::Idle: return "IDLE";
        case ActionState::Walk: return "WALK";
        case ActionState::Run:  return "RUN";
        case ActionState::Jump: return "JUMP";
        }
        return "?";
    }

private:
    float vy_ = 0.0f;       // 垂直速度
    float distance_ = 0.0f; // 累计路程（驱动步伐相位）
    Animator animator_;

    // 角色的碰撞盒：0.55 宽 × 1.55 高 × 0.4 深，跟随脚底位置
    AABB bodyBox(const mgl::vec3& pos) const {
        return AABB::fromCenter({pos.x, pos.y + 0.78f, pos.z},
                                {0.55f, 1.55f, 0.40f});
    }

    // 试探移动：先挪过去，撞上任何碰撞盒就退回来（工单 SD-403）
    void tryMove(const mgl::vec3& delta, const std::vector<AABB>& colliders) {
        mgl::vec3 next = robot.position + delta;
        AABB box = bodyBox(next);
        for (const AABB& c : colliders)
            if (aabbIntersect(box, c)) return; // 撞墙：本轴移动作废
        robot.position = next;
    }

    static float smoothTurn(float cur, float target, float t) {
        float diff = target - cur;
        while (diff > mgl::PI) diff -= 2.0f * mgl::PI;
        while (diff < -mgl::PI) diff += 2.0f * mgl::PI;
        if (t > 1.0f) t = 1.0f;
        return cur + diff * t;
    }
};
