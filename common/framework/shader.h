// ============================================================================
//  shader.h — 星穹互娱引擎组：GLSL 着色器编译/链接封装
//
//  用法：
//      fw::Shader shader{vertexSrc, fragmentSrc};   // 传入 GLSL 源码字符串
//      shader.use();
//      shader.setMat4("uModel", m.data());          // 传 16 个 float（列主序）
//      shader.setVec3("uColor", 1, 0, 0);
//
//  编译错误会连同 GLSL 报错行号一起打印到控制台——先看控制台再喊卡！
// ============================================================================
#pragma once

#include <glcore.h>

#include <string>

namespace fw {

class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    void setMat4(const char* name, const float* col16) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setFloat(const char* name, float v) const;
    void setInt(const char* name, int v) const;

    GLuint id() const { return program_; }

private:
    GLuint program_ = 0;
    GLint loc(const char* name) const;
};

} // namespace fw
