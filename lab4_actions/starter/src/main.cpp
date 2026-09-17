// ============================================================================
//  《CUBE RUNNER》第 4 周：注入灵魂——Game Jam 交付版
//
//  工单 SD-401：动画状态机 Animator::update（player.h）
//  工单 SD-402：跳跃运动学（player.h）
//  工单 SD-403：AABB 碰撞检测（physics.h + player.h::tryMove）
//
//  操作说明：
//      WASD    行走         SHIFT+WASD  冲刺
//      SPACE   跳跃         鼠标        环绕视角
//      C       跟随相机 ⇄ 自由飞行相机（Q/E 升降）
//      TAB     线框模式     ESC         退出
// ============================================================================
#include "camera.h"
#include "mgl.h"
#include "physics.h"
#include "player.h"
#include "robot.h"
#include "world.h"

#include <app.h>
#include <mesh.h>
#include <shader.h>

#include <GLFW/glfw3.h>

#include <cstdio>
#include <string>

static const char* kVertexSrc = R"(#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    gl_Position = uProj * uView * world;
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vUV = aUV;
}
)";

static const char* kFragmentSrc = R"(#version 410 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uViewPos;
uniform sampler2D uTex;
uniform int  uUseTex;
uniform float uUVScale;

void main() {
    vec3 base = uColor;
    if (uUseTex == 1)
        base *= texture(uTex, vUV * uUVScale).rgb;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 ambient = 0.28 * uLightColor;
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = 0.75 * diff * uLightColor;
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);
    vec3 specular = 0.30 * spec * uLightColor;
    FragColor = vec4((ambient + diffuse) * base + specular, 1.0);
}
)";

int main() {
    std::printf(
        "==============================================\n"
        " CUBE RUNNER | 第 4 周：Game Jam 交付版\n"
        " WASD: 行走  SHIFT: 冲刺  SPACE: 跳跃\n"
        " 鼠标: 视角  C: 切换相机  TAB: 线框  ESC: 退出\n"
        "==============================================\n");

    fw::App app{"CUBE RUNNER | StarDome Games - Game Jam Build", 1280, 720};
    fw::Shader shader{kVertexSrc, kFragmentSrc};
    fw::Mesh cube = fw::makeCube();
    fw::Mesh pyramid = fw::makePyramid();
    fw::Mesh plane = fw::makePlane();
    GLuint grassTex = fw::makeCheckerTexture(256, 8, 96, 160, 80, 76, 138, 66);

    World world;
    world.build();

    Player player;
    FollowCamera followCam;
    FreeCamera freeCam;
    bool useFreeCam = false;
    bool wireframe = false;

    app.setMouseCaptured(true);

    float titleTimer = 0.0f;

    while (app.beginFrame()) {
        if (app.keyPressed(GLFW_KEY_C)) useFreeCam = !useFreeCam;
        if (app.keyPressed(GLFW_KEY_TAB)) {
            wireframe = !wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        }

        mgl::mat4 view;
        mgl::vec3 eye;

        if (useFreeCam) {
            freeCam.update(float(app.mouseDX()), float(app.mouseDY()),
                           app.keyDown(GLFW_KEY_W), app.keyDown(GLFW_KEY_S),
                           app.keyDown(GLFW_KEY_A), app.keyDown(GLFW_KEY_D),
                           app.keyDown(GLFW_KEY_E), app.keyDown(GLFW_KEY_Q),
                           app.dt());
            view = freeCam.viewMatrix();
            eye = freeCam.position;
            PlayerInput idle{};
            idle.camYaw = followCam.forwardYaw();
            player.update(idle, world.colliders, app.dt());
        } else {
            PlayerInput in;
            in.forward = app.keyDown(GLFW_KEY_W);
            in.back = app.keyDown(GLFW_KEY_S);
            in.left = app.keyDown(GLFW_KEY_A);
            in.right = app.keyDown(GLFW_KEY_D);
            in.run = app.keyDown(GLFW_KEY_LEFT_SHIFT);
            in.jump = app.keyPressed(GLFW_KEY_SPACE);
            in.camYaw = followCam.forwardYaw();
            player.update(in, world.colliders, app.dt());

            followCam.update(player.robot.position, float(app.mouseDX()),
                             float(app.mouseDY()));
            view = followCam.viewMatrix();
            eye = followCam.eye;
        }

        // 窗口标题实时显示动作状态——穷人版调试 HUD
        titleTimer += app.dt();
        if (titleTimer > 0.25f) {
            titleTimer = 0.0f;
            std::string title = std::string("CUBE RUNNER | State: ") +
                                player.stateName() +
                                (useFreeCam ? " | FREE CAM" : "");
            glfwSetWindowTitle(app.handle(), title.c_str());
        }

        mgl::mat4 proj = mgl::perspective(mgl::radians(50.0f), app.aspect(),
                                          0.1f, 200.0f);
        mgl::vec3 lightDir = mgl::normalize({0.45f, -0.85f, 0.35f});

        shader.use();
        shader.setMat4("uView", view.data());
        shader.setMat4("uProj", proj.data());
        shader.setVec3("uLightDir", lightDir.x, lightDir.y, lightDir.z);
        shader.setVec3("uLightColor", 1.0f, 0.98f, 0.92f);
        shader.setVec3("uViewPos", eye.x, eye.y, eye.z);
        shader.setInt("uTex", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        shader.setInt("uUseTex", 1);
        shader.setFloat("uUVScale", 20.0f);
        mgl::mat4 ground = mgl::scale(world.groundSize, 1.0f, world.groundSize);
        shader.setMat4("uModel", ground.data());
        shader.setVec3("uColor", 1.0f, 1.0f, 1.0f);
        plane.draw();

        shader.setInt("uUseTex", 0);
        world.draw(shader, cube, pyramid);
        player.robot.draw(shader, cube);

        app.endFrame();
    }
    return 0;
}
