// ============================================================================
//  《CUBE RUNNER》第 2 周：无中生有——搭出方块镇
//
//  工单 SD-201：场景建模（world.h 里的 addTree / addHouse / addCrate）
//  工单 SD-202：Phong 光照（本文件片段着色器中的三段公式）
//
//  操作说明：
//      ←/→    环绕镇子观察          ↑/↓   调整相机高度
//      SPACE   暂停/继续自动环绕     L     让太阳转起来（看光照变化！）
//      TAB     线框模式             ESC   退出
// ============================================================================
#include "mgl.h"
#include "robot.h"
#include "world.h"

#include <app.h>
#include <mesh.h>
#include <shader.h>

#include <GLFW/glfw3.h>

#include <cstdio>

static const char* kVertexSrc = R"(#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vWorldPos;  // 顶点的世界坐标（光照要用）
out vec3 vNormal;    // 世界空间法线
out vec2 vUV;

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    gl_Position = uProj * uView * world;
    // 法线矩阵：模型矩阵含非均匀缩放时，法线不能直接乘 uModel，
    // 要乘它左上 3×3 的逆转置（推导见第 2 周讲义）
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vUV = aUV;
}
)";

// ---------------------------------------------------------------------------
// ★ 工单 SD-202 的战场：Phong 光照模型 ★
//     环境光 ambient  = ka · lightColor
//     漫反射 diffuse  = kd · max(N·L, 0) · lightColor
//     高光   specular = ks · max(V·R, 0)^shininess · lightColor
// ---------------------------------------------------------------------------
static const char* kFragmentSrc = R"(#version 410 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3 uColor;        // 物体基础色
uniform vec3 uLightDir;     // 平行光方向（从光源指向场景）
uniform vec3 uLightColor;   // 光的颜色
uniform vec3 uViewPos;      // 相机世界坐标（算高光要用）
uniform sampler2D uTex;     // 纹理（地面用）
uniform int  uUseTex;       // 是否采样纹理
uniform float uUVScale;     // 纹理平铺倍数

void main() {
    // 基础色：地面从棋盘格纹理采样，其他物体用纯色
    vec3 base = uColor;
    if (uUseTex == 1)
        base *= texture(uTex, vUV * uUVScale).rgb;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);            // 指向光源的方向

    // ====== TODO(明宇) 工单 SD-202：Phong 光照三件套 ======
    // 现在整个世界只有一个惨淡的保底亮度（下面那行），毫无立体感。
    // 请依次实现三个分量，每加一个，世界就活过来一分：
    //
    // ① 环境光 ambient：无论照不照得到都给保底亮度
    //        ambient = 0.28 · uLightColor
    //
    // ② 漫反射 diffuse：表面越正对光源越亮（Lambert 余弦定律）
    //        diff    = max(N·L, 0)                ← GLSL: max(dot(N, L), 0.0)
    //        diffuse = 0.75 · diff · uLightColor
    //
    // ③ 镜面高光 specular：反射光越对准眼睛越亮（Phong）
    //        V        = normalize(uViewPos - vWorldPos)   ← 指向相机
    //        R        = reflect(-L, N)                    ← 光线的反射方向
    //        spec     = max(V·R, 0)^32                    ← GLSL: pow(max(...), 32.0)
    //        specular = 0.30 · spec · uLightColor
    //
    // 最终合成：color = (ambient + diffuse) · base + specular
    //
    // 验收技巧：按 L 让太阳转起来——如果亮暗随太阳方向平滑变化、
    //           屋顶能看到刺眼的小亮斑，说明三个分量都对了。

    vec3 color = base * 0.3; // TODO: 删掉这行占位代码，换成 Phong 合成结果

    FragColor = vec4(color, 1.0);
}
)";

int main() {
    std::printf(
        "==============================================\n"
        " CUBE RUNNER | 第 2 周：方块镇竣工验收\n"
        " <-/->: 环绕观察  ^/v: 相机高度  SPACE: 暂停环绕\n"
        " L: 太阳旋转  TAB: 线框  ESC: 退出\n"
        "==============================================\n");

    fw::App app{"CUBE RUNNER - Week 2: BlockTown | StarDome Games", 1280, 720};
    fw::Shader shader{kVertexSrc, kFragmentSrc};
    fw::Mesh cube = fw::makeCube();
    fw::Mesh pyramid = fw::makePyramid();
    fw::Mesh plane = fw::makePlane();

    // 草地纹理：程序生成的双色棋盘格（美术组：这叫"极简像素风"）
    GLuint grassTex = fw::makeCheckerTexture(256, 8,
                                             96, 160, 80,    // 浅草绿
                                             76, 138, 66);   // 深草绿

    World world;
    world.build();

    Robot robot; // 主角先站在镇中心当"比例参照物"
    robot.armLiftR = mgl::radians(150.0f); // 跟验收的老周打个招呼

    float orbit = 0.0f, camHeight = 6.0f;
    bool orbiting = true, wireframe = false;
    float sunAngle = mgl::radians(35.0f);
    bool sunMoving = false;

    while (app.beginFrame()) {
        // --- 输入 ---
        if (app.keyPressed(GLFW_KEY_SPACE)) orbiting = !orbiting;
        if (app.keyPressed(GLFW_KEY_L)) sunMoving = !sunMoving;
        if (app.keyPressed(GLFW_KEY_TAB)) {
            wireframe = !wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        }
        if (orbiting) orbit += mgl::radians(12.0f) * app.dt();
        if (app.keyDown(GLFW_KEY_LEFT))  orbit -= mgl::radians(60.0f) * app.dt();
        if (app.keyDown(GLFW_KEY_RIGHT)) orbit += mgl::radians(60.0f) * app.dt();
        if (app.keyDown(GLFW_KEY_UP))    camHeight += 4.0f * app.dt();
        if (app.keyDown(GLFW_KEY_DOWN))  camHeight -= 4.0f * app.dt();
        if (camHeight < 1.0f) camHeight = 1.0f;
        if (sunMoving) sunAngle += mgl::radians(20.0f) * app.dt();

        // --- 相机（引擎组临时支援：绕镇中心的环绕相机，第 3 周由你接管）---
        const float radius = 14.0f;
        mgl::vec3 eye{radius * std::sin(orbit), camHeight, radius * std::cos(orbit)};
        mgl::mat4 view = mgl::lookAt(eye, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
        mgl::mat4 proj = mgl::perspective(mgl::radians(50.0f), app.aspect(),
                                          0.1f, 200.0f);

        // 太阳：平行光方向随 sunAngle 转动
        mgl::vec3 lightDir = mgl::normalize(
            {std::cos(sunAngle), -0.85f, std::sin(sunAngle)});

        // --- 渲染 ---
        shader.use();
        shader.setMat4("uView", view.data());
        shader.setMat4("uProj", proj.data());
        shader.setVec3("uLightDir", lightDir.x, lightDir.y, lightDir.z);
        shader.setVec3("uLightColor", 1.0f, 0.98f, 0.92f);
        shader.setVec3("uViewPos", eye.x, eye.y, eye.z);
        shader.setInt("uTex", 0);

        // 地面（用纹理）
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        shader.setInt("uUseTex", 1);
        shader.setFloat("uUVScale", 20.0f);
        mgl::mat4 ground = mgl::scale(world.groundSize, 1.0f, world.groundSize);
        shader.setMat4("uModel", ground.data());
        shader.setVec3("uColor", 1.0f, 1.0f, 1.0f);
        plane.draw();

        // 道具与主角（纯色）
        shader.setInt("uUseTex", 0);
        world.draw(shader, cube, pyramid);
        robot.draw(shader, cube);

        app.endFrame();
    }
    return 0;
}
