// ============================================================================
//  robot.h — 《CUBE RUNNER》主角：机器人"小宇"
//
//  美术组的建模软件许可证还在采购流程里（老周：估计到项目结束都批不下来），
//  所以主角由 7 个立方体拼成：躯干、头、眼罩、双臂、双腿。
//
//  ★ 层级建模（hierarchical modeling）★
//  每个身体部件的模型矩阵都从"根矩阵"出发向下组合：
//
//      root = T(位置) · R_y(朝向)                    ← 整个角色在世界中的位姿
//      部件 = root · T(关节点) · R(关节角) · T(偏移) · S(尺寸)
//
//  这样旋转关节角时，部件会绕"关节点"转，而不是绕自己的中心转——
//  这正是手臂看起来长在肩膀上的全部秘密。
//
//  工单 SD-102：躯干和头引擎组已经拼好当示范，四肢归你。
//
//  关节角约定（弧度，动画系统在第 4 周会驱动它们）：
//      armSwing*  > 0 手臂向前摆      legSwing* > 0 腿向前摆
//      armLift*   > 0 手臂向体侧抬起   crouch    > 0 下蹲（跳跃预备）
// ============================================================================
#pragma once

#include "mgl.h"

#include <mesh.h>
#include <shader.h>

struct Robot
{
    // 世界位姿
    mgl::vec3 position{0.0f, 0.0f, 0.0f};
    float yaw = 0.0f; // 绕 Y 轴朝向。yaw=0 时面朝 +Z（眼罩所在的一侧）

    // 关节角（弧度）
    float armSwingL = 0.0f, armSwingR = 0.0f;
    float armLiftL = 0.0f, armLiftR = 0.0f;
    float legSwingL = 0.0f, legSwingR = 0.0f;
    float headYaw = 0.0f;
    float crouch = 0.0f;
    float dickAngle = 2.356f;

    // 身高约 1.5 米：脚底 y=0，头顶 y≈1.42+0.16
    void draw(const fw::Shader &sh, const fw::Mesh &cube) const
    {
        using namespace mgl;

        auto part = [&](const mat4 &model, float r, float g, float b)
        {
            sh.setMat4("uModel", model.data());
            sh.setVec3("uColor", r, g, b);
            cube.draw();
        };

        // 根矩阵：先旋转朝向，再平移到世界位置（注意乘法顺序！）
        mat4 root = translate(position) * rotateY(yaw) * translate(0, -crouch, 0);

        // ── 示范 ①  躯干：中心 y=0.90，尺寸 0.50×0.60×0.28 ──
        part(root * translate(0, 0.90f, 0) * scale(0.50f, 0.60f, 0.28f),
             0.20f, 0.45f, 0.85f);

        // ── 示范 ②  头：颈关节在 y=1.26，头可独立转动（headYaw）──
        mat4 head = root * translate(0, 1.26f, 0) * rotateY(headYaw);
        part(head * translate(0, 0.16f, 0) * scale(0.32f, 0.32f, 0.32f),
             0.98f, 0.83f, 0.60f);
        // 眼罩：贴在头的 +Z 面（这就是"脸"的方向）
        part(head * translate(0, 0.20f, 0.165f) * scale(0.24f, 0.08f, 0.02f),
             0.12f, 0.12f, 0.18f);
        // mouse
        part(head * translate(0, 0.05f, 0.165f) * rotateZ(0.7854f) * scale(0.14f, 0.04f, 0.02f),
             0.12f, 0.12f, 0.18f);
        // caps
        for (int i = 1; i <= 10; i++)
        {
            part(head * translate(0, 0.28f+i*0.10f, 0) * scale(0.38f, 0.08f, 0.38f),
                 0.12f, 0.12f, 0.18f);
        }

        // ====== TODO(明宇) 工单 SD-102-a：左臂 ======
        // 规格书：肩关节在 (-0.32, 1.15, 0)，臂杆尺寸 0.14×0.50×0.14，
        //         从肩关节垂下（杆件中心在关节下方 0.30 处），颜色 (0.16, 0.36, 0.70)
        // 组合顺序：root · T(肩关节) · Rz(-armLiftL) · Rx(-armSwingL) · T(0,-0.30,0) · S(尺寸)
        //
        // mat4 lArm = root * ??? ;
        // part(lArm * ??? , 0.16f, 0.36f, 0.70f);

        mat4 lArm = root * translate(-0.32f, 1.15f, 0) * rotateZ(-armLiftL) * rotateX(-armSwingL); // 再旋转，平移到该去的位置
        part(lArm * translate(0, -0.30f, 0) * scale(0.14f, 0.50f, 0.14f), 0.16f, 0.36f, 0.70f);    // 这里是先做的，先缩放然后确立中心
        // ====== TODO(明宇) 工单 SD-102-b：右臂 ======
        // 镜像左臂：肩关节 x 取 +0.32，侧抬用 Rz(+armLiftR)（方向相反才对称）
        mat4 rArm = root * translate(0.32f, 1.15f, 0) * rotateZ(armLiftR) * rotateX(-armSwingR);
        part(rArm * translate(0, -0.30f, 0) * scale(0.14f, 0.50f, 0.14f), 0.16f, 0.36f, 0.70f);
        // ====== TODO(明宇) 工单 SD-102-c：左腿 ======
        // 规格书：髋关节在 (-0.13, 0.62, 0)，腿杆尺寸 0.16×0.60×0.16，
        //         从髋关节垂下（中心偏移 -0.31），颜色 (0.22, 0.25, 0.33)
        // 前后摆动用 Rx(-legSwingL)
        mat4 lLeg = root * translate(-0.13f, 0.62f, 0) * rotateX(-legSwingL);
        part(lLeg * translate(0, -0.31f, 0) * scale(0.16f, 0.60f, 0.16f), 0.22f, 0.25f, 0.33f);
        // ====== TODO(明宇) 工单 SD-102-d：右腿 ======
        // 镜像左腿：髋关节 x 取 +0.13
        mat4 rLeg = root * translate(0.13f, 0.62f, 0) * rotateX(-legSwingR);
        part(rLeg * translate(0, -0.31f, 0) * scale(0.16f, 0.60f, 0.16f), 0.22f, 0.25f, 0.33f);

        // 髋关节在 (-0.13, 0.62, 0)
        // 大小我自有定夺。
        mat4 Dick = root * translate(0, 0.62f, 0) * rotateX(-dickAngle);
        part(Dick * translate(0, -0.41f, 0) * scale(0.10f, 1.50f, 0.10f), 0.98f, 0.83f, 0.60f);

        // 验收自查：按 3 进入 T-pose，双臂应该水平伸直、绕"肩膀"旋转；
        //           如果手臂绕自己的中心转（转成了螺旋桨），说明 T(偏移) 的
        //           位置放错了——回去检查乘法顺序。
    }
};
