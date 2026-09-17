#include "shader.h"

#include <cstdio>

namespace fw {

namespace {

GLuint compile(GLenum type, const std::string& src) {
    GLuint sh = glCreateShader(type);
    const char* p = src.c_str();
    glShaderSource(sh, 1, &p, nullptr);
    glCompileShader(sh);

    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[fw] %s着色器编译失败:\n%s\n",
                     type == GL_VERTEX_SHADER ? "顶点" : "片段", log);
    }
    return sh;
}

} // namespace

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
    GLuint vs = compile(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSrc);

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[fw] 着色器链接失败:\n%s\n", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() {
    if (program_) glDeleteProgram(program_);
}

void Shader::use() const { glUseProgram(program_); }

GLint Shader::loc(const char* name) const {
    return glGetUniformLocation(program_, name);
}

void Shader::setMat4(const char* name, const float* col16) const {
    glUniformMatrix4fv(loc(name), 1, GL_FALSE, col16);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    glUniform3f(loc(name), x, y, z);
}

void Shader::setFloat(const char* name, float v) const {
    glUniform1f(loc(name), v);
}

void Shader::setInt(const char* name, int v) const {
    glUniform1i(loc(name), v);
}

} // namespace fw
