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
	template <class... Targs>
	void enqueueWork(std::function< void(thread_pool &pool, Targs&&...args) > fn, Targs&&...args);
	void enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set)>> tasks, std::function< void(thread_pool &pool) > fn);
	template <class... Targs>
	void enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targs&&...args)>> tasks, std::function< void(thread_pool &pool) > fn, Targs&&...args);
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

namespace details {

	template <typename T>
	struct rref_impl
	{
		rref_impl() = delete;
		rref_impl(T && x) : x{ std::move(x) } {}
		rref_impl(const rref_impl & other)
			: x{ std::move(const_cast<rref_impl &>(other).x) }, isCopied{ true }
		{
			//assert(other.isCopied == false);
		}
		rref_impl(rref_impl && other)
			: x{ std::move(other.x) }, isCopied{ std::move(other.isCopied) }
		{
		}
		rref_impl & operator=(rref_impl other) = delete;
		T && move()
		{
			return std::move(x);
		}
	private:
		T x;
		bool isCopied = false;
	};

	template<typename T> rref_impl<T> make_rref(T && x)
	{
		return rref_impl<T>{ std::move(x) };
	}
}

template <class... Targs>
void thread_pool::enqueueWork(std::function< void(thread_pool &pool, Targs&&...args) > fn, Targs&&...args)
{
	enqueueWork([=](thread_pool &pool) {
		fn(std::forward<Targs>(args)...);
	});
}

template <class... Targs>
void thread_pool::enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targs&&...args)>> tasks, std::function< void(thread_pool &pool) > fn, Targs&&...args)
{
	std::vector < std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set)> fcns{};
	fcns.reserve(tasks.size());
	for (auto & task : tasks)
	{
		auto task_rref = details::make_rref(std::move(task));
		fcns.emplace_back([=](thread_pool &p, std::shared_ptr<IWorkSet> set) {
			auto ff{ const_cast< details::rref_impl<std::function< void(thread_pool &, std::shared_ptr<IWorkSet>, Targs&&...args)>> &>(task_rref).move() };
			ff(p, set, std::forward<Targs>(args)...);
		});
	}
	enqueueWork(fcns, std::move(fn));
}

#endif //THREAD_POOL_H__