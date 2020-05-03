#ifndef SPMC_H
#define SPMC_H

#include <queues/queues_common.h>

#include <string.h> // memset

// -----------------------------------------------------------------------------
// To use this file:
// 1) copy this file and rename it to your liking
// 2) update in SPMC_ITEM_COUNT if you like
// 3) Replace Spmc_Data with whatever data type you want to queue
// 4) Rename SPMC_, Spmc_ and spmc_ to a tag of your choosing. For eg, your
//    new file name. Make sure to match the original case
// -----------------------------------------------------------------------------

#define SPMC_ITEM_COUNT 1024

// -----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Spmc_Type
{
    uint64_t payload;
}
Spmc_Type;

typedef struct Spmc_Cell
{
    atomic_size_t sequence;
    Spmc_Type     data;
}
Spmc_Cell;

typedef struct Spmc_State
{
    uint8_t       pad0[QUEUE_CACHELINE_BYTES];

    size_t        index_enqueue;
    uint8_t       pad2[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    atomic_size_t index_dequeue;
    uint8_t       pad3[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    Spmc_Cell     cells[SPMC_ITEM_COUNT];
}
Spmc_State;

// -----------------------------------------------------------------------------

Queue_Result spmc_init       (Spmc_State* queue);
Queue_Result spmc_try_enqueue(Spmc_State* queue, Spmc_Type const* data);
Queue_Result spmc_try_dequeue(Spmc_State* queue, Spmc_Type*       data);

#ifdef __cplusplus
}
#endif

#endif //SPMC_H

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
#ifdef SPMC_IMPLEMENTATION

#define SPMC_TOO_BIG (1024ULL * 256ULL)
#define SPMC_MASK    (SPMC_ITEM_COUNT - 1)

// -----------------------------------------------------------------------------

_Static_assert
(
      !(SPMC_ITEM_COUNT < 2)
    , "SPMC_ITEM_COUNT too small"
);

_Static_assert
(
      !(SPMC_ITEM_COUNT >= SPMC_TOO_BIG)
    , "SPMC_ITEM_COUNT too big"
);

_Static_assert
(
      !(SPMC_ITEM_COUNT & (SPMC_ITEM_COUNT - 1))
    , "SPMC_ITEM_COUNT not a power of 2"
);

// -----------------------------------------------------------------------------

Queue_Result spmc_init(Spmc_State* queue)
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

    memset(queue, 0, sizeof(Spmc_State));

    for (size_t i = 0; i < SPMC_ITEM_COUNT; i++)
    {
        atomic_store_explicit
        (
              &queue->cells[i].sequence
            , i
            , memory_order_relaxed
        );
    }

    atomic_store_explicit(&queue->index_dequeue, 0, memory_order_relaxed);

    return Queue_Ok;
}

// -----------------------------------------------------------------------------

Queue_Result spmc_try_enqueue(Spmc_State* queue, Spmc_Type const* data)
{
    size_t position = queue->index_enqueue;
    Spmc_Cell* cell = &queue->cells[position & SPMC_MASK];

    size_t sequence =
        atomic_load_explicit(&cell->sequence, memory_order_acquire);

    intptr_t difference = (intptr_t) sequence - (intptr_t) position;

    if (!difference)
    {
        queue->index_enqueue = position + 1;
        
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

Queue_Result spmc_try_dequeue(Spmc_State* queue, Spmc_Type* data)
{
    size_t position =
        atomic_load_explicit(&queue->index_dequeue, memory_order_relaxed);

    Spmc_Cell* cell = &queue->cells[position & SPMC_MASK];

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
                , position + SPMC_MASK + 1
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

#endif // SPMC_IMPLEMENTATION