#ifndef MPMC_H
#define MPMC_H

#include <queues/queues_common.h>

#include <string.h> // memset

// -----------------------------------------------------------------------------
// To use this file:
// 1) copy this file and rename it to your liking
// 2) update in MPMC_ITEM_COUNT if you like
// 3) Replace Mpmc_Data with whatever data type you want to queue
// 4) Rename MPMC_, Mpmc_ and mpmc_ to a tag of your choosing. For eg, your
//    new file name. Make sure to match the original case
// -----------------------------------------------------------------------------

#define MPMC_ITEM_COUNT 1024

// -----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mpmc_Type
{
    uint64_t payload;
}
Mpmc_Type;

typedef struct Mpmc_Cell
{
    atomic_size_t sequence;
    Mpmc_Type     data;
}
Mpmc_Cell;

typedef struct Mpmc_State
{
    uint8_t       pad0[QUEUE_CACHELINE_BYTES];

    atomic_size_t index_enqueue;
    uint8_t       pad2[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    atomic_size_t index_dequeue;
    uint8_t       pad3[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    Mpmc_Cell     cells[MPMC_ITEM_COUNT];
}
Mpmc_State;

// -----------------------------------------------------------------------------

Queue_Result mpmc_init       (Mpmc_State* queue);
Queue_Result mpmc_try_enqueue(Mpmc_State* queue, Mpmc_Type const* data);
Queue_Result mpmc_try_dequeue(Mpmc_State* queue, Mpmc_Type*       data);

#ifdef __cplusplus
}
#endif

#endif //MPMC_H

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
#ifdef MPMC_IMPLEMENTATION

#define MPMC_TOO_BIG (1024ULL * 256ULL)
#define MPMC_MASK    (MPMC_ITEM_COUNT - 1)

// -----------------------------------------------------------------------------

_Static_assert
(
      !(MPMC_ITEM_COUNT < 2)
    , "MPMC_ITEM_COUNT too small"
);

_Static_assert
(
      !(MPMC_ITEM_COUNT >= MPMC_TOO_BIG)
    , "MPMC_ITEM_COUNT too big"
);

_Static_assert
(
      !(MPMC_ITEM_COUNT & (MPMC_ITEM_COUNT - 1))
    , "MPMC_ITEM_COUNT not a power of 2"
);

// -----------------------------------------------------------------------------

Queue_Result mpmc_init(Mpmc_State* queue)
{
    if (!queue)
    {
        return Queue_Error_Null;
    }
    {
        intptr_t queue_value = (intptr_t) queue;

        if (queue_value & 0x0F)
        {
            return Queue_Error_Not_Aligned_16_Bytes;
        }
    }

    memset(queue, 0, sizeof(Mpmc_State));

    for (size_t i = 0; i < MPMC_ITEM_COUNT; i++)
    {
        atomic_store_explicit
        (
              &queue->cells[i].sequence
            , i
            , memory_order_relaxed
        );
    }

    atomic_store_explicit(&queue->index_enqueue, 0, memory_order_relaxed);
    atomic_store_explicit(&queue->index_dequeue, 0, memory_order_relaxed);

    return Queue_Ok;
}

// -----------------------------------------------------------------------------

Queue_Result mpmc_try_enqueue(Mpmc_State* queue, Mpmc_Type const* data)
{
    size_t position =    
        atomic_load_explicit(&queue->index_enqueue, memory_order_relaxed);

    Mpmc_Cell* cell = &queue->cells[position & MPMC_MASK];

    size_t sequence =
        atomic_load_explicit(&cell->sequence, memory_order_acquire);

    intptr_t difference = (intptr_t) sequence - (intptr_t) position;

    if (!difference)
    {
        if 
        (
            atomic_compare_exchange_weak_explicit
            (
                  &queue->index_enqueue
                , &position
                , position + 1
                , memory_order_relaxed
                , memory_order_relaxed
            )
        )
        {
            cell->data = *data;

            atomic_store_explicit
            (
                  &cell->sequence
                , position + 1
                , memory_order_release
            );

            return Queue_Ok;
        }
    }

    if (difference < 0)
    {
        return Queue_Full;
    }

    return Queue_Contention;
}

// -----------------------------------------------------------------------------

Queue_Result mpmc_try_dequeue(Mpmc_State* queue, Mpmc_Type* data)
{
    size_t position =
        atomic_load_explicit(&queue->index_dequeue, memory_order_relaxed);

    Mpmc_Cell* cell = &queue->cells[position & MPMC_MASK];

    size_t sequence =
        atomic_load_explicit(&cell->sequence, memory_order_acquire);

    intptr_t difference = (intptr_t) sequence - (intptr_t)(position + 1);

    if (!difference)
    {
        if 
        (
            atomic_compare_exchange_weak_explicit
            (
                  &queue->index_dequeue
                , &position
                , position + 1
                , memory_order_relaxed
                , memory_order_relaxed
            )
        )
        {
            *data = cell->data;

            atomic_store_explicit
            (
                  &cell->sequence
                , position + MPMC_MASK + 1
                , memory_order_release
            );

            return Queue_Ok;
        }
    }

    if (difference < 0)
    {
        return Queue_Empty;
    }

    return Queue_Contention;
}

#endif // MPMC_IMPLEMENTATION