
#ifndef _IASSERT
#define _IASSERT

#ifdef _CORTEX_M
#define IASSERT (void)(0)
#else
void PORT_iassert_fun(const char *msg, int line);

#define IASSERT(expr)                                                          \
  do {                                                                         \
    if (!(expr)) {                                                              \
      PORT_iassert_fun(__FUNCTION__, __LINE__); \
    } \
  } while (0)
#endif

#endif
