#include "app.h"

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>

namespace fw {

App::App(const std::string& title, int width, int height)
    : width_(width), height_(height) {
    if (!glfwInit()) {
        std::fprintf(stderr, "[fw] glfwInit 失败\n");
        std::exit(EXIT_FAILURE);
    }
    // 申请 OpenGL 4.1 Core Profile 上下文（macOS 支持的最高版本，全平台通用）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA 抗锯齿

    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        std::fprintf(stderr, "[fw] 创建窗口失败（显卡驱动支持 OpenGL 4.1 吗？）\n");
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // 垂直同步

    if (!glcoreInit(glfwGetProcAddress)) {
        std::fprintf(stderr, "[fw] OpenGL 函数加载失败\n");
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    std::printf("[fw] OpenGL  : %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::printf("[fw] Renderer: %s\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    glEnable(GL_DEPTH_TEST); // 深度测试：近处的物体挡住远处的物体
    lastTime_ = glfwGetTime();
}

App::~App() {
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

bool App::beginFrame() {
    if (glfwWindowShouldClose(window_)) return false;

    glfwPollEvents();
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window_, GLFW_TRUE);

    // 帧计时
    double now = glfwGetTime();
    dt_ = float(now - lastTime_);
    if (dt_ > 0.1f) dt_ = 0.1f; // 断点调试/卡顿保护：单帧最多按 0.1 秒算
    lastTime_ = now;
    time_ = float(now);

    // 键盘边沿检测
    for (int k = 0; k < 512; ++k) {
        prevKeys_[k] = curKeys_[k];
        curKeys_[k] = (glfwGetKey(window_, k) == GLFW_PRESS) ? 1 : 0;
    }

    // 鼠标位移
    double mx, my;
    glfwGetCursorPos(window_, &mx, &my);
    if (firstMouse_) { mouseX_ = mx; mouseY_ = my; firstMouse_ = false; }
    mouseDX_ = mx - mouseX_;
    mouseDY_ = my - mouseY_;
    mouseX_ = mx;
    mouseY_ = my;

    // 跟随窗口大小调整视口
    glfwGetFramebufferSize(window_, &width_, &height_);
    glViewport(0, 0, width_, height_);

    glClearColor(0.53f, 0.70f, 0.92f, 1.0f); // 天空蓝
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return true;
}

void App::endFrame() { glfwSwapBuffers(window_); }

bool App::keyDown(int key) const {
    return key >= 0 && key < 512 && curKeys_[key];
}

bool App::keyPressed(int key) const {
    return key >= 0 && key < 512 && curKeys_[key] && !prevKeys_[key];
}

void App::setMouseCaptured(bool captured) {
    mouseCaptured_ = captured;
    glfwSetInputMode(window_, GLFW_CURSOR,
                     captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    firstMouse_ = true; // 防止切换瞬间视角跳变
}

} // namespace fw
