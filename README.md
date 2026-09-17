# 《CUBE RUNNER》—— 计算机图形学实验

> 四次实验课，做出一个完整的 OpenGL 3D 游戏 Demo。
>
> 你将扮演 **明宇**——「星穹互娱（StarDome Games）」的 C++ 实习生，
> 在导师**老周**和策划**苏苏**的（不断）催促下，用四周时间从零打造
> Game Jam 参赛作品《CUBE RUNNER：方块小宇》。

## 课程总览（16 学时 / 4 次课）

| 周 | 实验 | 剧情 | 核心知识点 | 学生实现（挖空）|
|---|------|------|-----------|----------------|
| 1 | [lab1_character](lab1_character/guide.md) | 入职第一天：把机器人画到屏幕上 | 渲染管线、VAO/VBO、GLSL、齐次坐标、**几何变换**、层级建模 | translate/scale/rotateX·Y·Z、四肢的层级矩阵组合 |
| 2 | [lab2_world](lab2_world/guide.md) | 无中生有：搭出方块镇 | 场景建模、**Phong 光照**、法线矩阵、纹理映射 | 树/房/箱的模型矩阵组合、光照三分量 GLSL |
| 3 | [lab3_walk](lab3_walk/guide.md) | 让明宇动起来：行走与镜头 | **观察变换 lookAt**、**透视投影**、deltaTime、相机相对移动 | lookAt、perspective、Player 移动、第三人称跟随相机 |
| 4 | [lab4_actions](lab4_actions/guide.md) | 注入灵魂：跑、跳与最终交付 | 动画状态机、程序化动画、跳跃运动学、AABB 碰撞 | Run/Jump 姿态、重力积分、aabbIntersect |

每个 lab 目录包含：

```
labN_xxx/
├── guide.md     实验指导书（沉浸式剧情 + 知识讲解 + 分步任务 + 验收清单 + 选做挑战）
├── slides.md    上课 PPT（Marp 格式，可导出 pptx/pdf，见下）
├── starter/     学生起步工程：TODO 挖空版，可直接编译运行（表现"不对劲"等你修）
└── solution/    教师参考答案：完整可运行
```

**渐进式设计**：lab N 的 starter 基于 lab N-1 的 solution 演进——四次课的成果
最终汇成同一个游戏。上周挖空的函数（如几何变换）在下周以完成状态出现，
新的挖空点（如观察变换）接力登场。

## 环境要求

- **CMake ≥ 3.24**
- 支持 **C++20** 的编译器：VS 2022 / gcc 11+ / clang 14+ / Xcode 14+
- 支持 **OpenGL 4.1 Core Profile** 的显卡与驱动（2012 年后的设备均可，含 macOS）
- GLFW：已安装（apt/brew/vcpkg）则直接使用；否则首次配置时自动从 GitHub 下载（需网络）

```bash
# Ubuntu/Debian 推荐先装系统 GLFW，免下载：
sudo apt install libglfw3-dev
# macOS：
brew install glfw
```

## 构建与运行

```bash
cmake -B build
cmake --build build -j

# 运行（以第 1 周为例）
./build/lab1_starter     # 学生起步版
./build/lab1_solution    # 参考答案
```

Visual Studio：文件 → 打开 → 文件夹，选中本目录即可（自动识别 CMake）。
CLion：直接 Open 本目录。

## PPT 导出

`slides.md` 为 [Marp](https://marp.app/) 格式，改完 Markdown 一键出片：

```bash
# 需要 Node.js
npx @marp-team/marp-cli lab1_character/slides.md --pptx   # 导出 PowerPoint
npx @marp-team/marp-cli lab1_character/slides.md --pdf    # 导出 PDF
```

或安装 VS Code 插件 **Marp for VS Code**，实时预览、右键导出。

## 目录结构

```
cgexp/
├── CMakeLists.txt        一次配置构建全部 8 个目标（4 starter + 4 solution）
├── common/
│   ├── glcore/           迷你 OpenGL 4.1 函数加载器（自研，代替 glad/glew）
│   └── framework/        窗口/输入/着色器/网格封装（引擎组出品，学生不必修改）
├── lab1_character/       第 1 周：人物建模 + 几何变换
├── lab2_world/           第 2 周：场景建模 + 光照
├── lab3_walk/            第 3 周：行走 + 观察变换
└── lab4_actions/         第 4 周：动作系统 + 碰撞
```

## 教学设计说明（给教师）

- **挖空即时反馈**：starter 可编译可运行，但挖空处使用"无害占位"（单位矩阵、
  恒 false 等），造成明确可见的"惨状"（机器人压成一坨/世界漆黑/穿墙）。
  学生每补全一个函数，画面立即改善——正反馈驱动。
- **工单编号体系**：挖空点均标注 `====== TODO(明宇) 工单 SD-xxx ======`，
  与 guide.md 的任务、验收清单一一对应，方便课堂巡查与验收。
- **验收含口试**：每周验收清单最后 1~2 条为概念口试（如"为什么平移需要齐次
  坐标"），防止照抄网上代码。
- **分层拓展**：每份指导书末尾的"老周的加班挑战"按 ☆ 难度分级，
  供学有余力的学生深入（任意轴旋转、四元数、Blinn-Phong、镜头避障、
  平台跳跃机制等）。
- 原大纲的"贝塞尔曲线与分形图形"模块已按新方案移除，学时并入游戏项目主线。

## 许可

见 [LICENSE](LICENSE)。
