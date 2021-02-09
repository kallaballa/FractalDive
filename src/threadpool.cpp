#include "threadpool.hpp"

namespace fractaldive {
#ifndef _NO_THREADS
ThreadPool* ThreadPool::instance_ = nullptr;
std::mutex ThreadPool::instanceMtx_;
#endif
} /* namespace fractaldive */
