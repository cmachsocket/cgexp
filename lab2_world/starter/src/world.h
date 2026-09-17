// ============================================================================
//  world.h — 《CUBE RUNNER》第 2 周：新手村"方块镇"
//
//  世界由一批"道具（Prop）"组成：每个道具 = 形状 + 模型矩阵 + 颜色。
//  模型矩阵在 build() 里用你第 1 周写的 translate/rotate/scale 组合而成——
//  这就是"场景建模"：不造新几何体，用变换把旧几何体摆出一个世界。
//
//  工单 SD-201：完成 addTree / addHouse / addCrate（组合模型矩阵）
//  工单 SD-202：在着色器里实现 Phong 光照（见 main.cpp 的片段着色器）
// ============================================================================
#pragma once

#include "mgl.h"

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
    float groundSize = 40.0f;

    // ------------------------------------------------------------------
    // ====== TODO(明宇) 工单 SD-201-a：一棵树 ======
    // 美术组（不存在的）给的规格书：
    //   树干：立方体，截面 0.3×0.3，高 1.2，底面贴地   颜色 (0.45, 0.30, 0.16)
    //   下层树冠：四棱锥，底 1.6×1.6，高 1.4，从 y=1.0 开始  颜色 (0.16, 0.50, 0.22)
    //   上层树冠：四棱锥，底 1.1×1.1，高 1.1，从 y=1.8 开始  颜色 (0.20, 0.58, 0.26)
    //
    // 提醒 1：单位立方体的中心在原点——想让它"底面贴地、总高 1.2"，
    //         要先 scale 到 1.2 高，再 translate 到 y = 1.2/2 = 0.6。
    // 提醒 2：单位四棱锥的底面本来就在 y=0，直接平移到起始高度即可。
    // 提醒 3：矩阵乘法顺序——translate(...) * scale(...) 表示"先缩放再平移"。
    // ------------------------------------------------------------------
    void addTree(float x, float z) {
        using namespace mgl;
        (void)x; (void)z;
        // props.push_back({Shape::Cube,    ??? , {0.45f, 0.30f, 0.16f}});
        // props.push_back({Shape::Pyramid, ??? , {0.16f, 0.50f, 0.22f}});
        // props.push_back({Shape::Pyramid, ??? , {0.20f, 0.58f, 0.26f}});
    }

    // ------------------------------------------------------------------
    // ====== TODO(明宇) 工单 SD-201-b：一间房 ======
    // 规格书：
    //   房体：立方体 2.4 宽 × 1.8 高 × 2.0 深，底面贴地   颜色 (0.87, 0.78, 0.62)
    //   屋顶：四棱锥 2.9×1.3×2.5，坐在房体正上方(y=1.8)    颜色 (0.75, 0.28, 0.22)
    //   门：  立方体薄片 0.55×1.1×0.04，贴在 +Z 面(z=1.01) 颜色 (0.42, 0.27, 0.14)
    //   整间房可绕 Y 轴旋转 yawDeg 度后平移到 (x, 0, z)
    //
    // 关键思路：先造一个"基座矩阵" base = T(x,0,z) · Ry(yaw)，
    //           三个部件都乘在 base 后面——它们就会作为整体一起转、一起移。
    //           这正是第 1 周层级建模的复用！
    // ------------------------------------------------------------------
    void addHouse(float x, float z, float yawDeg) {
        using namespace mgl;
        (void)x; (void)z; (void)yawDeg;
        // mat4 base = ??? ;
        // props.push_back({Shape::Cube,    base * ??? , {0.87f, 0.78f, 0.62f}});
        // props.push_back({Shape::Pyramid, base * ??? , {0.75f, 0.28f, 0.22f}});
        // props.push_back({Shape::Cube,    base * ??? , {0.42f, 0.27f, 0.14f}});
    }

    // ------------------------------------------------------------------
    // ====== TODO(明宇) 工单 SD-201-c：木箱 ======
    // 规格书：边长 size 的立方体，底面贴地，绕 Y 转 yawDeg 度，
    //         摆到 (x, ?, z)。颜色 (0.72, 0.53, 0.30)。
    // 想一想：这次 translate、rotateY、scale 三个矩阵的先后顺序是什么？
    //         如果把 rotateY 放在 translate 前面（最左边），会发生什么惨剧？
    // ------------------------------------------------------------------
    void addCrate(float x, float z, float size, float yawDeg) {
        using namespace mgl;
        (void)x; (void)z; (void)size; (void)yawDeg;
        // props.push_back({Shape::Cube, ??? , {0.72f, 0.53f, 0.30f}});
    }

    // ------------------------------------------------------------------
    // 方块镇的"关卡设计"（策划：就先随便摆摆，能跑就行）
    // 这部分不用改——你把上面三个函数写对，镇子自己就会长出来。
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
        addCrate(2.4f, 3.2f, 0.7f, 45.0f); // 策划说箱子要堆着才像仓库
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
