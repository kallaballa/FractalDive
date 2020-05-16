#include "threadpool.hpp"

namespace fractaldive {
ThreadPool* ThreadPool::instance_ = nullptr;
#ifndef _NO_THREADS
std::mutex ThreadPool::instanceMtx_;
#endif
} /* namespace fractaldive2 */
