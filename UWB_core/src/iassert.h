/**
 * @brief Interactive assertion module
 * 
 * @file logs.h
 * @author Karol Trzcinski
 * @date 2018-06-29
 */
#ifndef _IASSERT
#define _IASSERT

#ifdef _CORTEX_M
#define IASSERT (void)(0)
#else

/**
 * @brief described in port.h
 * 
 * @param msg 
 * @param line 
 */
void PORT_iassert_fun(const char *msg, int line);

#define IASSERT(expr)                                                          \
  do {                                                                         \
    if (!(expr)) {                                                              \
      PORT_iassert_fun(__FUNCTION__, __LINE__); \
    } \
  } while (0)
#endif

#endif
