// ============================================================================
//  world.h — 《CUBE RUNNER》第 2 周：新手村"方块镇"
//
//  世界由一批"道具（Prop）"组成：每个道具 = 形状 + 模型矩阵 + 颜色。
//  模型矩阵在 build() 里用你第 1 周写的 translate/rotate/scale 组合而成——
//  这就是"场景建模"：不造新几何体，用变换把旧几何体摆出一个世界。
//
//  工单 SD-201：完成 addTree / addHouse / addCrate（组合模型矩阵）✔ 第 2 周完成
//  工单 SD-202：在着色器里实现 Phong 光照 ✔ 第 2 周完成
//
//  第 4 周新增：每个道具登记一个 AABB 碰撞盒（colliders），
//  策划终于受不了小宇穿墙了。AABB 求交见 physics.h（工单 SD-403）。
// ============================================================================
#pragma once

#include "mgl.h"
#include "physics.h"

#include <mesh.h>
#include <shader.h>

#include <vector>

enum class Shape { Cube, Pyramid };

struct Prop {
    Shape shape;
    mgl::mat4 model;
    mgl::vec3 color;
};

class World {
public:
    std::vector<Prop> props;
    std::vector<AABB> colliders; // 第 4 周新增：碰撞盒
    float groundSize = 40.0f;

    // ------------------------------------------------------------------
    // 一棵树 = 树干（细高立方体）+ 两层树冠（四棱锥）
    // ------------------------------------------------------------------
    void addTree(float x, float z) {
        using namespace mgl;
        // 树干：截面 0.3×0.3，高 1.2，底部贴地 → 中心在 y=0.6
        props.push_back({Shape::Cube,
                         translate(x, 0.6f, z) * scale(0.30f, 1.2f, 0.30f),
                         {0.45f, 0.30f, 0.16f}});
        // 下层树冠：底面 1.6×1.6，高 1.4，从 y=1.0 开始
        props.push_back({Shape::Pyramid,
                         translate(x, 1.0f, z) * scale(1.6f, 1.4f, 1.6f),
                         {0.16f, 0.50f, 0.22f}});
        // 上层树冠：小一号，从 y=1.8 开始，颜色浅一点
        props.push_back({Shape::Pyramid,
                         translate(x, 1.8f, z) * scale(1.1f, 1.1f, 1.1f),
                         {0.20f, 0.58f, 0.26f}});
        // 碰撞盒：只挡树干（树冠随便撞，策划说"有弹性"）
        colliders.push_back(AABB::fromCenter({x, 0.6f, z}, {0.30f, 1.2f, 0.30f}));
    }

    // ------------------------------------------------------------------
    // 一间房 = 房体（立方体）+ 屋顶（四棱锥），可整体绕 Y 旋转
    // 注意乘法顺序：先把部件拼好，再整体旋转、最后平移到位
    // ------------------------------------------------------------------
    void addHouse(float x, float z, float yawDeg) {
        using namespace mgl;
        mat4 base = translate(x, 0.0f, z) * rotateY(radians(yawDeg));
        // 房体：2.4 宽 × 1.8 高 × 2.0 深，底部贴地
        props.push_back({Shape::Cube,
                         base * translate(0, 0.9f, 0) * scale(2.4f, 1.8f, 2.0f),
                         {0.87f, 0.78f, 0.62f}});
        // 屋顶：比房体大一圈，坐在房体顶上
        props.push_back({Shape::Pyramid,
                         base * translate(0, 1.8f, 0) * scale(2.9f, 1.3f, 2.5f),
                         {0.75f, 0.28f, 0.22f}});
        // 门：贴在 +Z 面上的一块薄片
        props.push_back({Shape::Cube,
                         base * translate(0, 0.55f, 1.01f) * scale(0.55f, 1.1f, 0.04f),
                         {0.42f, 0.27f, 0.14f}});
        // 碰撞盒：房子旋转后仍用轴对齐盒近似——取旋转前包围盒的外接尺寸
        // （AABB 的 A 就是 Axis-aligned：偷懒但够用，精确的 OBB 是选做挑战）
        float c = std::fabs(std::cos(radians(yawDeg)));
        float s = std::fabs(std::sin(radians(yawDeg)));
        float ex = 2.4f * c + 2.0f * s, ez = 2.4f * s + 2.0f * c;
        colliders.push_back(AABB::fromCenter({x, 0.9f, z}, {ex, 1.8f, ez}));
    }

    // ------------------------------------------------------------------
    // 木箱：边长 size 的立方体，可绕 Y 随意转个角度显得自然
    // ------------------------------------------------------------------
    void addCrate(float x, float z, float size, float yawDeg) {
        using namespace mgl;
        props.push_back({Shape::Cube,
                         translate(x, size * 0.5f, z) * rotateY(radians(yawDeg)) *
                             scale(size, size, size),
                         {0.72f, 0.53f, 0.30f}});
        colliders.push_back(
            AABB::fromCenter({x, size * 0.5f, z}, {size, size, size}));
    }

    // ------------------------------------------------------------------
    // 方块镇的"关卡设计"（策划：就先随便摆摆，能跑就行）
    // ------------------------------------------------------------------
    void build() {
        addTree(-6.0f, -4.0f);
        addTree(-8.5f, 2.0f);
        addTree(6.5f, -6.0f);
        addTree(9.0f, 3.5f);
        addTree(2.5f, -9.0f);
        addTree(-3.0f, 8.0f);

        addHouse(-7.0f, -10.0f, 15.0f);
        addHouse(8.0f, -2.0f, -30.0f);
        addHouse(-10.0f, 6.0f, 75.0f);

        addCrate(2.0f, 3.0f, 0.8f, 20.0f);
        addCrate(3.0f, 3.4f, 0.6f, -15.0f);
        addCrate(2.4f, 3.2f + 0.0f, 0.7f, 45.0f); // 策划说箱子要堆着才像仓库
        addCrate(-2.5f, -3.0f, 0.9f, 0.0f);
    }

    void draw(const fw::Shader& sh, const fw::Mesh& cube,
              const fw::Mesh& pyramid) const {
        for (const Prop& p : props) {
            sh.setMat4("uModel", p.model.data());
            sh.setVec3("uColor", p.color.x, p.color.y, p.color.z);
            (p.shape == Shape::Cube ? cube : pyramid).draw();
        }
    }
};
