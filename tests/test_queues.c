#define MPMC_IMPLEMENTATION
#include <queues/mpmc.h>

#define MPSC_IMPLEMENTATION
#include <queues/mpsc.h>

#define SPMC_IMPLEMENTATION
#include <queues/spmc.h>

#define SPSC_IMPLEMENTATION
#include <queues/spsc.h>

#if defined(__STDC_NO_THREADS__)
    #error Cannot build tests, your C compiler does not support C11 threads :-(
#endif

#include <stdio.h>
#include <threads.h>

// -----------------------------------------------------------------------------

#define MAX_CONCURRENT      4
#define MAX_TEST_LOOP_COUNT 10000

// -----------------------------------------------------------------------------

#define LOG(x) printf(x "\n"); fflush(stdout);
#define TEST_SIMPLE(x)    \
    if (!(x))             \
    {                     \
        LOG("FAIL: " #x); \
        return 1;         \
    }

int test_null()
{
    TEST_SIMPLE(mpmc_init(NULL) == Queue_Error_Null);
    TEST_SIMPLE(spmc_init(NULL) == Queue_Error_Null);
    TEST_SIMPLE(mpsc_init(NULL) == Queue_Error_Null);
    TEST_SIMPLE(spsc_init(NULL) == Queue_Error_Null);

    return 0;
}

int test_not_aligned()
{
    struct 
    {
        Mpmc_State mpmc[2];
        Mpsc_State mpsc[2];
        Spmc_State spmc[2];
        Spsc_State spsc[2];
    }
    state;

    intptr_t pmpmc = 1 | (intptr_t) state.mpmc;
    intptr_t pmpsc = 1 | (intptr_t) state.mpsc;
    intptr_t pspmc = 1 | (intptr_t) state.spmc;
    intptr_t pspsc = 1 | (intptr_t) state.spsc;

    Mpmc_State* bad_mpmc = (Mpmc_State*) pmpmc;
    Mpsc_State* bad_mpsc = (Mpsc_State*) pmpsc;
    Spmc_State* bad_spmc = (Spmc_State*) pspmc;
    Spsc_State* bad_spsc = (Spsc_State*) pspsc;

    TEST_SIMPLE(mpmc_init(bad_mpmc) == Queue_Error_Not_Aligned_16_Bytes);
    TEST_SIMPLE(mpsc_init(bad_mpsc) == Queue_Error_Not_Aligned_16_Bytes);
    TEST_SIMPLE(spmc_init(bad_spmc) == Queue_Error_Not_Aligned_16_Bytes);
    TEST_SIMPLE(spsc_init(bad_spsc) == Queue_Error_Not_Aligned_16_Bytes);

    return 0;
}

int test_init_ok()
{
    _Alignas(16) struct
    {
        Mpmc_State mpmc;
        Mpsc_State mpsc;
        Spmc_State spmc;
        Spsc_State spsc;
    }
    state;

    TEST_SIMPLE(mpmc_init(&state.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_init(&state.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_init(&state.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_init(&state.spsc) == Queue_Ok);

    return 0;
}

int test_enqueue()
{
    _Alignas(16) struct
    {
        Mpmc_State mpmc;
        Mpsc_State mpsc;
        Spmc_State spmc;
        Spsc_State spsc;
    }
    state;

    struct
    {
        Mpmc_Type mpmc;
        Mpsc_Type mpsc;
        Spmc_Type spmc;
        Spsc_Type spsc;
    }
    data = 
    {
        {0}, {0}, {0}, {0}
    };

    TEST_SIMPLE(mpmc_init(&state.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_init(&state.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_init(&state.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_init(&state.spsc) == Queue_Ok);

    TEST_SIMPLE(mpmc_try_enqueue(&state.mpmc, &data.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_try_enqueue(&state.mpsc, &data.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_try_enqueue(&state.spmc, &data.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_try_enqueue(&state.spsc, &data.spsc) == Queue_Ok);

    return 0;
}

int test_dequeue()
{
    _Alignas(16) struct
    {
        Mpmc_State mpmc;
        Mpsc_State mpsc;
        Spmc_State spmc;
        Spsc_State spsc;
    }
    state;

    struct
    {
        Mpmc_Type mpmc;
        Mpsc_Type mpsc;
        Spmc_Type spmc;
        Spsc_Type spsc;
    }
    data = 
    {
        {1}, {2}, {3}, {4}
    };

    struct
    {
        Mpmc_Type mpmc;
        Mpsc_Type mpsc;
        Spmc_Type spmc;
        Spsc_Type spsc;
    }
    result = 
    {
        {0}, {0}, {0}, {0}
    };

    TEST_SIMPLE(mpmc_init(&state.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_init(&state.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_init(&state.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_init(&state.spsc) == Queue_Ok);

    TEST_SIMPLE(mpmc_try_enqueue(&state.mpmc, &data.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_try_enqueue(&state.mpsc, &data.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_try_enqueue(&state.spmc, &data.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_try_enqueue(&state.spsc, &data.spsc) == Queue_Ok);

    TEST_SIMPLE(mpmc_try_dequeue(&state.mpmc, &result.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_try_dequeue(&state.mpsc, &result.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_try_dequeue(&state.spmc, &result.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_try_dequeue(&state.spsc, &result.spsc) == Queue_Ok);
    
    TEST_SIMPLE(data.mpmc.payload == result.mpmc.payload);
    TEST_SIMPLE(data.mpsc.payload == result.mpsc.payload);
    TEST_SIMPLE(data.spmc.payload == result.spmc.payload);
    TEST_SIMPLE(data.spsc.payload == result.spsc.payload);
  
    return 0;
}

int test_empty()
{    
    _Alignas(16) struct
    {
        Mpmc_State mpmc;
        Mpsc_State mpsc;
        Spmc_State spmc;
        Spsc_State spsc;
    }
    state;

    struct
    {
        Mpmc_Type mpmc;
        Mpsc_Type mpsc;
        Spmc_Type spmc;
        Spsc_Type spsc;
    }
    result = 
    {
        {0}, {0}, {0}, {0}
    };

    TEST_SIMPLE(mpmc_init(&state.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_init(&state.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_init(&state.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_init(&state.spsc) == Queue_Ok);

    TEST_SIMPLE(mpmc_try_dequeue(&state.mpmc, &result.mpmc) == Queue_Empty);
    TEST_SIMPLE(mpsc_try_dequeue(&state.mpsc, &result.mpsc) == Queue_Empty);
    TEST_SIMPLE(spmc_try_dequeue(&state.spmc, &result.spmc) == Queue_Empty);
    TEST_SIMPLE(spsc_try_dequeue(&state.spsc, &result.spsc) == Queue_Empty);
 
    return 0;
}

int test_full()
{
    _Alignas(16) struct
    {
        Mpmc_State mpmc;
        Mpsc_State mpsc;
        Spmc_State spmc;
        Spsc_State spsc;
    }
    state;

    TEST_SIMPLE(mpmc_init(&state.mpmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_init(&state.mpsc) == Queue_Ok);
    TEST_SIMPLE(spmc_init(&state.spmc) == Queue_Ok);
    TEST_SIMPLE(spsc_init(&state.spsc) == Queue_Ok);

    for (uint64_t i = 0; i < MPMC_ITEM_COUNT; i++)
    {
        Mpmc_Type data = {i};
        TEST_SIMPLE(mpmc_try_enqueue(&state.mpmc, &data) == Queue_Ok);
    }
    for (uint64_t i = 0; i < MPSC_ITEM_COUNT; i++)
    {
        Mpsc_Type data = {i};
        TEST_SIMPLE(mpsc_try_enqueue(&state.mpsc, &data) == Queue_Ok);
    }
    for (uint64_t i = 0; i < SPMC_ITEM_COUNT; i++)
    {
        Spmc_Type data = {i};
        TEST_SIMPLE(spmc_try_enqueue(&state.spmc, &data) == Queue_Ok);
    }
    for (uint64_t i = 0; i < SPSC_ITEM_COUNT; i++)
    {
        Spsc_Type data = {i};
        TEST_SIMPLE(spsc_try_enqueue(&state.spsc, &data) == Queue_Ok);
    }
    
    struct
    {
        Mpmc_Type mpmc;
        Mpsc_Type mpsc;
        Spmc_Type spmc;
        Spsc_Type spsc;
    }
    data = 
    {
          {MPMC_ITEM_COUNT}
        , {MPSC_ITEM_COUNT}
        , {SPMC_ITEM_COUNT}
        , {SPSC_ITEM_COUNT}
    };

    TEST_SIMPLE(mpmc_try_enqueue(&state.mpmc, &data.mpmc) == Queue_Full);
    TEST_SIMPLE(mpsc_try_enqueue(&state.mpsc, &data.mpsc) == Queue_Full);
    TEST_SIMPLE(spmc_try_enqueue(&state.spmc, &data.spmc) == Queue_Full);
    TEST_SIMPLE(spsc_try_enqueue(&state.spsc, &data.spsc) == Queue_Full);

    return 0;
}

typedef enum Queue_Type
{
      Queue_Type_Mpmc
    , Queue_Type_Spmc
    , Queue_Type_Mpsc
    , Queue_Type_Spsc
}
Queue_Type;

typedef struct Queue_State
{
    Queue_Type type;
    
    union
    {
        Mpmc_State* mpmc;
        Spmc_State* spmc;
        Mpsc_State* mpsc;
        Spsc_State* spsc;
    };
}
Queue_State;

typedef struct Queue_Data
{
    atomic_size_t* start;
    Queue_State    state;
    size_t         thread_i_count;
    size_t         thread_o_count;
    size_t         count_in;
    size_t         count_out;
}
Queue_Data;

int enqueue(void* raw)
{
    Queue_Data* qd = (Queue_Data*) raw;

    while(!atomic_load_explicit(qd->start, memory_order_relaxed));

    size_t      thread_id = (size_t) thrd_current();
    size_t      count     = qd->count_in;
    Queue_State state     = qd->state;

    switch (state.type)
    {
        case Queue_Type_Mpmc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Mpmc_Type data = {thread_id + i};
                while (mpmc_try_enqueue(state.mpmc, &data) != Queue_Ok);
            }
            break;
        }

        case Queue_Type_Spmc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Spmc_Type data = {thread_id + i};
                while (spmc_try_enqueue(state.spmc, &data) != Queue_Ok);
            }
            break;
        }

        case Queue_Type_Mpsc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Mpsc_Type data = {thread_id + i};
                while (mpsc_try_enqueue(state.mpsc, &data) != Queue_Ok);
            }
            break;
        }

        case Queue_Type_Spsc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Spsc_Type data = {thread_id + i};
                while (spsc_try_enqueue(state.spsc, &data) != Queue_Ok);
            }
            break;
        }
    }

    return 0;
}

int dequeue(void* raw)
{
    Queue_Data* qd = (Queue_Data*) raw;

    while(!atomic_load_explicit(qd->start, memory_order_relaxed));

    size_t      count     = qd->count_out;
    Queue_State state     = qd->state;

    switch (state.type)
    {
        case Queue_Type_Mpmc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Mpmc_Type data = {0};
                while (mpmc_try_dequeue(state.mpmc, &data) != Queue_Ok);
            }
            break;
        }

        case Queue_Type_Spmc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Spmc_Type data = {0};
                while (spmc_try_dequeue(state.spmc, &data) != Queue_Ok);
            }
            break;
        }

        case Queue_Type_Mpsc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Mpsc_Type data = {0};
                while (mpsc_try_dequeue(state.mpsc, &data) != Queue_Ok);
            }
            break;
        }

        case Queue_Type_Spsc:
        {
            for (size_t i = 0; i < count; i++)
            {
                Spsc_Type data = {0};
                while (spsc_try_dequeue(state.spsc, &data) != Queue_Ok);
            }
            break;
        }
    }

    return 0;
}

int test_queue
(
      Queue_State state
    , size_t      thread_i_count
    , size_t      thread_o_count
)
{
    thrd_t threads_i[MAX_CONCURRENT] = {0};
    thrd_t threads_o[MAX_CONCURRENT] = {0};

    atomic_size_t start;
    atomic_store(&start, 0);

    Queue_Data queue_data = 
    {
          &start
        , state
        , thread_i_count 
        , thread_o_count
        , MAX_TEST_LOOP_COUNT * thread_o_count
        , MAX_TEST_LOOP_COUNT * thread_i_count
    };

    for (size_t i = 0; i < queue_data.thread_i_count; i++)
    {
        TEST_SIMPLE
        (
            thrd_create(&threads_i[i], enqueue, &queue_data) == thrd_success
        );
    }

    for (size_t i = 0; i < queue_data.thread_o_count; i++)
    {
        TEST_SIMPLE
        (
            thrd_create(&threads_o[i], dequeue, &queue_data) == thrd_success
        );
    }

    // go
    atomic_store_explicit
    (
          &start
        , 1
        , memory_order_relaxed
    );

    // finish
    for (size_t i = 0; i < queue_data.thread_i_count; i++)
    {
        int result_in = 0;

        TEST_SIMPLE
        (
            thrd_join(threads_i[i], &result_in) == thrd_success
        );

        TEST_SIMPLE(result_in == 0);
    }

    for (size_t i = 0; i < queue_data.thread_o_count; i++)
    {
        int result_out = 0;

        TEST_SIMPLE
        (
            thrd_join(threads_o[i], &result_out) == thrd_success
        );

        TEST_SIMPLE(result_out == 0);
    }

    return 0;
}

int test_queue_x_in_x_out(size_t thread_i_count, size_t thread_o_count)
{
    _Alignas(16) Mpmc_State state_mpmc;
    _Alignas(16) Spmc_State state_spmc;
    _Alignas(16) Mpsc_State state_mpsc;
    _Alignas(16) Spsc_State state_spsc;

    TEST_SIMPLE(mpmc_init(&state_mpmc) == Queue_Ok);
    TEST_SIMPLE(spmc_init(&state_spmc) == Queue_Ok);
    TEST_SIMPLE(mpsc_init(&state_mpsc) == Queue_Ok);
    TEST_SIMPLE(spsc_init(&state_spsc) == Queue_Ok);

    Queue_State queue_states[4] = 
    {
          { Queue_Type_Mpmc, { .mpmc = &state_mpmc } }
        , { Queue_Type_Spmc, { .spmc = &state_spmc } }
        , { Queue_Type_Mpsc, { .mpsc = &state_mpsc } }
        , { Queue_Type_Spsc, { .spsc = &state_spsc } }
    };

    int result = 0;

    result += test_queue
    (
          queue_states[0]
        , thread_i_count
        , thread_o_count
    );

    if (thread_i_count == 1)
    {
        result += test_queue
        (
              queue_states[1]
            , thread_i_count
            , thread_o_count
        );
    }

    if (thread_o_count == 1)
    {
        result += test_queue
        (
              queue_states[2]
            , thread_i_count
            , thread_o_count
        );
    }

    if ((thread_i_count == 1) && (thread_o_count == 1))
    {
        result += test_queue
        (
              queue_states[3]
            , thread_i_count
            , thread_o_count
        );
    }

    return result;
}

int main(int argument_count, char** arguments)
{    
    (void) argument_count;
    (void) arguments;
    
    #define RUN_SUITE(x)     \
        LOG("TEST: " #x)     \
        if (x) { return 1; } \
    
    RUN_SUITE(test_null());
    RUN_SUITE(test_not_aligned());
    RUN_SUITE(test_init_ok());
    RUN_SUITE(test_enqueue());
    RUN_SUITE(test_dequeue());
    RUN_SUITE(test_empty());
    RUN_SUITE(test_full());

    #define RUN_SUITE_XX(x_in, x_out)                         \
        LOG("TEST: test_queue (in/out): " #x_in "/" #x_out)   \
        if (test_queue_x_in_x_out(x_in, x_out)) { return 1; } \

    RUN_SUITE_XX(             1,              1);
    RUN_SUITE_XX(MAX_CONCURRENT,              1);
    RUN_SUITE_XX(             1, MAX_CONCURRENT);
    RUN_SUITE_XX(MAX_CONCURRENT, MAX_CONCURRENT);

    return 0;
}
