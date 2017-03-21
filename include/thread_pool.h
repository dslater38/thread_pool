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

#define NO_PARAM_PACKS_IN_LAMBDA 1

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
#if NO_PARAM_PACKS_IN_LAMBDA
// If your compiler can't expand parameter packs inside a lambda.
// only support max of 3 args.
	template <class Targ1>
	void enqueueWork(std::function< void(thread_pool &, Targ1) > fn, Targ1 arg1);
	template <class Targ1>
	void enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targ1)>> tasks, std::function< void(thread_pool &pool) > fn, Targ1 arg1);

	template <class Targ1, class Targ2>
	void enqueueWork(std::function< void(thread_pool &, Targ1, Targ2) > fn, Targ1 arg1, Targ2 arg2);
	template <class Targ1, class Targ2>
	void enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targ1, Targ2)>> tasks, std::function< void(thread_pool &pool) > fn, Targ1 arg1, Targ2 arg2);

	template <class Targ1, class Targ2, class Targ3>
	void enqueueWork(std::function< void(thread_pool &, Targ1, Targ2, Targ3) > fn, Targ1 arg1, Targ2 arg2, Targ3 arg3);
	template <class Targ1, class Targ2, class Targ3>
	void enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targ1, Targ2, Targ3)>> tasks, std::function< void(thread_pool &pool) > fn, Targ1 arg1, Targ2 arg2, Targ3 arg3);

#else
	template <class... Targs>
	void enqueueWork(std::function< void(thread_pool &, Targs&&...) > fn, Targs&&...args);
	template <class... Targs>
	void enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targs&&...args)>> tasks, std::function< void(thread_pool &pool) > fn, Targs&&...args);
#endif
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



#if NO_PARAM_PACKS_IN_LAMBDA
// If your compiler can't expand parameter packs inside a lambda.
// only support max of 3 args.
template <class Targ1>
void thread_pool::enqueueWork(std::function< void(thread_pool &, Targ1) > fn, Targ1 arg1)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p) {
		auto ff = const_cast< details::rref_impl<decltype(fn)> &>(task_rref).move();
		ff(p, std::forward<Targ1>(arg1));
	});
}
template <class Targ1>
void thread_pool::enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targ1)>> tasks, std::function< void(thread_pool &pool) > fn, Targ1 arg1)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p, , std::shared_ptr<IWorkSet> set) {
		auto ff = const_cast< details::rref_impl<tasks::value_type> &>(task_rref).move();
		ff(p, std::move(set), std::forward<Targ1>(arg1));
	});
}

template <class Targ1, class Targ2>
void thread_pool::enqueueWork(std::function< void(thread_pool &, Targ1, Targ2) > fn, Targ1 arg1, Targ2 arg2)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p) {
		auto ff = const_cast< details::rref_impl<decltype(fn)> &>(task_rref).move();
		ff(p, std::forward<Targ1>(arg1), std::forward<Targ1>(arg2));
	});
}
template <class Targ1, class Targ2>
void thread_pool::enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targ1, Targ2)>> tasks, std::function< void(thread_pool &pool) > fn, Targ1 arg1, Targ2 arg2)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p, , std::shared_ptr<IWorkSet> set) {
		auto ff = const_cast< details::rref_impl<tasks::value_type> &>(task_rref).move();
		ff(p, std::move(set), std::forward<Targ1>(arg1), std::forward<Targ1>(arg2));
	});
}

template <class Targ1, class Targ2, class Targ3>
void thread_pool::enqueueWork(std::function< void(thread_pool &, Targ1, Targ2, Targ3) > fn, Targ1 arg1, Targ2 arg2, Targ3 arg3)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p) {
		auto ff = const_cast< details::rref_impl<decltype(fn)> &>(task_rref).move();
		ff(p, std::forward<Targ1>(arg1), std::forward<Targ1>(arg2), std::forward<Targ1>(arg3));
	});
}
template <class Targ1, class Targ2, class Targ3>
void thread_pool::enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targ1, Targ2, Targ3)>> tasks, std::function< void(thread_pool &pool) > fn, Targ1 arg1, Targ2 arg2, Targ3 arg3)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p, , std::shared_ptr<IWorkSet> set) {
		auto ff = const_cast< details::rref_impl<tasks::value_type> &>(task_rref).move();
		ff(p, std::move(set), std::forward<Targ1>(arg1), std::forward<Targ1>(arg2), std::forward<Targ1>(arg3));
	});
}

#else
template <class... Targs>
void thread_pool::enqueueWork(std::function< void(thread_pool &, Targs&&...) > fn, Targs&&... args)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p) {
		auto ff = const_cast< details::rref_impl<decltype(fn)> &>(task_rref).move();
		ff(p, std::forward<Targs&&>(args)...);
	});
}
template <class... Targs>
void thread_pool::enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targs&&...)>> tasks, std::function< void(thread_pool &pool) > fn, Targs&&... args)
{
	auto task_rref = details::make_rref(fn);
	enqueueWork([=](thread_pool &p, , std::shared_ptr<IWorkSet> set) {
		auto ff = const_cast< details::rref_impl<tasks::value_type> &>(task_rref).move();
		ff(p, std::move(set), std::forward<Targs&&>(args)...);
	});
}

#endif

#if 0
template <typename... Targs>
class Fcn
{
    template < ::std::size_t... Indices>
    struct indices {};

    template < ::std::size_t N, ::std::size_t... Is>
    struct build_indices : build_indices<N-1, N-1, Is...>
    {};

    template < ::std::size_t... Is>
    struct build_indices<0, Is...> : indices<Is...>
    {};


    template <typename FuncT, typename ArgTuple, ::std::size_t... Indices>
    static
    auto call(const FuncT &f, ArgTuple &&args, const indices<Indices...> &)
       -> decltype(f(::std::get<Indices>(::std::forward<ArgTuple>(args))...))
    {
       return ::std::move(f(::std::get<Indices>(::std::forward<ArgTuple>(args))...));
    }
    
    template <typename FuncT, typename ArgTuple>
    static
    auto call(const FuncT &f, ArgTuple &&args)
         -> decltype(call(f, args,
                          build_indices< ::std::tuple_size<ArgTuple>::value>{}))
    {
        const build_indices< ::std::tuple_size<ArgTuple>::value> indices;

        return ::std::move(call(f, ::std::move(args), indices));
    }    
    
public:
    using IWorkSet=thread_pool::IWorkSet;

    explicit
    Fcn(std::function<void(thread_pool &, std::shared_ptr<IWorkSet> set, Targs&&...)>  fcn, Targs&&...args)
    : fcn_{std::move(fcn)}
    , args_{std::forward<Targs>(args)...}
    {
    }
    void operator()(thread_pool &p, std::shared_ptr<IWorkSet> set)
    {
        // return fcn_(p, std::move(set), std::forward<Targs>(args_)...);
        call( fcn_, std::move(args_) );
    }
private:
    std::function<void(thread_pool &, std::shared_ptr<IWorkSet> set, Targs&&...args)>   fcn_{};
    std::tuple<Targs...> args_{};
};


template< typename...Targs>
Fcn<Targs...> makeFcn( std::function<void(thread_pool &, std::shared_ptr<thread_pool::IWorkSet>, Targs...)>  fcn, Targs&&... args )
{
    return Fcn<Targs...>{std::move(fcn), std::forward<Targs>(args)...};
}

template <typename... Targs>
void thread_pool::enqueueWork(std::function< void(thread_pool &, Targs...) > fn, Targs&&... args)
{
    auto F = makeFcn(std::move(fn), std::forward<Targs>(args)...);
//    auto task_rref = details::make_rref(F);
    
	enqueueWork([&](thread_pool &pool) {
        
//        auto ff{ const_cast< details::rref_impl<std::function< void(thread_pool &)>> &>(task_rref).move() };
//		ff(pool);
	});
}

template <typename... Targs>
void thread_pool::enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set, Targs&&...args)>> tasks, std::function< void(thread_pool &pool) > fn, Targs&&...args)
{
	std::vector < std::function< void(thread_pool &, std::shared_ptr<IWorkSet> set)>> fcns{};
	fcns.reserve(tasks.size());
	for (auto & task : tasks)
	{        
        auto F = makeFcn(std::move(task), std::forward<Targs>(args)...);
        auto task_rref = details::make_rref(F);
        
		fcns.emplace_back([=](thread_pool &p, std::shared_ptr<IWorkSet> set) {
			auto ff = const_cast< details::rref_impl<std::function< void(thread_pool &, std::shared_ptr<IWorkSet>)>> &>(task_rref).move();
			ff(p, set);
		});
	}
	enqueueWork(fcns, std::move(fn));
}
#endif // 0

#endif //THREAD_POOL_H__