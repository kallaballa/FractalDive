#ifndef SRC_TIMETRACKER_HPP_
#define SRC_TIMETRACKER_HPP_

#include <SDL/SDL.h>
#include <map>
#include <string>
#include <sstream>
#include <limits>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cassert>

#ifndef _NO_THREADS
#include <mutex>
#endif

#include "util.hpp"

namespace fractaldive {

using std::ostream;
using std::stringstream;
using std::string;
using std::map;

struct TimeInfo {
	fd_highres_tick_t totalCnt_ = 0;
	fd_highres_tick_t totalTime_ = 0;
	double mean_ = 0;
	double variance_ = 0;

	void updateVariance(size_t t)
	{
		double oldMean = mean_;
		mean_ = mean_ + (t - mean_) / (totalCnt_);
		variance_ = (variance_ + (t - mean_) + (t -oldMean)) / (totalCnt_);
	}

	void add(fd_highres_tick_t t) {
		totalTime_ += t;
		++totalCnt_;
    updateVariance(t);

		if(totalCnt_ > (std::numeric_limits<fd_highres_tick_t>::max() - 1) || totalTime_ > (std::numeric_limits<fd_highres_tick_t>::max() - 1)) {
			totalCnt_ = 0;
			totalTime_ = 0;
		}
	}

	string str() const {
		stringstream ss;
		ss << std::fixed << std::setprecision(4) << std::setfill(' ') << std::setw(9) << totalTime_ << " /" << std::setfill(' ') << std::setw(9) << totalCnt_ << " = " << std::setfill(' ') << std::setw(9) << double(totalTime_)/totalCnt_ << " ~ " << std::setfill(' ') << std::setw(9) << variance_;
		return ss.str();
	}
};

class TimeTracker {
private:
	static TimeTracker* instance_;

	 map<string, TimeInfo> tiMap_;
	 bool enabled_;
#ifndef _NO_THREADS
	 std::mutex mapMtx;
#endif
	TimeTracker();
public:
	virtual ~TimeTracker();

	template<typename F> void execute(const string& name, F const &func)
	{
#ifndef _NO_TIMETRACK
		auto start = get_highres_tick();
		func();

#ifndef _NO_THREADS
		std::unique_lock<std::mutex> lock(mapMtx);
#endif
		TimeInfo& ti = tiMap_[name];
		auto duration = (get_highres_tick() - start);
		ti.add(duration);
#else
		func();
#endif
	}

	template<typename F> size_t measure(F const &func)
	{
#ifndef _NO_TIMETRACK
		auto start = get_highres_tick();
		func();
		auto duration = get_highres_tick() - start;
		return duration;
#else
		assert(false);
		return 0;
#endif
	}

	bool isEnabled() {
		return enabled_;
	}

	void setEnabled(bool e) {
		enabled_ = e;
	}

	void print(ostream& os) {
		os << str();
	}

	std::string str() {
#ifndef _NO_TIMETRACK
#ifndef _NO_THREADS
		std::unique_lock<std::mutex> lock(mapMtx);
#endif
		stringstream ss;
		size_t lenLongestName = 0;
		size_t accumTime = 0;

		for(const auto& it : tiMap_) {
			lenLongestName = std::max(it.first.size(), lenLongestName);
			accumTime += it.second.totalTime_;
		}

		size_t depth = 0;
		map<string, size_t> keyDepthMap_;
		map<size_t, fd_highres_tick_t> depthTotalMap_;

		for(const auto& it : tiMap_) {
			depth = std::count(it.first.begin(), it.first.end(), '.');
			keyDepthMap_[it.first] = depth;
			depthTotalMap_[depth] += it.second.totalTime_;
		}

		for(const auto& it : tiMap_) {
			depth = keyDepthMap_[it.first];
			fd_highres_tick_t depthTotal = depthTotalMap_[depth];
//			fd_highres_tick_t parentDepthTotal;
//			if(depth > 0)
//				parentDepthTotal = depthTotalMap_[depth - 1];
//			else
//				parentDepthTotal = depthTotal;

			double spent =  double(it.second.totalTime_) / double(depthTotal);
//			double loss = ((double(parentDepthTotal) / depthTotal) - 1.0) * 100.0;
			double loss = 0;
			ss << pad_string("    " + it.first + ":", lenLongestName + 6) << spent << " (" << loss << "%)" << std::endl;
		}

		ss << std::endl;

		return ss.str();
#endif
		return "";
	}

	static TimeTracker& getInstance() {
		assert(false);
		if(instance_ == nullptr) {
			instance_ = new TimeTracker();
		}
		return *instance_;
	}

  static void destroy() {
    if(instance_)
      delete instance_;

    instance_ = NULL;
  }

  static void epoch() {
#ifndef _NO_THREADS
		std::unique_lock<std::mutex> lock(instance_->mapMtx);
#endif
  	instance_->tiMap_.clear();
  }
};

} /* namespace fractaldive */

#endif /* SRC_TIMETRACKER_HPP_ */
