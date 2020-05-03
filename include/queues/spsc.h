#ifndef SPSC_H
#define SPSC_H

#include <queues/queues_common.h>

#include <string.h> // memset

// -----------------------------------------------------------------------------
// To use this file:
// 1) copy this file and rename it to your liking
// 2) update in SPSC_ITEM_COUNT
// 3) Replace Spsc_Data with whatever data type you want to queue
// 4) Rename SPSC_, Spsc_ and spsc_ to a tag of your choosing. For eg, your
//    new file name. Make sure to match the original case
// -----------------------------------------------------------------------------

#define SPSC_ITEM_COUNT 1024

// -----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Spsc_Type
{
    uint64_t payload;
}
Spsc_Type;

typedef struct Spsc_Cell
{
    atomic_size_t sequence;
    Spsc_Type     data;
}
Spsc_Cell;

typedef struct Spsc_State
{
    uint8_t       pad0[QUEUE_CACHELINE_BYTES];

    size_t        index_enqueue;
    uint8_t       pad2[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    size_t        index_dequeue;
    uint8_t       pad3[QUEUE_CACHELINE_BYTES - sizeof(atomic_size_t)];

    Spsc_Cell     cells[SPSC_ITEM_COUNT];
}
Spsc_State;

// -----------------------------------------------------------------------------

Queue_Result spsc_init       (Spsc_State* queue);
Queue_Result spsc_try_enqueue(Spsc_State* queue, Spsc_Type const* data);
Queue_Result spsc_try_dequeue(Spsc_State* queue, Spsc_Type*       data);

#ifdef __cplusplus
}
#endif

#endif //SPSC_H

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
#ifdef SPSC_IMPLEMENTATION

#define SPSC_TOO_BIG (1024ULL * 256ULL)
#define SPSC_MASK    (SPSC_ITEM_COUNT - 1)

// -----------------------------------------------------------------------------

_Static_assert
(
      !(SPSC_ITEM_COUNT < 2)
    , "SPSC_ITEM_COUNT too small"
);

_Static_assert
(
      !(SPSC_ITEM_COUNT >= SPSC_TOO_BIG)
    , "SPSC_ITEM_COUNT too big"
);

_Static_assert
(
      !(SPSC_ITEM_COUNT & (SPSC_ITEM_COUNT - 1))
    , "SPSC_ITEM_COUNT not a power of 2"
);

// -----------------------------------------------------------------------------

Queue_Result spsc_init(Spsc_State* queue)
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

    memset(queue, 0, sizeof(Spsc_State));

    for (size_t i = 0; i < SPSC_ITEM_COUNT; i++)
    {
        atomic_store_explicit
        (
              &queue->cells[i].sequence
            , i
            , memory_order_relaxed
        );
    }

    return Queue_Ok;
}

// -----------------------------------------------------------------------------

Queue_Result spsc_try_enqueue(Spsc_State* queue, Spsc_Type const* data)
{
    size_t position = queue->index_enqueue;

    Spsc_Cell* cell = &queue->cells[position & SPSC_MASK];

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

Queue_Result spsc_try_dequeue(Spsc_State* queue, Spsc_Type* data)
{
    size_t position = queue->index_dequeue;

    Spsc_Cell* cell = &queue->cells[position & SPSC_MASK];

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
                , position + SPSC_MASK + 1
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

#endif // SPSC_IMPLEMENTATION