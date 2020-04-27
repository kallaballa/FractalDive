#include "threadpool.hpp"

namespace fractaldive {
ThreadPool* ThreadPool::instance_ = nullptr;
std::mutex ThreadPool::instanceMtx_;

} /* namespace fractaldive2 */
