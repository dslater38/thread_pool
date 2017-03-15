#ifndef THREAD_POOL_H__
#define THREAD_POOL_H__
#pragma once

#include <thread>
#include <vector>
#include <condition_var>
#include <mutex>

class thread_pool
{
public:
    thread_pool(size_t poolsize=0);
private:
    bool                                   triggered{false};
    std::condition_variable         trigger_{};
    std::vector<std::thread>     pool_{};
    std::mutex                          mtx_{};
};

#endif //THREAD_POOL_H__