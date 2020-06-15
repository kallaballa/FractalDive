#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include "types.hpp"

namespace fractaldive {

class Config {
private:
	static Config* instance_;
	Config();
	virtual ~Config();
public:
	fd_dim_t width_ = 0;
	fd_dim_t height_ = 0;
	fd_dim_t frameSize_ = 0;
	fd_dim_t frameTiling_ = 0;
	size_t panSmoothLen_ = 0;
	fd_highres_tick_t benchmarkTimeoutMillis_ = 0;
	fd_iter_count_t startIterations_ = 0;
	fd_iter_count_t minIterations_ = 0;
	fd_iter_count_t maxIterations_ = 0;
	fd_float_t detailThreshold_ = 0;
	fd_float_t zoomFactor_ = 1.5;
	fd_float_t zoomSpeed_ = 0;
	fd_float_t fps_ = 0;

	static Config& getInstance() {
		if(instance_ == nullptr)
			instance_ = new Config();

		return *instance_;
	}

	void resetToDefaults();
};

} /* namespace fractaldive */

#endif /* SRC_CONFIG_HPP_ */
