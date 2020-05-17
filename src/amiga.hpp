#include <ctime>
#include <csignal>
#include <cstdlib>
#include <new>
#include <malloc.h>

#ifndef SRC_AMIGA_HPP_
#define SRC_AMIGA_HPP_

extern "C" int _gettimeofday( struct timeval *tv, void *tz )
{
	return 0;
}

extern "C" _sig_func_ptr signal (int __sig, _sig_func_ptr __handler) {
	return 0;
}

void* operator new(std::size_t size) {
    return malloc(size);
}

void* operator new[](std::size_t size) {
    return malloc(size);
}

void operator delete(void* ptr) {
    free(ptr);
}

void operator delete[](void* ptr) {
    free(ptr);
}

void* operator new(std::size_t size, const std::nothrow_t&) {
    return malloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) {
    return malloc(size);
}

void operator delete(void* ptr, const std::nothrow_t&) {
    free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) {
    free(ptr);
}




#endif /* SRC_AMIGA_HPP_ */
