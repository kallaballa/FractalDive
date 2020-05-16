#ifndef SRC_THREADPOOL_HPP_
#define SRC_THREADPOOL_HPP_
#include <cassert>
#include <deque>
#include <vector>

#ifndef _NO_THREADS
#include <thread>
#include <condition_variable>
#include <mutex>
#endif

#include <functional>
#ifdef _JAVASCRIPT
#include <emscripten/threading.h>
#endif
namespace fractaldive {

#ifndef _NO_THREADS
class Semaphore {
public:
    Semaphore (int count_ = 0)
    : count_(count_) {}

    inline void notify( int tid ) {
        std::unique_lock<std::mutex> lock(mtx);
        count_++;
//        std::cout << "thread " << tid <<  " notify" << std::endl;
        //notify the waiting thread
        cv.notify_one();
    }

    inline void wait( int tid ) {
        std::unique_lock<std::mutex> lock(mtx);
        while(count_ == 0) {
//        		std::cout << "thread " << tid << " wait" << std::endl;
            //wait on the mutex until notify is called
            cv.wait(lock);
//            std::cout << "thread " << tid << " run" << std::endl;
        }
        count_--;
    }
  	size_t count() {
  		return count_;
 		}
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count_;
};

class ThreadPool {
private:
	static ThreadPool* instance_;
	static std::mutex instanceMtx_;
	std::mutex queueMtx_;
	std::mutex runningMtx_;
	std::vector<std::thread*> pool_;
	std::deque<std::function<void()>> work_queue_;
	Semaphore workSema_;
	Semaphore joinSema_;
	std::vector<bool> running_;
public:
	ThreadPool(const size_t size) :
		queueMtx_(),
		pool_(size),
		work_queue_(),
		workSema_(0),
		joinSema_(0),
		running_(size, false) {
		for(size_t i = 0; i < pool_.size(); ++i) {
			pool_[i] = new std::thread([&,i](){
				while(true) {
					workSema_.wait(i+1);
					runningMtx_.lock();
					running_[i] = true;
					runningMtx_.unlock();

					queueMtx_.lock();
					std::function<void()> func = work_queue_.front();
					work_queue_.pop_front();
					queueMtx_.unlock();
					func();
					runningMtx_.lock();
					running_[i] = false;
					runningMtx_.unlock();

					joinSema_.notify(i+1);
				}
			});
		}
	}

	~ThreadPool() {
		for(size_t i = 0; i < pool_.size(); ++i) {
			delete pool_[i];
		}
	}

	size_t size() {
		return pool_.size();
	}
	static size_t extra_cores() {
		size_t numThreads = std::thread::hardware_concurrency();
#ifdef _JAVASCRIPT
		numThreads = 1;
	#ifdef _JAVASCRIPT_MT
		numThreads = emscripten_num_logical_cores();
	#endif
#endif
		assert(numThreads > 0);

		return numThreads - 1; //reserve a core for main thread
	}

	static ThreadPool& getInstance() {
		std::unique_lock<std::mutex> lock(instanceMtx_);
		if(instance_ == nullptr) {
			assert(ThreadPool::extra_cores() > 0);
			instance_ = new ThreadPool(ThreadPool::extra_cores());
		}

		return *instance_;
	}

	void work(const std::function<void()>& func) {
		queueMtx_.lock();
		work_queue_.push_back(func);
		queueMtx_.unlock();
		workSema_.notify(0);
	}

	void join() {
		bool running = true;
		while(true) {
			running = false;
			runningMtx_.lock();
			for(size_t i = 0; i < running_.size(); ++i) {
				if(running_[i]) {
					running = true;
					break;
				}
			}
			runningMtx_.unlock();
			if(running == false)
				return;

			joinSema_.wait(0);
		}
	}
};
#else
class Semaphore {
public:
    Semaphore (int count_ = 0) {}

    inline void notify( int tid ) {
    }

    inline void wait( int tid ) {
    }
  	size_t count() {
  		return 0;
 		}
private:
};

class ThreadPool {
private:
	static ThreadPool* instance_;
public:
	ThreadPool(const size_t size) {
	}

	~ThreadPool() {
	}

	size_t size() {
		return 0;
	}

	static size_t extra_cores() {
		return 0;
	}

	static ThreadPool& getInstance() {
		if(instance_ == nullptr)
			instance_ = new ThreadPool(0);
		return *instance_;
	}

	void work(const std::function<void()>& func) {

	}

	void join() {

	}
};

#endif

} /* namespace fractaldive */

#endif /* SRC_THREADPOOL_HPP_ */
