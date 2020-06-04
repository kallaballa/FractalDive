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
#include <future>
#endif
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
			stop(false) {
		for (size_t i = 0; i < threads; ++i)
			workers.emplace_back([this]
			{
				for(;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
								[this] {return this->stop || !this->tasks.empty();});
						if(this->stop && this->tasks.empty())
						return;
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();
				}
			});
	}

	// add new work item to the pool
	template<class F, class ... Args>
	auto enqueue(F&& f, Args&&... args)
	-> std::future<typename std::result_of<F(Args...)>::type> {
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared<std::packaged_task<return_type()> >(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				assert(false);

			tasks.emplace([task]() {(*task)();});
		}
		condition.notify_one();
		return res;
	}

	// the destructor joins all threads
	inline ~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread &worker : workers)
			worker.join();
	}

	size_t size() {
		return workers.size();
	}
private:
	// need to keep track of threads so we can join them
	std::vector<std::thread> workers;
	// the task queue
	std::queue<std::function<void()> > tasks;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
	static ThreadPool* instance_;
	static std::mutex instanceMtx_;
};
#else
class ThreadPool {
public:
	static size_t cores() {
		return 0;
	}

	static ThreadPool& getInstance() {
		std::unique_lock<std::mutex> lock(instanceMtx_);
		if (instance_ == nullptr) {
			instance_ = new ThreadPool(0);
		}
		return *instance_;
	}

	inline ThreadPool(size_t threads) :	{
	}

	template<class F, class ... Args>
	auto enqueue(F&& f, Args&&... args)
	-> std::future<typename std::result_of<F(Args...)>::type> {
		using return_type = typename std::result_of<F(Args...)>::type;
		std::future<return_type> res;
		assert(false);
		return res;
	}

	inline ~ThreadPool() {
	}

	size_t size() {
		return 0;
	}
private:
	static ThreadPool* instance_;
	static std::mutex instanceMtx_;
};
#endif
} /* namespace fractaldive */

#endif /* SRC_THREADPOOL_HPP_ */
