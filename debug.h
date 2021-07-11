#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define PRINT_ERROR(...)                                            \
  fprintf(stderr, "error:%s:%s:%d:", __FILE__, __func__, __LINE__); \
  fprintf(stderr, __VA_ARGS__)
#define PRINT_DEBUG(...)                                            \
  fprintf(stderr, "debug:%s:%s:%d:", __FILE__, __func__, __LINE__); \
  fprintf(stderr, __VA_ARGS__)
#else
#include <assert.h>
#define eprintf(...)     (void)(0)
#define PRINT_ERROR(...) (void)(0)
#define PRINT_ERROR(...) (void)(0)

#endif

#endif
