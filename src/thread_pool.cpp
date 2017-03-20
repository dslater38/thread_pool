#include "thread_pool.h"

thread_pool::~thread_pool()
{
	initiateShutdown();
    for( auto &tr : pool_ )
    {
        tr.get();
    }
}

static
size_t
pool_size(size_t p)
{
	auto sz = ((p == 0) ? std::thread::hardware_concurrency() : p);
    return (sz > 1) ? (sz-1) : sz;
}

thread_pool::thread_pool(size_t poolsize)
{
    auto sz = pool_size(poolsize);
    pool_.reserve(sz);
    for( auto i=0u; i<sz; ++i )
    {
		pool_.emplace_back( std::async( std::launch::async, [this] { waitForWork(); } ) );
    }
}

thread_pool::thread_pool(std::function<void(thread_pool &pool)> fn, size_t poolsize)
	: thread_pool{poolsize}
{
	enqueueWork(std::move(fn));
}

void thread_pool::waitForWork()
{
	for(;;)
    {
        std::function< void(thread_pool &pool) > fn{};
        {
            std::unique_lock<std::mutex> lk{mtx_};
            trigger_.wait( lk, [this]{ return waitPred(); } );
            if( !workRequests_.empty() )
            {
                fn = std::move(workRequests_.front());
                workRequests_.pop();
            }
			else if (shutdown_)
			{
				break;
			}
        }
        if( fn )
        {
            fn(*this);
        }
    }
}

void thread_pool::enqueueWork( std::function< void(thread_pool &pool) > fn )
{
    {
        std::unique_lock<std::mutex> lk{mtx_};
        workRequests_.emplace( std::move(fn));
    }
    trigger_.notify_one();
}


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

class WorkSet : public thread_pool::IWorkSet
{
public:
	WorkSet(uint32_t numTasks, std::function< void(thread_pool &pool) > fn, thread_pool &p)
		: counter_{ numTasks }
		, fn_{ std::move(fn) }
		, pool_{ p }
	{

	}
	void finishSubTask() override
	{
		if (--counter_ == 0)
		{
			pool_.enqueueWork(std::move(fn_));
		}
	}
private:
	std::atomic_uint32_t	counter_;
	std::function< void(thread_pool &pool) > fn_;
	thread_pool		&pool_;
};

void 
thread_pool::enqueueWork(std::vector< std::function< void(thread_pool &, std::shared_ptr<IWorkSet> )>> tasks, std::function< void(thread_pool &pool) > fn)
{
	auto numTasks = (uint32_t)(tasks.size());
	auto wSet = std::static_pointer_cast<IWorkSet>(std::make_shared<WorkSet>(numTasks, std::move(fn), *this));
	for (auto &f : tasks)
	{
		auto ff_rref = make_rref(std::move(f));
		enqueueWork([=](thread_pool &p) {
			auto ff{ const_cast< rref_impl<std::function< void(thread_pool &, std::shared_ptr<IWorkSet>)>> &>(ff_rref).move() };
			ff(p,wSet);
		});
	}
}

void thread_pool::waitForAllWorkers()
{
	waitForWork();
}

void thread_pool::initiateShutdown()
{
	auto doNotify = false;
	{
		std::unique_lock<std::mutex> lk{ mtx_ };
		if (!shutdown_)
		{
			shutdown_ = true;
			doNotify = true;
		}
	}
	if (doNotify)
	{
		trigger_.notify_all();
	}
}