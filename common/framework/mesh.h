// ============================================================================
//  mesh.h — 星穹互娱引擎组：基础网格（顶点数据 → VAO/VBO/EBO）
//
//  美术组还没到岗，所以我们的全部美术资产就是三种图元：
//      makeCube()    单位立方体（边长 1，中心在原点）—— 万物之源
//      makePlane()   单位平面（XZ 平面上 1×1，法线朝 +Y）—— 大地
//      makePyramid() 单位四棱锥（底 1×1，高 1）—— 屋顶、树冠
//
//  顶点格式：位置(3) + 法线(3) + 纹理坐标(2)，共 8 个 float。
//  想知道 VAO/VBO 是怎么建的？看 mesh.cpp，那里每一行都有注释。
// ============================================================================
#pragma once

#include <glcore.h>

#include <vector>

namespace fw {

class Mesh {
public:
    // vertices: 每 8 个 float 一个顶点 (px,py,pz, nx,ny,nz, u,v)
    Mesh(const std::vector<float>& vertices,
         const std::vector<unsigned int>& indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;

    void draw() const;

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    GLsizei indexCount_ = 0;
};

Mesh makeCube();
Mesh makePlane();
Mesh makePyramid();

// 程序化纹理：美术组不在，我们自己"画"一张。
// 生成 size×size 的棋盘格纹理（两种 RGB 颜色交替），返回 GL 纹理对象。
GLuint makeCheckerTexture(int size, int cells,
                          unsigned char r1, unsigned char g1, unsigned char b1,
                          unsigned char r2, unsigned char g2, unsigned char b2);

} // namespace fw
