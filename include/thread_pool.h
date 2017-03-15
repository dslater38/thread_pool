#ifndef THREAD_POOL_H__
#define THREAD_POOL_H__
#pragma once

#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <functional>

class thread_pool
{
public:
    ~thread_pool();
    thread_pool(size_t poolsize=0);
    void enqueueWork(const  std::function< void() > &fn );
private:
    void waitForWork();
    bool waitPred()const {
        return (!workRequests_.empty() || shutdown_);
    }
private:
    bool                                    shutdown_{false};
    std::condition_variable         trigger_{};
    std::vector<std::thread>     pool_{};
    std::mutex                          mtx_{};
    std::queue< std::function< void() > > workRequests_{};
};

#endif //THREAD_POOL_H__