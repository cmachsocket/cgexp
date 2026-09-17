# =============================================================================
#  CUBE RUNNER —— 计算机图形学实验（4 次课 × 4 学时）
#
#  一次配置，构建全部 8 个可执行目标：
#     lab{1..4}_starter    学生起步版（有 TODO 挖空，能编译能运行）
#     lab{1..4}_solution   教师参考答案（完整功能）
#
#  用法（命令行）：
#     cmake -B build && cmake --build build -j
#  用法（Visual Studio / CLion）：直接"打开文件夹"，IDE 会自动识别。
#
#  GLFW 依赖解析顺序：
#     1. 系统已安装的 glfw3（apt/brew/vcpkg 装过就直接用）
#     2. 自动从 GitHub 下载源码构建（首次配置需要网络，之后有缓存）
# =============================================================================
cmake_minimum_required(VERSION 3.24)
project(cube_runner LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 依赖：GLFW ---
find_package(glfw3 3.3 QUIET)
if(glfw3_FOUND)
    message(STATUS "GLFW: 使用系统安装版本")
else()
    message(STATUS "GLFW: 系统未安装，将从 GitHub 下载源码构建（需要网络）")
    include(FetchContent)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(glfw)
endif()

# --- 共享框架库：GL 加载器 + 窗口/着色器/网格封装 ---
add_library(framework STATIC
    common/glcore/glcore.cpp
    common/framework/app.cpp
    common/framework/shader.cpp
    common/framework/mesh.cpp
)
target_include_directories(framework PUBLIC
    common/glcore
    common/framework
)
target_link_libraries(framework PUBLIC glfw)
if(UNIX AND NOT APPLE)
    target_link_libraries(framework PUBLIC dl)
endif()

# --- 8 个实验目标 ---
function(add_lab name dir)
    add_executable(${name} ${dir}/src/main.cpp)
    target_include_directories(${name} PRIVATE ${dir}/src)
    target_link_libraries(${name} PRIVATE framework)
    if(MSVC)
        target_compile_options(${name} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra)
    endif()
endfunction()

add_lab(lab1_starter  lab1_character/starter)
add_lab(lab1_solution lab1_character/solution)
add_lab(lab2_starter  lab2_world/starter)
add_lab(lab2_solution lab2_world/solution)
add_lab(lab3_starter  lab3_walk/starter)
add_lab(lab3_solution lab3_walk/solution)
add_lab(lab4_starter  lab4_actions/starter)
add_lab(lab4_solution lab4_actions/solution)
