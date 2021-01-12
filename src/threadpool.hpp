#ifndef SRC_THREADPOOL_HPP_
#define SRC_THREADPOOL_HPP_

#include <cassert>

#ifdef _JAVASCRIPT
#include <emscripten/threading.h>
#endif

#include <vector>
#include <queue>
#include <memory>
#ifndef _NO_THREADS
#include <thread>
#include <mutex>
#include <condition_variable>
#endif
#include <future>
#include <functional>
#include <stdexcept>

namespace fractaldive {

#ifndef _NO_THREADS
class ThreadPool {
public:
	static size_t cores() {
		size_t numThreads = 0;
#ifdef _JAVASCRIPT
		numThreads = 1;
#ifdef _JAVASCRIPT_MT
		numThreads = emscripten_num_logical_cores();
#endif
#else
		numThreads = std::thread::hardware_concurrency();
#endif
		return numThreads;
	}

	static size_t size() {
		return getInstance().workers_.size();
	}

	static ThreadPool& getInstance() {
		std::unique_lock<std::mutex> lock(instanceMtx_);
		if (instance_ == nullptr) {
			assert(ThreadPool::cores() > 1);
			instance_ = new ThreadPool(ThreadPool::cores());
		}

		return *instance_;
	}

	// the constructor just launches some amount of workers
	inline ThreadPool(size_t threads) :
			stop_(false) {
		for (size_t i = 0; i < threads; ++i)
			workers_.emplace_back([this]
			{
				for(;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex_);
						this->condition_.wait(lock,
								[this] {return this->stop_ || !this->tasks_.empty();});
						if(this->stop_ && this->tasks_.empty())
						return;
						task = std::move(this->tasks_.front());
						this->tasks_.pop();
					}

					task();
				}
			});
	}

	// add new work item to the pool
	template<class F, class ... Args>
	void enqueue(F&& f, Args&&... args) {
		auto task = std::make_shared<std::function<void()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		{
			std::unique_lock<std::mutex> lock(queue_mutex_);

			// don't allow enqueueing after stopping the pool
			assert(!stop_);

			tasks_.emplace([task]() {(*task)();});
		}
		condition_.notify_one();
	}

	// the destructor joins all threads
	inline ~ThreadPool() {
		stop();
	}

	size_t taskCount() {
		std::unique_lock<std::mutex> lock(queue_mutex_);
		return tasks_.size();
	}

	void stop() {
		{
			std::unique_lock<std::mutex> lock(queue_mutex_);
			if (stop_)
				return;
			stop_ = true;
		}
		condition_.notify_all();
		for (std::thread &worker : workers_)
			worker.join();
	}

private:
	// need to keep track of threads so we can join them
	std::vector<std::thread> workers_;
	// the task queue
	std::queue<std::function<void()> > tasks_;

	// synchronization
	std::mutex queue_mutex_;
	std::condition_variable condition_;
	bool stop_;
	static ThreadPool* instance_;
	static std::mutex instanceMtx_;
};
#else
class ThreadPool {
public:
	static size_t cores() {
		return 0;
	}

	static size() {
		return 0;
	}

	static ThreadPool& getInstance() {
		assert(false);
		return *instance_;
	}

	inline ThreadPool(size_t threads) {
	}

	inline ~ThreadPool() {
	}

	// add new work item to the pool
	template<class F, class ... Args>
	void enqueue(F&& f, Args&&... args) {

	}

	void stop() {
	}

private:
	static ThreadPool* instance_;
	static std::mutex instanceMtx_;
};
#endif
}
/* namespace fractaldive */

#endif /* SRC_THREADPOOL_HPP_ */
