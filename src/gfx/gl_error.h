#if !defined(GL_ERROR_H)
#define GL_ERROR_H

#include <stdbool.h>

// x86 only
#ifdef _MSC_VER
#define ASSERT_BREAKPOINT(x) \
    if (!(x))                \
    {                        \
        printf("Unsupported breakpoint asm in msc, exiting GL error");\
        exit(-1);\
    }
#else

#define ASSERT_BREAKPOINT(x) \
    if (!(x))                \
    {                        \
        asm("int $3");       \
        asm("nop");          \
    }
#endif

#define GL_CALL(x)   \
    glClearError(); \
    x;              \
    ASSERT_BREAKPOINT(glCheckError(#x, __FILE__, __LINE__))

void glClearError();
bool glCheckError(const char *function_name, const char *file, int line);

#endif // GL_ERROR_H
