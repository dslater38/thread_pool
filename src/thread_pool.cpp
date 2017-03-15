#include "thread_pool.h"

thread_pool::thread_pool(size_t poolsize)
{
    if( poolsize == 0 )
        poolsize = std::thread::hardware_concurrency();
    pool_.reserve(poolsize);
    
}