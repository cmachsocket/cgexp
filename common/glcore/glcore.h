// ============================================================================
//  glcore.h — 星穹互娱引擎组：迷你 OpenGL 4.1 Core 函数加载器
//
//  OpenGL 的函数并不在头文件里，而是藏在显卡驱动中，必须在运行时逐个
//  "查询函数地址"。本文件用 X-Macro 声明了本项目用到的所有 GL 函数指针，
//  glcoreInit() 负责在窗口创建后把它们全部加载好。
//
//  （引擎组注：这就是 glad / glew 这类库做的事情——我们自己写一个，
//    顺便让你知道它没有魔法。）
// ============================================================================
#pragma once

#include "GL/glcorearb.h"

// 项目中用到的全部 OpenGL 函数（类型, 名字）
#define GLCORE_FUNC_LIST(X)                                                    \
    /* --- 状态与查询 --- */                                                   \
    X(PFNGLCLEARPROC, glClear)                                                 \
    X(PFNGLCLEARCOLORPROC, glClearColor)                                       \
    X(PFNGLENABLEPROC, glEnable)                                               \
    X(PFNGLDISABLEPROC, glDisable)                                             \
    X(PFNGLVIEWPORTPROC, glViewport)                                           \
    X(PFNGLGETINTEGERVPROC, glGetIntegerv)                                     \
    X(PFNGLGETSTRINGPROC, glGetString)                                         \
    X(PFNGLGETERRORPROC, glGetError)                                           \
    X(PFNGLCULLFACEPROC, glCullFace)                                           \
    X(PFNGLFRONTFACEPROC, glFrontFace)                                         \
    X(PFNGLDEPTHFUNCPROC, glDepthFunc)                                         \
    X(PFNGLPOLYGONMODEPROC, glPolygonMode)                                     \
    X(PFNGLLINEWIDTHPROC, glLineWidth)                                         \
    X(PFNGLBLENDFUNCPROC, glBlendFunc)                                         \
    /* --- 着色器 --- */                                                       \
    X(PFNGLCREATESHADERPROC, glCreateShader)                                   \
    X(PFNGLSHADERSOURCEPROC, glShaderSource)                                   \
    X(PFNGLCOMPILESHADERPROC, glCompileShader)                                 \
    X(PFNGLGETSHADERIVPROC, glGetShaderiv)                                     \
    X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)                           \
    X(PFNGLDELETESHADERPROC, glDeleteShader)                                   \
    X(PFNGLCREATEPROGRAMPROC, glCreateProgram)                                 \
    X(PFNGLATTACHSHADERPROC, glAttachShader)                                   \
    X(PFNGLLINKPROGRAMPROC, glLinkProgram)                                     \
    X(PFNGLGETPROGRAMIVPROC, glGetProgramiv)                                   \
    X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)                         \
    X(PFNGLUSEPROGRAMPROC, glUseProgram)                                       \
    X(PFNGLDELETEPROGRAMPROC, glDeleteProgram)                                 \
    X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)                       \
    X(PFNGLUNIFORM1IPROC, glUniform1i)                                         \
    X(PFNGLUNIFORM1FPROC, glUniform1f)                                         \
    X(PFNGLUNIFORM3FPROC, glUniform3f)                                         \
    X(PFNGLUNIFORM3FVPROC, glUniform3fv)                                       \
    X(PFNGLUNIFORM4FVPROC, glUniform4fv)                                       \
    X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv)                           \
    /* --- 顶点数组与缓冲 --- */                                               \
    X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)                             \
    X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)                             \
    X(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)                       \
    X(PFNGLGENBUFFERSPROC, glGenBuffers)                                       \
    X(PFNGLBINDBUFFERPROC, glBindBuffer)                                       \
    X(PFNGLBUFFERDATAPROC, glBufferData)                                       \
    X(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)                                 \
    X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)                     \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)             \
    X(PFNGLDRAWARRAYSPROC, glDrawArrays)                                       \
    X(PFNGLDRAWELEMENTSPROC, glDrawElements)                                   \
    /* --- 纹理 --- */                                                         \
    X(PFNGLGENTEXTURESPROC, glGenTextures)                                     \
    X(PFNGLBINDTEXTUREPROC, glBindTexture)                                     \
    X(PFNGLTEXIMAGE2DPROC, glTexImage2D)                                       \
    X(PFNGLTEXPARAMETERIPROC, glTexParameteri)                                 \
    X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap)                               \
    X(PFNGLACTIVETEXTUREPROC, glActiveTexture)                                 \
    X(PFNGLDELETETEXTURESPROC, glDeleteTextures)                               \
    X(PFNGLPIXELSTOREIPROC, glPixelStorei)

// 声明所有函数指针（真实变量名带 glcore_ 前缀，再用宏映射回标准 GL 名）
#define GLCORE_DECLARE(type, name) extern type glcore_##name;
GLCORE_FUNC_LIST(GLCORE_DECLARE)
#undef GLCORE_DECLARE

#define glClear glcore_glClear
#define glClearColor glcore_glClearColor
#define glEnable glcore_glEnable
#define glDisable glcore_glDisable
#define glViewport glcore_glViewport
#define glGetIntegerv glcore_glGetIntegerv
#define glGetString glcore_glGetString
#define glGetError glcore_glGetError
#define glCullFace glcore_glCullFace
#define glFrontFace glcore_glFrontFace
#define glDepthFunc glcore_glDepthFunc
#define glPolygonMode glcore_glPolygonMode
#define glLineWidth glcore_glLineWidth
#define glBlendFunc glcore_glBlendFunc
#define glCreateShader glcore_glCreateShader
#define glShaderSource glcore_glShaderSource
#define glCompileShader glcore_glCompileShader
#define glGetShaderiv glcore_glGetShaderiv
#define glGetShaderInfoLog glcore_glGetShaderInfoLog
#define glDeleteShader glcore_glDeleteShader
#define glCreateProgram glcore_glCreateProgram
#define glAttachShader glcore_glAttachShader
#define glLinkProgram glcore_glLinkProgram
#define glGetProgramiv glcore_glGetProgramiv
#define glGetProgramInfoLog glcore_glGetProgramInfoLog
#define glUseProgram glcore_glUseProgram
#define glDeleteProgram glcore_glDeleteProgram
#define glGetUniformLocation glcore_glGetUniformLocation
#define glUniform1i glcore_glUniform1i
#define glUniform1f glcore_glUniform1f
#define glUniform3f glcore_glUniform3f
#define glUniform3fv glcore_glUniform3fv
#define glUniform4fv glcore_glUniform4fv
#define glUniformMatrix4fv glcore_glUniformMatrix4fv
#define glGenVertexArrays glcore_glGenVertexArrays
#define glBindVertexArray glcore_glBindVertexArray
#define glDeleteVertexArrays glcore_glDeleteVertexArrays
#define glGenBuffers glcore_glGenBuffers
#define glBindBuffer glcore_glBindBuffer
#define glBufferData glcore_glBufferData
#define glDeleteBuffers glcore_glDeleteBuffers
#define glVertexAttribPointer glcore_glVertexAttribPointer
#define glEnableVertexAttribArray glcore_glEnableVertexAttribArray
#define glDrawArrays glcore_glDrawArrays
#define glDrawElements glcore_glDrawElements
#define glGenTextures glcore_glGenTextures
#define glBindTexture glcore_glBindTexture
#define glTexImage2D glcore_glTexImage2D
#define glTexParameteri glcore_glTexParameteri
#define glGenerateMipmap glcore_glGenerateMipmap
#define glActiveTexture glcore_glActiveTexture
#define glDeleteTextures glcore_glDeleteTextures
#define glPixelStorei glcore_glPixelStorei

// 加载函数：loader 传入 glfwGetProcAddress 即可。全部加载成功返回 true。
using GlcoreLoadFn = void (*(*)(const char* name))();
bool glcoreInit(GlcoreLoadFn loader);
