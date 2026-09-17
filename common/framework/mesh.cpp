#include "mesh.h"

#include <utility>

namespace fw {

Mesh::Mesh(const std::vector<float>& vertices,
           const std::vector<unsigned int>& indices)
    : indexCount_(GLsizei(indices.size())) {
    // 1. VAO：记录"顶点数据怎么解释"的状态对象
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // 2. VBO：把顶点数据搬进显存
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    // 3. EBO：索引数据（哪三个顶点拼一个三角形）
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 GLsizeiptr(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    // 4. 告诉 OpenGL 顶点的内存布局：|位置 xyz|法线 xyz|纹理 uv|
    const GLsizei stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

Mesh::Mesh(Mesh&& other) noexcept
    : vao_(std::exchange(other.vao_, 0)),
      vbo_(std::exchange(other.vbo_, 0)),
      ebo_(std::exchange(other.ebo_, 0)),
      indexCount_(std::exchange(other.indexCount_, 0)) {}

void Mesh::draw() const {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// 单位立方体：24 个顶点（每个面 4 个，法线不同所以不能共用）、36 个索引
// ---------------------------------------------------------------------------
Mesh makeCube() {
    const float h = 0.5f;
    const std::vector<float> v = {
        // 位置              法线            UV
        // +Z 前面
        -h, -h,  h,   0, 0, 1,   0, 0,
         h, -h,  h,   0, 0, 1,   1, 0,
         h,  h,  h,   0, 0, 1,   1, 1,
        -h,  h,  h,   0, 0, 1,   0, 1,
        // -Z 后面
         h, -h, -h,   0, 0, -1,  0, 0,
        -h, -h, -h,   0, 0, -1,  1, 0,
        -h,  h, -h,   0, 0, -1,  1, 1,
         h,  h, -h,   0, 0, -1,  0, 1,
        // +X 右面
         h, -h,  h,   1, 0, 0,   0, 0,
         h, -h, -h,   1, 0, 0,   1, 0,
         h,  h, -h,   1, 0, 0,   1, 1,
         h,  h,  h,   1, 0, 0,   0, 1,
        // -X 左面
        -h, -h, -h,  -1, 0, 0,   0, 0,
        -h, -h,  h,  -1, 0, 0,   1, 0,
        -h,  h,  h,  -1, 0, 0,   1, 1,
        -h,  h, -h,  -1, 0, 0,   0, 1,
        // +Y 顶面
        -h,  h,  h,   0, 1, 0,   0, 0,
         h,  h,  h,   0, 1, 0,   1, 0,
         h,  h, -h,   0, 1, 0,   1, 1,
        -h,  h, -h,   0, 1, 0,   0, 1,
        // -Y 底面
        -h, -h, -h,   0, -1, 0,  0, 0,
         h, -h, -h,   0, -1, 0,  1, 0,
         h, -h,  h,   0, -1, 0,  1, 1,
        -h, -h,  h,   0, -1, 0,  0, 1,
    };
    std::vector<unsigned int> idx;
    for (unsigned int f = 0; f < 6; ++f) {
        unsigned int b = f * 4;
        idx.insert(idx.end(), {b, b + 1, b + 2, b, b + 2, b + 3});
    }
    return Mesh{v, idx};
}

// ---------------------------------------------------------------------------
// 单位平面：XZ 平面上 1×1，法线 +Y。UV 取 0~1，平铺由着色器/纹理参数决定。
// ---------------------------------------------------------------------------
Mesh makePlane() {
    const float h = 0.5f;
    const std::vector<float> v = {
        -h, 0,  h,   0, 1, 0,   0, 0,
         h, 0,  h,   0, 1, 0,   1, 0,
         h, 0, -h,   0, 1, 0,   1, 1,
        -h, 0, -h,   0, 1, 0,   0, 1,
    };
    const std::vector<unsigned int> idx = {0, 1, 2, 0, 2, 3};
    return Mesh{v, idx};
}

// ---------------------------------------------------------------------------
// 单位四棱锥：底面 1×1（y=0），锥尖 (0,1,0)。侧面法线按面计算。
// ---------------------------------------------------------------------------
Mesh makePyramid() {
    const float h = 0.5f;
    // 每个侧面法线 = 归一化后的面法线。侧面斜率 1:0.5 → 法线 (0, 0.4472, 0.8944) 类推
    const float ny = 0.4472136f, nz = 0.8944272f;
    const std::vector<float> v = {
        // 底面（法线 -Y）
        -h, 0, -h,   0, -1, 0,   0, 0,
         h, 0, -h,   0, -1, 0,   1, 0,
         h, 0,  h,   0, -1, 0,   1, 1,
        -h, 0,  h,   0, -1, 0,   0, 1,
        // +Z 侧面
        -h, 0,  h,   0, ny,  nz,   0, 0,
         h, 0,  h,   0, ny,  nz,   1, 0,
         0, 1,  0,   0, ny,  nz,   0.5f, 1,
        // -Z 侧面
         h, 0, -h,   0, ny, -nz,   0, 0,
        -h, 0, -h,   0, ny, -nz,   1, 0,
         0, 1,  0,   0, ny, -nz,   0.5f, 1,
        // +X 侧面
         h, 0,  h,    nz, ny, 0,   0, 0,
         h, 0, -h,    nz, ny, 0,   1, 0,
         0, 1,  0,    nz, ny, 0,   0.5f, 1,
        // -X 侧面
        -h, 0, -h,   -nz, ny, 0,   0, 0,
        -h, 0,  h,   -nz, ny, 0,   1, 0,
         0, 1,  0,   -nz, ny, 0,   0.5f, 1,
    };
    const std::vector<unsigned int> idx = {
        0, 2, 1, 0, 3, 2,      // 底面（从下往上看逆时针）
        4, 5, 6,               // 四个侧面
        7, 8, 9,
        10, 11, 12,
        13, 14, 15,
    };
    return Mesh{v, idx};
}

GLuint makeCheckerTexture(int size, int cells,
                          unsigned char r1, unsigned char g1, unsigned char b1,
                          unsigned char r2, unsigned char g2, unsigned char b2) {
    std::vector<unsigned char> pixels(size_t(size) * size * 3);
    const int cell = size / cells;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool odd = ((x / cell) + (y / cell)) % 2;
            size_t i = (size_t(y) * size + x) * 3;
            pixels[i + 0] = odd ? r1 : r2;
            pixels[i + 1] = odd ? g1 : g2;
            pixels[i + 2] = odd ? b1 : b2;
        }
    }
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    return tex;
}

} // namespace fw
