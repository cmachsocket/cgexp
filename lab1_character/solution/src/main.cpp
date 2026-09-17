// ============================================================================
//  《CUBE RUNNER》第 1 周：把机器人画到屏幕上
//
//  工单 SD-101：实现 mgl 数学库的几何变换（translate / scale / rotate）
//  工单 SD-102：用层级建模拼出主角机器人（见 robot.h）
//
//  操作说明：
//      ←/→      手动旋转展示台          SPACE  暂停/继续自动旋转
//      1/2/3/4   站立 / 挥手 / T-pose / 行走定格
//      TAB       线框模式（看看模型的三角形本质）
//      ESC       退出
// ============================================================================
#include "mgl.h"
#include "robot.h"

#include <app.h>
#include <mesh.h>
#include <shader.h>

#include <GLFW/glfw3.h>

#include <cstdio>

// ---------------------------------------------------------------------------
// 着色器（引擎组提供，第 2 周你会学到它的原理并升级它）
// ---------------------------------------------------------------------------
static const char* kVertexSrc = R"(#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;   // 模型矩阵：模型空间 -> 世界空间（明宇的工单！）
uniform mat4 uView;    // 观察矩阵：世界空间 -> 相机空间
uniform mat4 uProj;    // 投影矩阵：相机空间 -> 裁剪空间

out vec3 vNormal;

void main() {
    // 图形学第一公式：裁剪坐标 = 投影 × 观察 × 模型 × 顶点
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
    // 法线要用"法线矩阵"变换（处理非均匀缩放），第 2 周细讲
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
}
)";

static const char* kFragmentSrc = R"(#version 410 core
in vec3 vNormal;
out vec4 FragColor;

uniform vec3 uColor;

void main() {
    // 简易明暗：法线越朝向光源越亮（完整光照模型第 2 周实现）
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(0.5, 1.0, 0.6));
    float diff = max(dot(N, L), 0.0);
    FragColor = vec4(uColor * (0.35 + 0.65 * diff), 1.0);
}
)";

int main() {
    std::printf(
        "==============================================\n"
        " CUBE RUNNER | 第 1 周：机器人\"小宇\"诞生记\n"
        " <-/->: 转展示台  SPACE: 自动旋转  1~4: 姿势\n"
        " TAB: 线框  ESC: 退出\n"
        "==============================================\n");

    fw::App app{"CUBE RUNNER - Week 1: Robot | StarDome Games", 1280, 720};
    fw::Shader shader{kVertexSrc, kFragmentSrc};
    fw::Mesh cube = fw::makeCube();
    fw::Mesh plane = fw::makePlane();

    Robot robot;
    float turntable = 0.0f; // 展示台角度
    bool spinning = true;
    bool wireframe = false;

    while (app.beginFrame()) {
        // --- 输入 ---
        if (app.keyPressed(GLFW_KEY_SPACE)) spinning = !spinning;
        if (spinning) turntable += mgl::radians(30.0f) * app.dt();
        if (app.keyDown(GLFW_KEY_LEFT))  turntable -= mgl::radians(90.0f) * app.dt();
        if (app.keyDown(GLFW_KEY_RIGHT)) turntable += mgl::radians(90.0f) * app.dt();

        if (app.keyPressed(GLFW_KEY_TAB)) {
            wireframe = !wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        }

        // 姿势预设：直接设置关节角，验证层级建模的正确性
        if (app.keyPressed(GLFW_KEY_1)) robot = Robot{}; // 站立
        if (app.keyPressed(GLFW_KEY_2)) {                // 挥手
            robot = Robot{};
            robot.armLiftR = mgl::radians(150.0f);
            robot.headYaw = mgl::radians(-15.0f);
        }
        if (app.keyPressed(GLFW_KEY_3)) {                // T-pose（游戏业圣诞树）
            robot = Robot{};
            robot.armLiftL = robot.armLiftR = mgl::radians(90.0f);
        }
        if (app.keyPressed(GLFW_KEY_4)) {                // 行走定格帧（第 4 周预告）
            robot = Robot{};
            robot.legSwingL = mgl::radians(28.0f);
            robot.legSwingR = mgl::radians(-28.0f);
            robot.armSwingL = mgl::radians(-25.0f);
            robot.armSwingR = mgl::radians(25.0f);
        }

        robot.yaw = turntable;

        // --- 相机（引擎组临时支援，第 3 周由你接管）---
        mgl::mat4 view = mgl::lookAt({0.0f, 1.6f, 4.0f},   // 相机位置
                                     {0.0f, 0.85f, 0.0f},  // 看向机器人胸口
                                     {0.0f, 1.0f, 0.0f});  // 世界上方向
        mgl::mat4 proj = mgl::perspective(mgl::radians(50.0f), app.aspect(),
                                          0.1f, 100.0f);

        // --- 渲染 ---
        shader.use();
        shader.setMat4("uView", view.data());
        shader.setMat4("uProj", proj.data());

        // 展示台地面：一块 6×6 的深色平面
        mgl::mat4 ground = mgl::scale(6.0f, 1.0f, 6.0f);
        shader.setMat4("uModel", ground.data());
        shader.setVec3("uColor", 0.30f, 0.33f, 0.38f);
        plane.draw();

        robot.draw(shader, cube);

        app.endFrame();
    }
    return 0;
}
