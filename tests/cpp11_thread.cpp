#include "cpp11_thread.h"

#if (__cplusplus < 201103L)
    #error C++11 is required for the C++ version of this file
#endif

#include <thread>

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
    std::thread* result = new std::thread([func, arg](){ func(arg); });

    *thr = reinterpret_cast<thrd_t>(result);

    return thrd_success;
}

intptr_t thrd_current()
{
    std::size_t hash = 
        std::hash<std::thread::id>{}(std::this_thread::get_id());

    // No way of getting current thread pointer in C++11, so this'll do.
    return static_cast<intptr_t>(hash);
}

int thrd_join(thrd_t thr, int *)
{
    std::thread* thread = reinterpret_cast<std::thread*>(thr);

    thread->join();
    delete thread;

    return thrd_success;
}