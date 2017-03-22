#include <iostream>
#include "thread_pool.h"
#include <iostream>
#include <sstream>
#include <atomic>
#include <array>
#include "matrix.h"

#ifdef WIN32
#include <WinSock2.h>
#endif

//~ SOCKET barr()
//~ {
	//~ return INVALID_SOCKET;
//~ }

//~ void foo(int i)
//~ {

//~ }

template<class T>
void output(const T &i)
{
	std::stringstream ss{};
	ss << "0x" << std::ios::hex << std::this_thread::get_id() << ": " << i << std::endl;
	std::cout << ss.str();
}

//~ double det(matrix m)
//~ {
	//~ return 0.0;
//~ }

// int runFib(int n);

std::uint64_t
fib2(std::uint64_t n)
{
    return (n < 2) ? n : fib2(n-1)+fib2(n-2);
}

void
fib(thread_pool &p,  std::uint64_t &n)
{
    using IWorkSet=thread_pool::IWorkSet;
    if( n > 1 )
    {
        auto sharedState = std::make_shared<std::array<std::uint64_t,2>>( std::array<std::uint64_t,2>{n-1, n-2} );
        
        std::vector<std::function< void(thread_pool &, std::shared_ptr<IWorkSet>)>> tasks {
            [&p,sharedState](thread_pool &, std::shared_ptr<IWorkSet> set) {
                std::uint64_t &a = (*sharedState)[0];
                fib(p,a);
                set->finishSubTask();
            },
            [&p,sharedState](thread_pool &, std::shared_ptr<IWorkSet> set) {
                std::uint64_t &b = (*sharedState)[1];
                fib(p,b);
                set->finishSubTask();
            }
        };
        
        p.enqueueWork( tasks, [&n,sharedState](thread_pool &p) {
            std::uint64_t &a = (*sharedState)[0];
            std::uint64_t &b = (*sharedState)[1];
            n = a+b;
        });
    }
}

int main(int argc, char ** argv)
{
    
	//~ auto m = matrix{ 4,4 };

	//~ for (auto i = 0; i < 4; ++i)
	//~ {
		//~ for (auto j = 0; j < 4; ++j)
		//~ {
			//~ m.at(i, j) = (i == j ? 7.0 : i + j);
		//~ }
	//~ }

	//~ std::cout << "det: " << m.det() << std::endl;
    
    if( argc == 2 )
    {
        char *end = nullptr;
        std::uint64_t n = std::strtoull(argv[1], &end, 0);
        
        auto n1 = fib2(n);
        
        std::cout << "recursive fib2(" << n << ") == " << n1 << std::endl;
        
        auto n2 = n;
        
        
        thread_pool pool{};
    
        pool.enqueueWork( [&](thread_pool &p) {
            fib(p,n2);
        });
        
        pool.waitForAllWorkers();
        
        std::cout << "threaded fib(" << n << ") == " << n2 << std::endl;
        return 0;
    }
    return 1;
    

	//~ std::atomic_uint64_t count{1000ull};

	//~ std::function<void(thread_pool &pool)> foo = [&](thread_pool &pool) {
		//~ if (--count > 0)
		//~ {
			//~ pool.enqueueWork(foo);
		//~ }
		//~ else
		//~ {
			//~ output("shutdown");
			//~ pool.initiateShutdown();
		//~ }
		//~ output( count );
	//~ };

	//~ thread_pool pool{foo};

	//~ pool.waitForAllWorkers();

    return 0;
}

//~ int runFib(int n)
//~ {
	//~ int result = 0;

	//~ std::function<void(thread_pool &, std::shared_ptr<thread_pool::IWorkSet> )> fib = [=,&result](thread_pool &pool, std::shared_ptr<thread_pool::IWorkSet> set) {
		//~ if (n == 0 || n == 1)
		//~ {
			//~ result += n;
			//~ set->finishSubTask();
		//~ }
		//~ else
		//~ {
			//~ auto vec = std::vector< std::function< void(thread_pool &, std::shared_ptr<thread_pool::IWorkSet> set, int n)>>{};
			//~ vec[0] = [=]()
			
		//~ }
	//~ };
//~ }