// ============================================================================
//  app.h — 星穹互娱引擎组：窗口与主循环封装
//
//  引擎组已经替你处理好了开窗、创建 OpenGL 4.1 Core 上下文、加载 GL 函数、
//  垂直同步、键盘鼠标输入和帧计时。你只需要：
//
//      fw::App app{"窗口标题", 1280, 720};
//      while (app.beginFrame()) {   // 处理输入、计算 dt、清屏
//          ... 你的渲染代码 ...
//          app.endFrame();          // 交换双缓冲
//      }
//
//  （老周：不要在实验课上重写窗口系统，那是掉进无底洞的第一步。）
// ============================================================================
#pragma once

#include <glcore.h>

#include <string>

struct GLFWwindow;

namespace fw {

class App {
public:
    App(const std::string& title, int width, int height);
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    // 每帧开头调用：轮询事件、更新 dt、清屏。窗口被关闭时返回 false。
    bool beginFrame();
    // 每帧结尾调用：交换前后缓冲。
    void endFrame();

    // --- 输入查询 ---
    bool keyDown(int glfwKey) const;      // 该键当前是否按住
    bool keyPressed(int glfwKey) const;   // 该键本帧是否刚被按下（边沿触发）
    double mouseDX() const { return mouseDX_; }  // 本帧鼠标横向位移
    double mouseDY() const { return mouseDY_; }  // 本帧鼠标纵向位移
    void setMouseCaptured(bool captured); // 捕获鼠标（隐藏光标，用于相机控制）
    bool mouseCaptured() const { return mouseCaptured_; }

    // --- 帧信息 ---
    float dt() const { return dt_; }         // 上一帧耗时（秒）
    float time() const { return time_; }     // 程序启动至今（秒）
    int width() const { return width_; }
    int height() const { return height_; }
    float aspect() const { return height_ ? float(width_) / float(height_) : 1.0f; }

    GLFWwindow* handle() const { return window_; }

private:
    GLFWwindow* window_ = nullptr;
    int width_ = 0, height_ = 0;
    float dt_ = 0.0f, time_ = 0.0f;
    double lastTime_ = 0.0;
    double mouseX_ = 0.0, mouseY_ = 0.0;
    double mouseDX_ = 0.0, mouseDY_ = 0.0;
    bool firstMouse_ = true;
    bool mouseCaptured_ = false;
    unsigned char prevKeys_[512] = {};
    unsigned char curKeys_[512] = {};
};

} // namespace fw
