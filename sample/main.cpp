#include <iostream>
#include "thread_pool.h"
#include <iostream>
#include <sstream>
#include <atomic>
#include <array>
#include "matrix.h"

#include <WinSock2.h>

SOCKET barr()
{
	return INVALID_SOCKET;
}

void foo(int i)
{

}

template<class T>
void output(const T &i)
{
	std::stringstream ss{};
	ss << "0x" << std::ios::hex << std::this_thread::get_id() << ": " << i << std::endl;
	std::cout << ss.str();
}

double det(matrix m)
{
	return 0.0;
}

int runFib(int n);

int main(int, char **)
{
    
	auto m = matrix{ 4,4 };

	for (auto i = 0; i < 4; ++i)
	{
		for (auto j = 0; j < 4; ++j)
		{
			m.at(i, j) = (i == j ? 7.0 : i + j);
		}
	}

	std::cout << "det: " << m.det() << std::endl;

	std::atomic_uint64_t count{1000ull};

	std::function<void(thread_pool &pool)> foo = [&](thread_pool &pool) {
		if (--count > 0)
		{
			pool.enqueueWork(foo);
		}
		else
		{
			output("shutdown");
			pool.initiateShutdown();
		}
		output( count );
	};

	thread_pool pool{foo};

	pool.waitForAllWorkers();

    return 0;
}

int runFib(int n)
{
	int result = 0;

	std::function<void(thread_pool &, std::shared_ptr<thread_pool::IWorkSet> )> fib = [=,&result](thread_pool &pool, std::shared_ptr<thread_pool::IWorkSet> set) {
		if (n == 0 || n == 1)
		{
			result += n;
			set->finishSubTask();
		}
		else
		{
			auto vec = std::vector< std::function< void(thread_pool &, std::shared_ptr<thread_pool::IWorkSet> set)>>{};
			n = n - 1;
			
		}
	};
}