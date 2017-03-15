#include <iostream>
#include "thread_pool.h"
#include <iostream>
#include <sstream>

int main(int, char **)
{
    thread_pool pool{};
    
    for( auto i=0; i<100; ++i )
    {
        pool.enqueueWork( [] {
            std::stringstream ss{};
            ss << std::this_thread::get_id() << std::endl;
            std::cout << ss;
            std::this_thread::sleep_for( std::chrono::milliseconds(500) );
        });
    }
    
    return 0;
}