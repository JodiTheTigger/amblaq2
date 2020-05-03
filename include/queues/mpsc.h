#ifndef MPSC_H
#define MPSC_H

#include <queues/queues_common.h>

#include <string.h> // memset

// -----------------------------------------------------------------------------
// To use this file:
// 1) copy this file and rename it to your liking
// 2) update in MPSC_ITEM_COUNT if you like
// 3) Replace Mpsc_Data with whatever data type you want to queue
// 4) Rename MPSC_, Mpsc_ and mpsc_ to a tag of your choosing. For eg, your
//    new file name. Make sure to match the original case
// -----------------------------------------------------------------------------

#define MPSC_ITEM_COUNT 1024

// -----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mpsc_Type
{
    uint64_t payload;
}
Mpsc_Type;

typedef struct Mpsc_Cell
{
    atomic_size_t sequence;
    Mpsc_Type     data;
}
Mpsc_Cell;

typedef struct Mpsc_State
{
    uint8_t       pad0[QUEUE_CACHELINE_BYTES];

    atomic_size_t index_enqueue;
    uint8_t       pad2[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    size_t        index_dequeue;
    uint8_t       pad3[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    Mpsc_Cell     cells[MPSC_ITEM_COUNT];
}
Mpsc_State;

// -----------------------------------------------------------------------------

Queue_Result mpsc_init       (Mpsc_State* queue);
Queue_Result mpsc_try_enqueue(Mpsc_State* queue, Mpsc_Type const* data);
Queue_Result mpsc_try_dequeue(Mpsc_State* queue, Mpsc_Type*       data);

#ifdef __cplusplus
}
#endif

#endif //MPSC_H

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
#ifdef MPSC_IMPLEMENTATION

#define MPSC_TOO_BIG (1024ULL * 256ULL)
#define MPSC_MASK    (MPSC_ITEM_COUNT - 1)

// -----------------------------------------------------------------------------

_Static_assert
(
      !(MPSC_ITEM_COUNT < 2)
    , "MPSC_ITEM_COUNT too small"
);

_Static_assert
(
      !(MPSC_ITEM_COUNT >= MPSC_TOO_BIG)
    , "MPSC_ITEM_COUNT too big"
);

_Static_assert
(
      !(MPSC_ITEM_COUNT & (MPSC_ITEM_COUNT - 1))
    , "MPSC_ITEM_COUNT not a power of 2"
);

// -----------------------------------------------------------------------------

Queue_Result mpsc_init(Mpsc_State* queue)
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

    memset(queue, 0, sizeof(Mpsc_State));

    for (size_t i = 0; i < MPSC_ITEM_COUNT; i++)
    {
        atomic_store_explicit
        (
              &queue->cells[i].sequence
            , i
            , memory_order_relaxed
        );
    }

    atomic_store_explicit(&queue->index_enqueue, 0, memory_order_relaxed);

    return Queue_Ok;
}

// -----------------------------------------------------------------------------

Queue_Result mpsc_try_enqueue(Mpsc_State* queue, Mpsc_Type const* data)
{
    size_t position =    
        atomic_load_explicit(&queue->index_enqueue, memory_order_relaxed);

    Mpsc_Cell* cell = &queue->cells[position & MPSC_MASK];

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

Queue_Result mpsc_try_dequeue(Mpsc_State* queue, Mpsc_Type* data)
{
    size_t position = queue->index_dequeue;

    Mpsc_Cell* cell = &queue->cells[position & MPSC_MASK];

    size_t sequence =
        atomic_load_explicit(&cell->sequence, memory_order_acquire);

    intptr_t difference = (intptr_t) sequence - (intptr_t)(position + 1);

    if (!difference)
    {
        queue->index_dequeue = position + 1;
        
        {
            *data = cell->data;

            atomic_store_explicit
            (
                  &cell->sequence
                , position + MPSC_MASK + 1
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

#endif // MPSC_IMPLEMENTATION