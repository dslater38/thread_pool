#include <iostream>
#include "thread_pool.h"
#include <iostream>
#include <sstream>

int main(int, char **)
{
    thread_pool pool{};
    
    for( auto i=0; i<1000; ++i )
    {
        pool.enqueueWork( [] {
            std::stringstream ss{};
            ss << "0x" << std::ios::hex << std::this_thread::get_id() << std::endl;
            std::cout << ss.str();
            // std::this_thread::sleep_for( std::chrono::milliseconds(500) );
        });
    }
    // std::this_thread::sleep_for( std::chrono::seconds(20) );
    return 0;
}