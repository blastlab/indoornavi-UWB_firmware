
#ifndef _IASSERT
#define _IASSERT

#ifdef _CORTEX_M
#define IASSERT (void)(0)
#else
#include "assert.h"
#include "stdio.h"
//#define IASSERT(expr) TEST_ASSERT_MESSAGE(expr, #expr)
#define IASSERT(expr)                             \
    do                                            \
    {                                             \
        if (!(expr))                              \
            printf("=== ASSERT === " #expr "\n"); \
    } while (0)
#endif

#endif