VPATH = src:sample:SimpleServer

CXXFLAGS=-std=c++11 -I ./include -pthread -O0 -g 


objects=thread_pool.o


thread_pool: $(objects)
	echo done
	
sample/test: $(objects) main.o
	g++ -lpthread -o $@ $?

server : $(objects) server.o native.o
	g++ -lpthread -o $@ $?