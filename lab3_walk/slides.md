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

# 《CUBE RUNNER》开发日志 · 第 3 周
## 让明宇动起来：行走与镜头

**计算机图形学实验（三）** ｜ 4 学时

观察变换 lookAt · 透视投影 perspective · deltaTime · 相机相对移动 · 第三人称相机

---

## 剧情开场：那封著名的邮件

> **发件人：引擎组**
> **主题：[通知] 临时支援 API 回收**
>
> 各位：借出的 `mgl::lookAt` / `mgl::perspective`
> 临时实现已于今晨**回收**。相关项目请自行实现。
> 文档？我们相信你们的数学功底。祝好。

运行 `lab3_starter`：**一团马赛克粥**
没有观察和投影矩阵的世界 = 原点处的一锅乱炖

> 老周："每年都这一出。正好，今天我要告诉你一个
> 颠覆认知的事实——**相机这个东西，根本不存在**。"

---

## 本周工单

| 工单 | 内容 | 文件 |
|------|------|------|
| SD-302 | 手写 lookAt + perspective | `mgl.h` |
| SD-303 | 第三人称跟随相机（球面坐标）| `camera.h` |
| SD-301 | WASD → 世界空间移动 + 平滑转身 | `player.h` |

推荐顺序：**302 → 303 → 301**
（先让世界显形 → 再让镜头动 → 最后让人动）

策划验收标准（苏苏）：
"像《塞尔达》那样——镜头吊在身后，按 W 朝镜头前方走"

---

## 相机不存在：观察变换的本质

OpenGL 里**没有相机对象**。所谓"把相机移到 (0,2,5)"，实际是：

> **把整个世界朝反方向搬，
> 让"相机"永远呆在原点、看向 -Z**

```
你以为：      相机 ──▶ 移动到世界某处
实际上：      世界 ──▶ 被搬到相机跟前（逆变换）
```

所以观察矩阵 V = **相机位姿变换的逆**
= 旋转的逆（转置）+ 平移的逆（取负）

---

## lookAt 第 1 步：搭出相机的三根轴

给定 eye（相机位置）、center（看向哪）、up（世界上方向）：

```cpp
vec3 f = normalize(center - eye);   // 前 (front)
vec3 s = normalize(cross(f, up));   // 右 (side) = 前 × 上
vec3 u = cross(s, f);               // 真正的上 = 右 × 前
```

```
        u ▲
          │  ↗ f (看向那边)
          │ ╱
   ───────┼──────▶ s
         eye
```

三个都是**单位向量**且两两垂直——一套正交基

---

## lookAt 第 2 步：填矩阵

$$
V = \begin{pmatrix}
 s_x & s_y & s_z & -s\cdot eye \\
 u_x & u_y & u_z & -u\cdot eye \\
 -f_x & -f_y & -f_z & f\cdot eye \\
 0 & 0 & 0 & 1
\end{pmatrix}
$$

三个要点：

- s/u/-f 按**行**摆：正交矩阵的逆 = 转置
- 第 4 列是平移的逆：把 eye 搬回原点
- 第三行取 **-f**：OpenGL 相机看向自己的 **-Z**

**自测**：`lookAt({0,0,5},{0,0,0},{0,1,0})` 必须把原点映射到 (0,0,-5)

---

## 透视投影：把视锥体捏成方块

人眼看到的是**视锥体**（frustum）
perspective 把它捏成 NDC 标准立方体（各轴 -1~1）：

```
   眼睛 ▷────────┐            ┌──────┐
        ╲  远处大 │    捏！    │      │
         ╲ 物体多 │   ──▶     │ NDC  │
          ╲______│            └──────┘
        近小远大的锥         远处的东西被压小了
```

| 参数 | 直觉 |
|------|------|
| fovy 视场角 | 大=鱼眼广角，小=狙击镜。50° 舒适区 |
| aspect 宽高比 | 不跟窗口一致 → 小宇变胖/变瘦 |
| zNear / zFar | 可视范围。**zNear 千万别设 0** |

---

## perspective 矩阵：灵魂是那个 -1

记 t = 1/tan(fovy/2)：

$$
P = \begin{pmatrix}
 t/aspect & 0 & 0 & 0 \\
 0 & t & 0 & 0 \\
 0 & 0 & \frac{f+n}{n-f} & \frac{2fn}{n-f} \\
 0 & 0 & \mathbf{-1} & 0
\end{pmatrix}
$$

**-1 的作用**：把顶点的 -z（深度）抄送进 w 分量
硬件随后执行**透视除法** xyz ÷ w ——除的正是深度
→ 距离两倍，尺寸减半。"近大远小"就这么来的

⚠️ 从**全零矩阵**开始填（单位矩阵的 m[15]=1 会毁掉透视除法）

---

## deltaTime：运动的铁律

```cpp
robot.position += dir * walkSpeed * dt;   // ✔ 每秒 walkSpeed 米
robot.position += dir * 0.1f;             // ✘ 每帧 0.1 米
```

第二种写法的下场：

| 设备 | 帧率 | 实际速度 |
|------|------|---------|
| 办公本 | 60 Hz | 6 米/秒 |
| 电竞屏 | 144 Hz | **14.4 米/秒**（外挂警告）|

**一切运动都乘 dt**（框架已备好 `app.dt()`）

---

## 相机相对移动："前"是哪边？

玩家按 W 想的是"朝**镜头对着的方向**走"，不是世界 +Z

把相机水平朝向 camYaw 展开成两个水平基向量：

```cpp
vec3 front{ sin(camYaw), 0,  cos(camYaw) };   // 镜头的"前"
vec3 right{ -cos(camYaw), 0, sin(camYaw) };   // front 绕 Y 转 +90°

vec3 dir = normalize(front * fwd + right * side);
robot.position += dir * (walkSpeed * dt);
```

**必须 normalize**：W+D 斜着走否则是 √2 倍速（老玩家漂移技）

---

## 平滑转身：atan2 与 ±π 回绕

方向向量 → 朝向角：`targetYaw = atan2(dir.x, dir.z)`
瞬间掉头像触电 → 要**插值**过去

陷阱：从 170° 转到 -170°
直接插值会倒着转 **340°**，正确答案是顺着转 **20°**

```cpp
float diff = target - cur;
while (diff >  PI) diff -= 2*PI;   // 归到 [-π, π]
while (diff < -PI) diff += 2*PI;
cur += diff * t;                    // smoothTurn 已提供，读懂它
```

---

## 第三人称相机：一根看不见的自拍杆

小宇头顶绑一根长 d 的自拍杆，另一端是镜头
杆的方向 = 两个角 = **球面坐标**：

```cpp
offset.x = d * cos(pitch) * sin(yaw);
offset.y = d * sin(pitch);
offset.z = d * cos(pitch) * cos(yaw);
eye  = target + offset;
view = lookAt(eye, target, {0,1,0});
```

鼠标横滑 → yaw（绕圈）｜ 竖滑 → pitch（俯仰）

---

## pitch 必须限位（图形程序员的成年礼）

pitch 拉到 ±90° 时：**视线与 up 共线**

→ `cross(f, up)` 返回零向量
→ normalize 除以 0 → **NaN 风暴，屏幕抽搐**

本项目限位：pitch ∈ [-15°, 65°]

> 课堂仪式：写完 SD-303 后，注释掉限位，
> 把视角拉到头顶正上方，欣赏 NaN 风暴并**截图存档**
> （实验报告指定素材），然后把限位加回来。

---

## 动手任务（工单发布）

**SD-302**（mgl.h）：先 perspective（马赛克→畸形但有透视）
再 lookAt（**方块镇完整显形**）——自测用 (0,0,5) 那组数

**SD-303**（camera.h）：五步救活钉死的监控探头
鼠标→角度 / pitch 限位 / 注视点抬到胸口 / 球面坐标 / 防穿地

**SD-301**（player.h）：四步让小宇迈腿
基向量 / normalize / 乘 dt / smoothTurn
（腿部摆动动画引擎组已代写——人一动腿自己就摆）

**手感调参**：walkSpeed、灵敏度、distance、fovy——找到你的手感

---

## 验收清单

1. ☐ 手写 lookAt/perspective 正确，窗口拉伸不变形
2. ☐ WASD 相机相对移动，斜向不加速
3. ☐ 平滑转身，无抽搐、无反向长绕
4. ☐ 鼠标环绕流畅，pitch 有限位不翻车
5. ☐ C 键双相机切换正常
6. ☐ 白板推导 lookAt：为什么是转置+负平移
7. ☐ 一句话说清 perspective 里 -1 的作用

**加班挑战**：滚轮变焦 ☆ ｜ 相机弹簧 ☆☆ ｜ 镜头避障 ☆☆☆ ｜ 正交投影对比 ☆☆☆☆

---

<!-- _class: lead -->

## 下周预告

> 苏苏（试玩后）："能走了！！但他走路像**幽灵滑行**……
> 而且他能**穿过房子**？？我设计的箱堆成了摆设！"

Game Jam 交付倒计时一周：

冲刺、跳跃、动画状态机、碰撞检测

> 老周："别慌，你已经握着全部的数学了。
> 剩下的是把它们**编排成生命**。"

**第 4 周：注入灵魂——跑、跳与最终交付**
