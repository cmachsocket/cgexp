# CUBE RUNNER — Computer Graphics Lab Course

Four lab sessions (16 class hours) that build **one complete OpenGL 3D game demo**,
instead of four isolated experiments.

Students role-play as **Mingyu**, a C++ intern at fictional game studio
*StarDome Games*, completing weekly work orders toward a Game Jam entry:

| Week | Lab | Topic | Student-implemented (hollowed-out) parts |
|------|-----|-------|------------------------------------------|
| 1 | `lab1_character` | Character modeling & geometric transforms | translate/scale/rotate matrices, hierarchical limb assembly |
| 2 | `lab2_world` | Scene modeling & lighting | scene placement matrices, Phong lighting in GLSL |
| 3 | `lab3_walk` | Walking & viewing transforms | lookAt, perspective, camera-relative movement, third-person camera |
| 4 | `lab4_actions` | Actions: run, jump, collision | animation state machine, jump kinematics, AABB intersection |

Tech stack: **C++20 · OpenGL 4.1 Core Profile · GLFW · CMake**.
Each lab ships a runnable `starter/` (with `TODO` holes) and a full `solution/`.
Lecture slides are in [Marp](https://marp.app/) Markdown (`slides.md`), lab manuals
in `guide.md` (Chinese).

```bash
cmake -B build && cmake --build build -j
./build/lab1_starter
```

See [README.md](README.md) (Chinese) for full documentation.
