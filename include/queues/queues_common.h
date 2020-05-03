#pragma once

#if !defined(__STDC__)
    #error Standard C is required for the C version of this file
#endif

#if (__STDC_VERSION__ < 201112L)
    #error C11 is required for the C version of this file
#endif

#if defined(__STDC_NO_ATOMICS__)
    #error Oh no, your C compiler does not support C11 atomics :-(
#endif

#include <stdatomic.h>
#include <stdint.h>
#include <stddef.h>

#define QUEUE_CACHELINE_BYTES 64

typedef enum Queue_Result
{
      Queue_Ok
    , Queue_Full
    , Queue_Empty
    , Queue_Contention

    , Queue_Error = 128
    , Queue_Error_Not_Aligned_16_Bytes
    , Queue_Error_Null
}
Queue_Result;