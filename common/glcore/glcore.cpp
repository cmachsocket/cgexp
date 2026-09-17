#include "glcore.h"

#include <cstdio>

#define GLCORE_DEFINE(type, name) type glcore_##name = nullptr;
GLCORE_FUNC_LIST(GLCORE_DEFINE)
#undef GLCORE_DEFINE

bool glcoreInit(GlcoreLoadFn loader) {
    bool ok = true;
#define GLCORE_LOAD(type, name)                                                \
    glcore_##name = reinterpret_cast<type>(loader(#name));                     \
    if (!glcore_##name) {                                                      \
        std::fprintf(stderr, "[glcore] 加载 OpenGL 函数失败: %s\n", #name);   \
        ok = false;                                                            \
    }
    GLCORE_FUNC_LIST(GLCORE_LOAD)
#undef GLCORE_LOAD
    return ok;
}
