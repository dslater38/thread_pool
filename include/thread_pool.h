#ifndef THREAD_POOL_H__
#define THREAD_POOL_H__
#pragma once

#include <vector>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <functional>
#include <iostream>
#include <future>

class thread_pool
{
public:
	struct IWorkSet
	{
		virtual void finishSubTask() = 0;
	};
public:
    ~thread_pool();
    explicit thread_pool(size_t poolsize=0);
	thread_pool(std::function<void(thread_pool &pool)> fn, size_t poolsize=0);
	thread_pool(const thread_pool &) = delete;
	thread_pool(thread_pool &&) = delete;
	thread_pool &operator=(const thread_pool &) = delete;
	thread_pool &operator=(thread_pool &&) = delete;
    void enqueueWork(std::function< void(thread_pool &pool) > fn );
	void enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set)>> tasks, std::function< void(thread_pool &pool) > fn);
    void waitForAllWorkers();
	void initiateShutdown();
private:
    void waitForWork();
    bool waitPred()const {
        return (!workRequests_.empty() || shutdown_);
    }
private:
    bool									shutdown_{false};
    std::condition_variable					trigger_{};
	std::vector<std::future<void>>			pool_{};
    std::mutex								mtx_{};
    std::queue< std::function< void(thread_pool &pool) > >	workRequests_{};
	// std::condition_variable					waitForShutdown_{};
};

#endif //THREAD_POOL_H__