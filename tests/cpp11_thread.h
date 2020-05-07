#ifndef CPP11_THREAD_H
#define CPP11_THREAD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t thrd_t; 
typedef int(*thrd_start_t)(void*);

#define thrd_success 0

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
intptr_t thrd_current();
int thrd_join(thrd_t thr, int *res);

#ifdef __cplusplus
}
#endif


#endif