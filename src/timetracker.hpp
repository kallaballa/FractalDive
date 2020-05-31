#ifndef SRC_TIMETRACKER_HPP_
#define SRC_TIMETRACKER_HPP_

#include <SDL/SDL.h>
#include <map>
#include <string>
#include <sstream>
#include <limits>
#include <iomanip>

#ifndef _NO_THREADS
#include <mutex>
#endif

namespace fractaldive {

using std::ostream;
using std::stringstream;
using std::string;
using std::map;

struct TimeInfo {
	long totalCnt_ = 0;
	long totalTime_ = 0;
	long gameCnt_ = 0;
	long gameTime_ = 0;
	long last_ = 0;
	float mean_ = 0;
	float variance_ = 0;
	map<string, TimeInfo> children_;

	void updateVariance(size_t t)
	{
		float oldMean = mean_;
		mean_ = mean_ + (t - mean_) / (totalCnt_);
		variance_ = (variance_ + (t - mean_) + (t -oldMean)) / (totalCnt_);
	}

	void add(size_t t) {
		last_ = t;
		totalTime_ += t;
    gameTime_ += t;
		++totalCnt_;
    ++gameCnt_;
    updateVariance(t);

		if(totalCnt_ == std::numeric_limits<long>::max() || totalTime_ == std::numeric_limits<long>::max()) {
			totalCnt_ = 0;
			totalTime_ = 0;
		}

    if(gameCnt_ == std::numeric_limits<long>::max() || gameTime_ == std::numeric_limits<long>::max()) {
      gameCnt_ = 0;
      gameTime_ = 0;
    }
	}

	void newGame() {
	  gameCnt_ = 0;
	  gameTime_ = 0;
	}

	string str() const {
		stringstream ss;
		ss << std::fixed << std::setprecision(4) << std::setfill(' ') << std::setw(9) << totalTime_ << " /" << std::setfill(' ') << std::setw(9) << totalCnt_ << " = " << std::setfill(' ') << std::setw(9) << float(totalTime_)/totalCnt_ << " ~ " << std::setfill(' ') << std::setw(9) << variance_;
		return ss.str();
	}
};

inline std::ostream& operator<<(ostream& os, TimeInfo& ti) {
	os << ti.str();
	return os;
}

class TimeTracker {
private:
	static TimeTracker* instance_;

	 map<string, TimeInfo> tiMap_;
	 bool enabled_;
#ifndef _NO_THREADS
	 std::mutex mapMtx;
#endif
	TimeTracker();

	std::string pad_string(std::string s, size_t num) {
		s.append(num - s.length(), ' ');
		return s;
	}
public:
	virtual ~TimeTracker();

	template<typename F> void execute(const string& name, F const &func)
	{
#ifndef _NO_TIMETRACK
		auto start = SDL_GetTicks();
		func();
		auto duration = SDL_GetTicks() - start;

#ifndef _NO_THREADS
		std::unique_lock<std::mutex> lock(mapMtx);
#endif
		tiMap_[name].add(duration);
#else
		func();
#endif
	}

	template<typename F> void execute(const string& parentName, const string& name, F const &func)
	{
#ifndef _NO_TIMETRACK
		auto start = SDL_GetTicks();
		func();
		auto duration = SDL_GetTicks() - start;

#ifndef _NO_THREADS
		std::unique_lock<std::mutex> lock(mapMtx);
#endif
		tiMap_[parentName].children_[name].add(duration);
#else
		func();
#endif
	}

	template<typename F> size_t measure(F const &func)
	{
#ifndef _NO_TIMETRACK
		auto start = SDL_GetTicks();
		func();
		auto duration = SDL_GetTicks() - start;
		return duration;
#else
		func();
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
		ss << "Time tracking info: <name>: <total> / <count> = <mean> ~ <variance>" << std::endl;
		for(std::pair<string,TimeInfo> it : tiMap_) {
			ss << pad_string("    " + it.first + ":", 25) << it.second << std::endl;
			for(auto itc : it.second.children_) {
				ss << pad_string("        " + itc.first + ":", 25) << itc.second << std::endl;
			}
		}
		ss << std::endl;

		return ss.str();
#endif
		return "";
	}

	const map<string, TimeInfo>& getMap() {
		return tiMap_;
	}

	void reset() {
		tiMap_.clear();
	}

	static TimeTracker& getInstance() {
		if(instance_ == NULL)
			instance_ = new TimeTracker();

		return *instance_;
	}

  static void destroy() {
    if(instance_)
      delete instance_;

    instance_ = NULL;
  }

  static void epoch() {
    for(auto pair : instance_->tiMap_) {
      pair.second.newGame();
      for(auto pairc : pair.second.children_) {
        pairc.second.newGame();
      }
    }
  }
};

} /* namespace fractaldive */

#endif /* SRC_TIMETRACKER_HPP_ */
