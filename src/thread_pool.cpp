#include "thread_pool.h"

thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lk{mtx_};
        shutdown_ = true;
    }
    trigger_.notify_all();
    for( auto &tr : pool_ )
    {
        tr.join();
    }
}

static
size_t
pool_size(size_t p)
{
    return ((p==0) ? std::thread::hardware_concurrency() : p );
}

thread_pool::thread_pool(size_t poolsize)
{
    auto sz = pool_size(poolsize);
    pool_.reserve(sz);
    for( auto i=0u; i<sz; ++i )
    {
        pool_.emplace_back( [this]{ waitForWork(); } );
    }
}

void thread_pool::waitForWork()
{
    while(shutdown_==false)
    {
        std::function< void() > fn{};
        {
            std::unique_lock<std::mutex> lk{mtx_};
            trigger_.wait( lk, [this]{ return !waitPred(); } );
            if( !workRequests_.empty() )
            {
                fn = std::move(workRequests_.front());
                workRequests_.pop();
            }
        }
        if( fn )
        {
            fn();
        }
    }
}

void thread_pool::enqueueWork( const std::function< void() > &fn )
{
    {
        std::unique_lock<std::mutex> lk{mtx_};
        workRequests_.emplace( fn );
    }
    trigger_.notify_one();
}