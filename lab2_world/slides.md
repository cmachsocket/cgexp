---
marp: true
theme: gaia
paginate: true
backgroundColor: #1a1a2e
color: #eaeaea
style: |
  section { font-family: "Noto Sans CJK SC", "Microsoft YaHei", sans-serif; }
  h1, h2 { color: #7ec8ff; }
  strong { color: #ffd166; }
  code { background: #16213e; color: #a8e6cf; }
  pre code { background: #16213e; }
  table { font-size: 0.8em; }
  blockquote { border-left: 4px solid #ffd166; color: #c9c9c9; }
---

<!-- _class: lead -->

# 《CUBE RUNNER》开发日志 · 第 2 周
## 无中生有：搭出方块镇

**计算机图形学实验（二）** ｜ 4 学时

场景建模 · 变换组合复用 · Phong 光照 · 法线矩阵 · 纹理映射

---

## 剧情开场：策划入职

工位上出现一杯奶茶和一张便利贴：

> "我是新策划**苏苏**。主角超可爱！
> 但他为什么站在**虚空**里？？
> 关卡设计稿见附件，本周求个新手村～"

《方块镇设计稿 v0.1》全文：

> 要有树（若干）。要有房子（三间，别排成一排，丑）。
> 要有木箱（堆在仓库区）。要有草地。要有阳光。
> **要有生活的气息。**

---

## 老周的需求翻译

| 苏苏说 | 技术上是 |
|--------|---------|
| 要有树/房/箱 | **场景建模**：用 T·R·S 摆放几何体（工单 SD-201）|
| 要有阳光 | **Phong 光照模型**（工单 SD-202）|
| 要有草地 | **纹理映射**（程序生成棋盘格）|
| 生活的气息 | 玄学，但光照占 80% |

> 老周："现在引擎里没有光，所有东西都是死人脸色。
> 运行 lab2_starter 看看——一片昏暗的空草地。"

---

## 场景建模 = 变换的批量艺术

美术资产依旧只有三件：**立方体、平面、四棱锥**

但一个变换矩阵 = 一次"摆放"：

```
一棵树 = 树干(瘦高立方体) + 两层树冠(四棱锥)
一间房 = 房体(立方体) + 屋顶(四棱锥) + 门(薄片)
一个镇 = 6树 + 3房 + 4箱 + 灵感
```

```cpp
struct Prop { Shape shape; mat4 model; vec3 color; };
// 渲染：for(prop) { setMat4("uModel"); draw(); }
```

**几何体只有一份，实例千姿百态**——实例化思想的雏形

---

## 第一坑：贴地问题（几何体的锚点）

单位**立方体**：中心在原点
→ 高 h 的箱子贴地：`translate(x, h/2, z) * scale(w, h, d)`

单位**四棱锥**：底面已在 y=0
→ 直接平移到起始高度：`translate(x, y0, z) * scale(...)`

```
     ┌──┐ ← 立方体：要抬 h/2       ▲  ← 四棱锥：直接放
     │  │                         ╱ ╲
──────┴──┴──────地面──────────────┴───┴──────
```

**想清楚锚点在哪 = 消灭 90% 的场景 bug**

---

## 整体旋转：基座矩阵（层级建模复用）

苏苏：房子别排成一排 → 每间房转个角度
房体/屋顶/门必须**作为整体**转 → 上周的族谱思想：

```cpp
mat4 base = translate(x, 0, z) * rotateY(radians(yawDeg));

房体 = base * translate(0, 0.9, 0) * scale(2.4, 1.8, 2.0);
屋顶 = base * translate(0, 1.8, 0) * scale(2.9, 1.3, 2.5);
门   = base * translate(0, 0.55, 1.01) * scale(0.55, 1.1, 0.04);
```

课堂快问：`rotateY` 挪到 `translate` 左边，房子会怎样？
（提示：绕世界原点公转，飞出镇外）

---

## 光是什么：Phong 反射模型（1975）

真实光照 = 光线弹射无数次（电影 CG，一帧渲几小时）
游戏等不起 → 用三个分量**骗过人眼**：

$$
color = (ambient + diffuse) \cdot base + specular
$$

| 分量 | 公式 | 模拟什么 |
|------|------|---------|
| ambient 环境光 | $k_a \cdot C_{light}$ | 四面八方的杂散光（保底亮度）|
| diffuse 漫反射 | $k_d \cdot \max(N \cdot L, 0) \cdot C_{light}$ | 表面越正对光越亮 |
| specular 高光 | $k_s \cdot \max(V \cdot R, 0)^n \cdot C_{light}$ | 反射对准眼睛的亮斑 |

---

## Phong 的四个向量（全部要 normalize！）

```
        N (法线)
         ▲    R (反射方向)
    L    │   ↗
     ↖   │  ╱        V = 指向相机
      ↖  │ ╱      ↗
────────┴────────── 表面
```

```glsl
vec3 N = normalize(vNormal);
vec3 L = normalize(-uLightDir);        // 指向光源！
vec3 V = normalize(uViewPos - vWorldPos);
vec3 R = reflect(-L, N);               // GLSL 内置函数
```

n = shininess：**越大亮斑越小越锐**（塑料 32，金属 256+）

---

## 法线的隐秘角落：法线矩阵

法线是"方向"不是"点"——不能直接乘模型矩阵！

非均匀缩放的反例：把球横向拉长
→ 法线若跟着拉长，就**不再垂直**于表面了

正确姿势（顶点着色器里已写好，但要懂**为什么**）：

```glsl
vNormal = transpose(inverse(mat3(uModel))) * aNormal;
```

**逆转置矩阵**：对旋转部分等价于原矩阵，对缩放部分取倒数
——正好抵消掉非均匀缩放对垂直性的破坏（考点预警）

---

## 纹理映射一分钟入门

地面的草地格子**不是图片文件**——代码现场生成 256×256 棋盘格：

```cpp
GLuint tex = fw::makeCheckerTexture(256, 8,
                 96,160,80,   // 浅草绿
                 76,138,66);  // 深草绿
```

采样三件套：
1. 顶点自带 **UV 坐标**（0~1，人话：贴纸上的位置）
2. 片段着色器 `texture(uTex, vUV * 平铺倍数)`
3. GPU 自动做双线性过滤 + mipmap（`glGenerateMipmap`）

想换真实草皮照片？→ 加班挑战 ☆☆☆（stb_image）

---

## 今天的惨状与拯救路线

运行 `lab2_starter`：昏暗空草地 + 孤零零的小宇

| 工单 | 文件 | 写完后 |
|------|------|--------|
| SD-201-a `addTree` | world.h | 6 棵树拔地而起 |
| SD-201-b `addHouse` | world.h | 3 间房各有朝向 |
| SD-201-c `addCrate` | world.h | 仓库区箱堆成型 |
| SD-202 Phong | main.cpp 片段着色器 | **世界活了** |

SD-202 递进体验：
0.3 占位=阴间 → +ambient=微亮 → +diffuse=**立体感降临** → +specular=材质感

---

## 动手任务（工单发布）

**推荐顺序**：SD-201（先把镇子摆出来）→ SD-202（再点亮它）

规格书全部印在 TODO 注释里，照着组合矩阵即可

**验收技巧**：
- 写一个函数编译一次，看着镇子逐步长大
- 按 **L** 让太阳转起来——光影流动 = Phong 三件套全对
- 按 **TAB** 线框模式，检查几何体有没有穿插

**经典翻车速查**：
全黑 → L 方向反了 ｜ 高光糊一片 → 忘了 pow ｜ 局部亮瞎 → 忘了 normalize

---

## 验收清单

1. ☐ 树/房/箱正确落地：不悬空、不入土
2. ☐ 三间房整体旋转不散架
3. ☐ 按 L：明暗随太阳平滑流动，屋顶有高光斑
4. ☐ 说清楚三个光照分量各自模拟什么
5. ☐ 说清楚法线为什么要乘逆转置矩阵

**加班挑战**：昼夜循环 ☆ ｜ 点光源路灯 ☆☆ ｜ stb_image 真纹理 ☆☆☆ ｜ Blinn-Phong ☆☆☆☆

---

<!-- _class: lead -->

## 下周预告

> 苏苏："镇子好看！但小宇像个雕像杵在广场中央，
> NPC 感太强了。"

下周一早，引擎组将发出那封著名的邮件：
**"临时支援 API 已回收"**

lookAt 和 perspective 将变成空壳
相机的数学，从此归你管

**第 3 周：让明宇动起来——行走与观察变换**
（剧透：所谓相机，根本不存在）
