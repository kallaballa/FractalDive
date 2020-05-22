#ifndef SRC_TIMETRACKER_HPP_
#define SRC_TIMETRACKER_HPP_

#include <SDL/SDL.h>
#include <map>
#include <string>
#include <sstream>
#include <limits>

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
	map<string, TimeInfo> children_;

	void add(size_t t) {
		last_ = t;
		totalTime_ += t;
    gameTime_ += t;
		++totalCnt_;
    ++gameCnt_;

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
		ss << (double)totalTime_/(double)totalCnt_/1000.0 << " = (" << (double)totalTime_/(double)1000.0 << '\\' << totalCnt_ << ')';
		return ss.str();
	}
};

inline std::ostream& operator<<(ostream& os, TimeInfo& ti) {
	os << (double)ti.totalTime_/(double)ti.totalCnt_/1000 << " = (" << (double)ti.totalTime_/(double)1000 << '\\' << ti.totalCnt_ << ')';
	return os;
}

class TimeTracker {
private:
	static TimeTracker* instance_;

	 map<string, TimeInfo> tiMap_;
	 bool enabled_;
	TimeTracker();
public:
	virtual ~TimeTracker();

	template<typename F> void execute(const string& name, F const &func)
	{
		auto start = SDL_GetTicks();
		func();
		auto duration = SDL_GetTicks() - start;
		tiMap_[name].add(duration);
	}

	template<typename F> void execute(const string& parentName, const string& name, F const &func)
	{
		auto start = SDL_GetTicks();
		func();
		auto duration = SDL_GetTicks() - start;

		tiMap_[parentName].children_[name].add(duration);
	}

	template<typename F> size_t measure(F const &func)
	{
		auto start = SDL_GetTicks();
		func();
		auto duration = SDL_GetTicks() - start;
		return duration;
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
		stringstream ss;
		ss << "Time tracking info: " << std::endl;
		for(auto it : tiMap_) {
			ss << "\t" << it.first << ": " << it.second << std::endl;
			for(auto itc : it.second.children_) {
				ss << "\t\t" << itc.first << ": " << itc.second << std::endl;
			}
		}
		ss << std::endl;

		return ss.str();
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

  static void newGame() {
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
