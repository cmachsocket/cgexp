# 从固定管线到可编程管线 —— 《CUBE RUNNER》从零实现教程

> 写给只用过固定管线（`glBegin/glEnd`、`glTranslatef`、`glLightfv`）的读者。
> 目标：读完本教程，你能看懂并亲手复现这个项目（含 `common/` 框架）的每一行代码。
>
> 学法建议：**每章先读原理，再打开对应文件对照着读代码，最后做「动手一试」**。
> 不要跳章——后面的每一章都建立在前一章的心智模型上。

---

## 第 0 章 心智模型大迁移：GPU 不再替你干活了

### 0.1 你熟悉的固定管线在做什么

固定管线时代，OpenGL 是一台「功能齐全但功能固定」的自动售货机：

```c
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
glTranslatef(0, 0, -5);        //  OpenGL 内部帮你维护矩阵栈
glRotatef(angle, 0, 1, 0);     //  帮你算矩阵乘法
glLightfv(GL_LIGHT0, GL_POSITION, pos);  // 帮你算光照
glBegin(GL_TRIANGLES);         // 一个顶点一个顶点地喂
    glNormal3f(0, 0, 1);
    glVertex3f(0, 0, 0);
glEnd();
```

矩阵、光照、雾、纹理混合——全是 OpenGL **内部实现好**的，你只负责调参数。

### 0.2 可编程管线：自动售货机变成了裸机

现代 OpenGL（Core Profile，2008 年起）把这些「贴心服务」**全部删除**了：

| 固定管线 | 现代 OpenGL | 谁来补这个洞 |
|---|---|---|
| `glVertex3f` 逐点提交 | 顶点数组一次性传入显存（VBO/VAO） | 框架 `mesh.cpp` |
| `glTranslatef / glRotatef` | 没有矩阵！自己写数学库 | `mgl.h`（学生手写） |
| `gluLookAt / gluPerspective` | 自己构造观察/投影矩阵 | `mgl.h`（学生手写） |
| `glLightfv` 光照 | 在着色器里手算 Phong | lab2 的片段着色器 |
| 内部固定的逐顶点计算 | **顶点着色器**：你写程序决定每个顶点去哪 | 各 lab 的 vertex shader |
| 内部固定的逐像素着色 | **片段着色器**：你写程序决定每个像素什么颜色 | 各 lab 的 fragment shader |

核心一句话：**GPU 上跑着两段你亲手写的小程序（着色器），OpenGL 只剩「传数据、调度、画三角形」三件事。**

### 0.3 渲染管线：一个顶点的一生

```
CPU 端                                GPU 端
─────────                      ─────────────────────────────────
顶点数组 (VBO)  ──上传──▶  ① 顶点着色器：每个顶点跑一次
                                输入：位置/法线/UV + uniform(矩阵等)
                                输出：裁剪空间坐标 gl_Position
                                     │
                            ② 图元装配 + 裁剪 + 透视除法
                                三个顶点拼成三角形，屏幕外的丢掉
                                     │
                            ③ 光栅化：三角形 → 一堆像素（片元）
                                顶点属性在三角形内部**插值**
                                     │
                            ④ 片段着色器：每个像素跑一次
                                输入：插值后的法线/UV + uniform
                                输出：这个像素的颜色
                                     │
                            ⑤ 深度测试 + 混合 → 写入帧缓冲
```

对照固定管线：①④以前是 OpenGL 内部写死的电路，现在是你写的 GLSL 程序；
②③⑤是 GPU 硬件行为，不可编程，只需要理解。

### 0.4 本项目的代码地图

```
cgexp/
├── common/                  「引擎组」框架（可编程管线的体力活）
│   ├── glcore/              OpenGL 函数加载器            → 第 1 章
│   └── framework/
│       ├── app.{h,cpp}      窗口/主循环/输入/计时         → 第 1、8 章
│       ├── mesh.{h,cpp}     顶点数据 → VAO/VBO/EBO       → 第 2 章
│       └── shader.{h,cpp}   GLSL 编译/链接/uniform       → 第 3 章
├── lab1_character/  变换矩阵 + 层级建模                    → 第 4、5 章
├── lab2_world/      Phong 光照 + 纹理                     → 第 6 章
├── lab3_walk/       lookAt/perspective + 主循环/输入      → 第 7、8 章
└── lab4_actions/    动画状态机 + 跳跃 + 碰撞              → 第 9 章
```

---

## 第 1 章 开门第一件事：窗口、上下文与函数加载

> 对应代码：`common/framework/app.cpp`、`common/glcore/`

### 1.1 为什么不能直接开始画

OpenGL 本身**不管窗口**——它只是一个渲染规范。在屏幕上开窗口、接收键盘鼠标，
是操作系统的事。固定管线时代你可能用过 GLUT；本项目的替代品是 **GLFW**
（跨平台、仍在维护、工业界主流）。

### 1.2 Core Profile 与「上下文」

`app.cpp:17-19`：

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
```

这三行在开窗**之前**告诉 GLFW：「给我一个 OpenGL 4.1 **核心模式**的上下文」。

- **上下文（context）**：OpenGL 的全部状态（当前绑定的缓冲、着色器、纹理……）
  都装在这个大状态机对象里。OpenGL 是一台巨大的状态机，这是理解一切 API 的钥匙。
- **Core Profile**：固定管线函数（`glBegin`、`glTranslatef`……）在这个模式下
  **不存在**，调用就报错——从制度上强迫你学现代写法。
- 选 4.1 是因为这是 macOS 支持的最高版本，全平台课程项目的最大公约数。

### 1.3 glcore：OpenGL 函数为什么要「加载」？

`common/glcore/glcore.h` 开头的注释已经点破：

> OpenGL 的函数并不在头文件里，而是藏在显卡驱动中，必须在运行时逐个"查询函数地址"。

Windows/Linux 的系统库只暴露 OpenGL 1.1 的老函数，其余函数的地址要问驱动要。
固定管线时代你用不到新函数所以没感觉；现代写法**所有函数都要自己加载**。
工业界用 glad/glew 库自动生成，本项目手写了一个迷你版（`glcore.h`，70 行），
原理一目了然：

```cpp
// glcore.cpp:12 —— 对每个函数名，问 GLFW 要地址，存进函数指针
glcore_##name = reinterpret_cast<type>(loader(#name));
```

`loader` 就是 `glfwGetProcAddress`。加载完成后用宏把 `glcore_glClear`
映射回标准名 `glClear`，之后代码就和教科书上一模一样了。

### 1.4 主循环骨架

```cpp
fw::App app{"标题", 1280, 720};       // 开窗 + 建上下文 + 加载函数
while (app.beginFrame()) {            // 窗口没关就继续
    // 1. 处理输入（键盘/鼠标）
    // 2. 计算这一帧的矩阵
    // 3. 发绘制命令
    app.endFrame();                   // 交换双缓冲：把画好的一帧放上屏幕
}
```

- **双缓冲**：GPU 在「后缓冲」画画，画完 `glfwSwapBuffers` 瞬间交换到前台。
  没有它，你会看到画了一半的画面（闪烁）。固定管线里 GLUT 也替你做过，
  现在轮到你自己调用。
- **垂直同步**（`app.cpp:32` `glfwSwapInterval(1)`）：交换等待显示器的刷新信号，
  帧率锁定 60Hz，画面不撕裂。
- **深度测试**（`app.cpp:43` `glEnable(GL_DEPTH_TEST)`）：每个像素记录深度，
  近的挡住远的。这就是固定管线里的 `glEnable(GL_DEPTH_TEST)`，一字未改。

> **动手一试**：在 `app.cpp:85` 把 `glClearColor` 的颜色改成 `(1, 0, 0, 1)`，
> 重新编译，整个天空变红——你刚刚修改了每一帧的第一条渲染命令。

---

## 第 2 章 顶点数据：从 glVertex3f 到 VBO/VAO

> 对应代码：`common/framework/mesh.cpp`

### 2.1 为什么不再一个顶点一个顶点地喂

`glBegin/glVertex3f/glEnd` 每画一个顶点都是一次 CPU→GPU 的函数调用，
一万个顶点就是三万次调用——CPU 被活活拖死。现代做法：
**把成千上万个顶点打包成一个数组，一次性传进显存，之后每帧只需说一句
「画第 1 号缓冲区里那 36 个索引」。**

### 2.2 三个核心对象

读 `mesh.cpp` 的构造函数（`mesh.cpp:7-37`），四步缺一不可：

**① VBO（顶点缓冲对象）——数据本体**

```cpp
glGenBuffers(1, &vbo_);                        // 向 GPU 申请一个缓冲区编号
glBindBuffer(GL_ARRAY_BUFFER, vbo_);           // 把它绑到"顶点数组"插槽上
glBufferData(GL_ARRAY_BUFFER, 字节数, 数据指针, GL_STATIC_DRAW);  // 复制进显存
```

注意 `glBindBuffer` 的语义：OpenGL 是状态机，几乎所有函数都不直接传对象，
而是「先绑定，再操作当前绑定的那个」。这是从固定管线继承来的世界观。

**② EBO（索引缓冲）——哪三个顶点拼一个三角形**

立方体有 8 个角，但有 24 个顶点（`mesh.cpp:58` 注释解释了为什么：
**每个面法线不同，同一个角在三个面上要存三份**）。如果不用索引，
12 个三角形 × 3 = 36 个顶点要重复存储；有了 EBO，只存 24 个顶点 +
36 个「下标」，画的时候 GPU 按下标取顶点。

**③ VAO（顶点数组对象）——数据布局说明书**

VBO 里只是一串 float，GPU 怎么知道哪几个是位置、哪几个是法线？
`mesh.cpp:27-34`：

```cpp
// 每个顶点 8 个 float：|位置 xyz|法线 xyz|纹理 uv|
glVertexAttribPointer(0, 3, GL_FLOAT, ..., 步长32字节, 偏移0);   // 属性0：位置
glVertexAttribPointer(1, 3, GL_FLOAT, ..., 步长32字节, 偏移12);   // 属性1：法线
glVertexAttribPointer(2, 2, GL_FLOAT, ..., 步长32字节, 偏移24);   // 属性2：UV
```

属性编号 0/1/2 会和顶点着色器里的 `layout(location = 0/1/2)` 一一对应（第 3 章）。
VAO 把「哪份数据 + 怎么解释」这份说明书整体记下来，以后每次绘制只需绑定 VAO。

**④ 绘制**

```cpp
void Mesh::draw() const {
    glBindVertexArray(vao_);                                  // 拿出说明书
    glDrawElements(GL_TRIANGLES, indexCount_, ...);           // 一条命令画完
}
```

### 2.3 三种图元

`makeCube()` / `makePlane()` / `makePyramid()` 只是往上面的流程里填不同的
顶点数组。整个游戏的所有模型（机器人、树、房子、地面）都是这三种图元
**缩放、旋转、平移**出来的——这就是第 4、5 章的内容。

> **动手一试**：在 `makeCube()` 里把 `h` 改成 `1.0f`，重新编译，
> 所有物体变大 2 倍但比例关系不变——因为模型矩阵是在「单位立方体」上作用的。

---

## 第 3 章 着色器：你在 GPU 上写的第一段程序

> 对应代码：`common/framework/shader.cpp`、各 lab `main.cpp` 顶部的 GLSL 字符串

### 3.1 GLSL 长什么样

看 lab1 的顶点着色器（`lab1_character/solution/src/main.cpp:27-44`）：

```glsl
#version 410 core
layout(location = 0) in vec3 aPos;      // 对应 VAO 里的属性 0（位置）
layout(location = 1) in vec3 aNormal;   // 属性 1（法线）

uniform mat4 uModel, uView, uProj;      // CPU 每帧传进来的矩阵

out vec3 vNormal;                       // 传给片段着色器（会被插值）

void main() {
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);  // 图形学第一公式
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
}
```

和固定管线对照：

- `gl_Position = P·V·M·顶点` 就是以前 `GL_PROJECTION × GL_MODELVIEW × 顶点`
  干的事——只不过矩阵现在**你自己算、自己传、自己乘**。
- `in`：从 VAO 来的逐顶点输入；`uniform`：所有顶点共享的全局参数（每帧由 CPU 设置）；
  `out`：传给下一阶段，光栅化时会在三角形内部自动插值。

片段着色器（每个像素跑一次）：

```glsl
in vec3 vNormal;            // 插值后的法线——注意！是三个顶点法线的加权平均
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    float diff = max(dot(normalize(vNormal), L), 0.0);   // Lambert 余弦
    FragColor = vec4(uColor * (0.35 + 0.65*diff), 1.0);
}
```

`dot(N, L)` 就是固定管线 `GL_DIFFUSE` 光照内部算的 Lambert 余弦定律，
现在你亲手写它——**lab2 的工单 SD-202 就是把这个升级成完整 Phong**。

### 3.2 编译与链接：GPU 程序的「构建系统」

`shader.cpp` 做的事和你用 gcc 编译 C 程序完全同构：

```
glCreateShader + glShaderSource + glCompileShader   ≈  gcc -c main.c  →  main.o
glCreateProgram + glAttachShader + glLinkProgram    ≈  gcc main.o ...  →  a.out
glUseProgram                                        ≈  运行 a.out
```

编译失败时 `glGetShaderInfoLog` 能拿到带行号的报错——**调着色器报错的第一课：
先看控制台**。

### 3.3 uniform：CPU 与 GPU 的传参通道

```cpp
shader.use();                                    // 启用这套着色器
shader.setMat4("uModel", model.data());          // 按名字找到 uniform，传 16 个 float
```

`glGetUniformLocation(program_, name)` 按字符串查参数位置，
`glUniformMatrix4fv` 上传数据。你每帧算的矩阵就是这样流进 GPU 的。

> **动手一试**：把 lab1 片段着色器里的 `0.35 + 0.65 * diff` 改成 `1.0`，
> 机器人变成纯平涂色块——你刚刚亲手「关掉」了漫反射光照。

---

## 第 4 章 数学：齐次坐标与 MVP 矩阵

> 对应代码：`lab1_character/solution/src/mgl.h`、lab1 的 guide.md「知识加油站」

### 4.1 为什么是 4×4 矩阵

三维点 `(x,y,z)` 加一维变成 `(x,y,z,1)`——**齐次坐标**。
原因一句话：平移 `x' = x + tx` 不是线性变换，3×3 矩阵**表示不了**；
升到 4 维后，平移也能写成矩阵乘法：

```
| 1 0 0 tx |   | x |   | x+tx |
| 0 1 0 ty | × | y | = | y+ty |
| 0 0 1 tz |   | z |   | z+tz |
| 0 0 0 1  |   | 1 |   |  1   |
```

于是旋转、缩放、平移**统一成一种运算**，可以任意连乘复合——
这是整个计算机图形学的地基。`mgl.h` 里的 `translate/scale/rotateX/Y/Z`
（lab1 学生要手写的工单 SD-101）就是在填这些矩阵的 16 个数。

### 4.2 乘法顺序：图形学第一坑

矩阵乘法不满足交换律。`M = T · R · S` 作用在顶点 `v` 上是
**从右往左**生效：先缩放，再旋转，最后平移。

- `T·R·S`：物体绕**自己的中心**转完再搬走（正确：机器人原地转身）
- `R·T·S`：物体先搬走再绕**世界原点**转（错误：机器人绕着远处一个点公转）

guide.md 的验收口试第 5 条就是让学生现场演示写反的效果。

### 4.3 MVP：三级坐标系接力

```
模型空间 --模型矩阵M--> 世界空间 --观察矩阵V--> 相机空间 --投影矩阵P--> 裁剪空间
(部件在自己原点旁)      (机器人站在镇子里)     (以相机为原点)        (准备变成屏幕像素)
```

lab1 里 `M` 是学生写的（工单），`V` 和 `P` 暂时由框架提供；
到 lab3，`lookAt`（造 V）和 `perspective`（造 P）也要学生手写（第 7 章）。

> **动手一试**：在 lab1 的 `mgl.h` 里把 `rotateY` 的某个 `sin` 改成 `-sin`，
> 机器人转向立刻反了——矩阵里的每一个数都有肉眼可见的含义。

---

## 第 5 章 层级建模：机器人为什么不散架

> 对应代码：`lab1_character/solution/src/robot.h`

### 5.1 核心思想：矩阵连乘 = 父子关系

手臂不是独立放在世界里的——它的位置是
**「身体的矩阵 × 肩关节的平移 × 关节的旋转 × 自身的缩放」**。
这就是固定管线时代 `glPushMatrix/glPopMatrix` 干的事！现在的写法：

```cpp
mat4 body = translate(pos) * rotateY(yaw) * scale(...);
draw(cube, body);                                        // 画躯干

mat4 armR = body * translate(肩关节位置) * rotateZ(摆臂角) * scale(手臂尺寸);
draw(cube, armR);                                        // 画手臂
```

`body` 乘在最左边 ⇒ 身体怎么动，手臂跟着动（不散架）；
`rotateZ(摆臂角)` 乘在中间 ⇒ 手臂还能绕自己的肩关节旋转（能动）。
**层级建模 = 把父节点的矩阵传下去接着乘**，一棵树下来就是完整骨骼。

### 5.2 绕「关节点」而非「部件中心」

验收口试第 3 条：手臂必须绕**肩关节**转。秘诀在乘法顺序：
先 `translate(到关节)` 把原点挪到关节，再 `rotate`（绕新原点转），
最后 `translate(从关节到部件中心)` + `scale` 画出部件本身。
写反了就会出现「手臂绕着自己的中心原地打转」的鬼畜画面——
这正是 starter 挖空后呈现的「惨状」之一。

> **动手一试**：把 lab1 按 `3`（T-pose）再按 `4`（行走定格），
> 然后在 `robot.h` 里把某条胳膊的 `translate` 顺序调换，看灾难现场。

---

## 第 6 章 光照与纹理：让像素自己算颜色

> 对应代码：`lab2_world/solution/src/main.cpp` 的着色器、`world.h`、`mesh.cpp:158` 的 `makeCheckerTexture`

### 6.1 Phong 三剑客

固定管线里你写 `glLightfv(GL_LIGHT0, GL_AMBIENT/DIFFUSE/SPECULAR, ...)`，
OpenGL 内部算的就是 Phong 模型。lab2 的片段着色器把这三行公式摆在了明处：

```glsl
vec3 ambient  = 0.28 * lightColor;                      // ① 环境光：保底亮度
vec3 diffuse  = 0.75 * max(dot(N,L), 0) * lightColor;   // ② 漫反射：正对光源才亮
vec3 specular = 0.30 * pow(max(dot(V,R),0), 32) * lightColor;  // ③ 高光：反射对准眼睛
color = (ambient + diffuse) * baseColor + specular;
```

- **ambient** 模拟「光在墙壁间弹射无数次后的均匀照亮」——粗暴但有效的近似；
- **diffuse** 就是 Lambert 余弦：表面越正对光源越亮， matte（哑光）材质全靠它；
- **specular** 模拟镜面反射的高光斑，指数 32 控制光斑大小（越大越像金属/塑料）。
- 物理图像、推导和「为什么高光要算视线方向 V」，lab2 的 guide.md 讲得很细，
  配合 `L` 键转动太阳实时观察，是最好的教具。

### 6.2 法线矩阵：为什么法线不能乘模型矩阵

非均匀缩放（比如 x 方向压扁 2 倍）时，顶点乘 `M` 没问题，
但法线乘 `M` 会被「带歪」——不再垂直于表面。数学结论：
**法线要乘 `M` 左上 3×3 的逆转置矩阵** `transpose(inverse(mat3(uModel)))`。
这就是每个 lab 顶点着色器里那行的来历，也是验收口试题。
（均匀缩放时逆转置 ≈ 原矩阵，所以 lab1 里看不出差别；压扁一个立方体试试就露馅。）

### 6.3 纹理：从顶点颜色到贴图

固定管线的 `glTexImage2D` 在现代管线**原样保留**（`mesh.cpp:172-182`），
用法流程：生成纹理对象 → 绑定 → 上传像素 → 设置环绕/过滤参数 →
着色器里 `uniform sampler2D uTex; texture(uTex, uv)` 采样。
本项目的草地纹理是**程序生成**的棋盘格（`makeCheckerTexture`），
不需要任何图片文件。UV 超出 0~1 时 `GL_REPEAT` 让它平铺——
配合着色器里的 `vUV * uUVScale`，一块地面铺出 20×20 的格子。

> **动手一试**：lab2 里把 `spec` 的指数 `32.0` 改成 `4.0`，
> 屋顶的高光斑会糊成一大片（塑料感 → 皮革感）。

---

## 第 7 章 相机：lookAt 与 perspective 手写推导

> 对应代码：`lab3_walk/solution/src/mgl.h:185-215`、`camera.h`

### 7.1 lookAt：把世界「搬」到相机眼里

固定管线的 `gluLookAt(eye, center, up)` 被删了，学生要自己写（工单 SD-302）。
思想：**观察矩阵 = 相机机位变换的逆**。相机在 `(ex,ey,ez)` 看向某个方向，
等价于世界整体反向平移 + 反向旋转：

1. 由 `center - eye` 得前方向 `f`；
2. `s = normalize(cross(f, up))` 得右方向，`u = cross(s, f)` 得修正上方向；
3. 用 `s, u, -f` 三个单位向量当行拼出旋转矩阵（正交矩阵的逆 = 转置，白送的）；
4. 再乘上平移 `(-eye)` ——所以口试答案是「**旋转的转置 + 平移的逆**」。

### 7.2 perspective：近大远小的数学

透视投影的核心操作是「**除以 z**」（远的压小）——但矩阵乘法里没有除法！
又一次靠齐次坐标救场：perspective 矩阵把 `-z` 偷偷塞进 `w` 分量，
之后 GPU 做「透视除法」（x/w, y/w, z/w）时，除以 z 的效果自动出现。
这就是口试题「perspective 矩阵里那个 -1 的作用」——它位于第 4 行第 3 列，
负责把 `-z` 搬进 `w`。

### 7.3 第三人称跟随相机

`camera.h` 用两个角（yaw 环绕角 + pitch 俯仰角）加一个距离，
把球坐标换算成相机位置（`eye = target + offset`），再 `lookAt(eye, target, up)`。
鼠标横向移动改 yaw、纵向改 pitch（加限位防止翻过头顶）——
固定管线时代你调 `gluLookAt` 的参数，现在参数流进你手写的矩阵，本质相同。

> **动手一试**：lab3 里把 `perspective` 的视场角 `radians(50.0)` 改成
> `radians(90.0)`，世界瞬间变成广角鱼眼——视场角就是「镜头的焦距」。

---

## 第 8 章 主循环：deltaTime 与输入处理

> 对应代码：`common/framework/app.cpp:52-105`、lab3/4 的 `main.cpp`

### 8.1 为什么移动要乘 dt

不同机器帧率不同（60Hz / 144Hz / 卡顿时 20Hz）。如果每帧固定走 0.1 米，
144Hz 的机器上角色速度快一倍多。正确姿势：

```cpp
位置 += 速度 * dt;    // dt = 上一帧的真实耗时（秒），速度单位是"米/秒"
```

`app.cpp:60-64` 负责测出 dt（还带了 0.1 秒上限的防卡顿保护）。
这个「位移 = 速度 × 时间」的积分思想，到 lab4 会升级成完整的重力积分。

### 8.2 键盘：按住 vs 刚按下

- `keyDown(W)`：W 此刻是否按住——适合「持续移动」（每帧查询，每帧走一点）；
- `keyPressed(SPACE)`：本帧是否**刚按下**（边沿触发）——适合「跳跃/切换」，
  实现就是 `cur && !prev`（`app.cpp:97`），框架存了上一帧的按键快照。

### 8.3 鼠标：捕获模式

`setMouseCaptured(true)` 隐藏光标并锁定在窗口内（FPS 游戏的标准操作），
之后每帧读「位移增量」`mouseDX/mouseDY` 而非绝对坐标，
增量直接喂给相机的 yaw/pitch。

---

## 第 9 章 动画、跳跃与碰撞：让规则自己跑起来

> 对应代码：`lab4_actions/solution/src/player.h`、`physics.h`

### 9.1 动画状态机

没有骨骼动画系统，四肢怎么动？**程序化动画**：每帧由代码直接算关节角。
状态机四个状态，转移条件一目了然（`player.h:158-162`）：

```
Idle ──按WASD──▶ Walk ──按住SHIFT──▶ Run
  ↑                                  │
  └────────── 松开按键 ◀─────────────┘
任何地面状态 ──按SPACE──▶ Jump ──落地──▶ 回到对应状态
```

每个状态就是一组正弦函数：`legSwing = sin(相位) × 摆幅`，
相位由累计路程驱动（走得快摆得也快），Run 的摆幅和频率都比 Walk 大。
窗口标题栏实时显示当前状态，方便验收。

### 9.2 跳跃：两行欧拉积分

```cpp
vy += gravity * dt;          // 速度积分：v = v + g·t
y  += vy * dt;               // 位置积分：y = y + v·t
```

起跳给一个向上的初速度 `vy = jumpSpeed`，之后重力每帧把 vy 往下拉，
自然形成抛物线；`y <= 0` 时落地清零。「只有 `onGround` 才能起跳」
这一条 if 就杜绝了二段跳。

### 9.3 AABB 碰撞

AABB（轴对齐包围盒）就是不旋转的长方体箱子。两个 AABB 相交的判定：
**三个轴全部重叠才算撞上**（分离轴定理的 3D 特例，几行代码）：

```cpp
bool aabbIntersect(a, b) {
    return a.min.x < b.max.x && a.max.x > b.min.x &&   // x 轴重叠
           a.min.y < b.max.y && a.max.y > b.min.y &&   // y 轴重叠
           a.min.z < b.max.z && a.max.z > b.min.z;     // z 轴重叠
}
```

移动时 **X、Z 轴分开试探**（`player.h:135-136`）：先试 X 方向，撞了就只取消 X；
再试 Z——效果是撞墙时能**沿墙滑动**而不是整个卡住，这就是商业游戏的标配手感。

> **动手一试**：lab4 跳上木箱试试——落地检测用的就是角色 AABB 与箱子 AABB
> 的碰撞结果。把 `jumpSpeed` 从 4.5 改成 6.0，能跳得比房顶还高。

---

## 第 10 章 建议的复现路径（自检清单）

按这个顺序亲手敲一遍，每步都能运行验证：

1. ☐ 用 GLFW 开一个窗口，清屏为天蓝色（第 1 章）——约 30 行
2. ☐ 封装 shader.cpp，画一个**无变换的三角形**（NDC 坐标直接输出）
3. ☐ 封装 mesh.cpp，画单位立方体（先不管矩阵，顶点着色器里写死一个缩放）
4. ☐ 手写 `mat4` 和 translate/scale/rotate，加上 MVP，让立方体转起来（lab1 核心）
5. ☐ 用层级矩阵拼出机器人，验证 1/2/3/4 四个姿势（lab1 完整）
6. ☐ 片段着色器实现 Phong 三分量 + 棋盘格纹理地面（lab2 完整）
7. ☐ 手写 lookAt/perspective，实现 WASD + 鼠标第三人称相机（lab3 完整）
8. ☐ 加状态机、重力跳跃、AABB 碰撞（lab4 完整）

每一步做不出来时，去翻对应 lab 的 **solution** 对照——但请先给自己 20 分钟。

## 附：从固定管线带来的好习惯，哪些还管用

- **矩阵直觉**（栈、连乘、顺序）——直接迁移，只是矩阵现在在你手里；
- **光照模型的物理图像**——Phong 公式原封不动，只是从黑盒搬到了你眼前；
- **深度测试/双缓冲/纹理参数**——API 几乎没变；
- **状态机世界观**（先绑定再操作）——现代 OpenGL 变本加厉地依赖它。

唯一需要「戒掉」的：遇到问题先想「OpenGL 有没有现成函数」。
现代 OpenGL 的答案是：**没有，但你自己写得出来——而且写一遍就懂了。**
