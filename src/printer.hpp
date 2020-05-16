#ifndef SRC_PRINTER_HPP_
#define SRC_PRINTER_HPP_

#include <cstdio>

#ifdef _AMIGA
#include <sstream>
#else
#include <iostream>
#endif

#ifndef _NO_THREADS
#include <mutex>
#endif

namespace fractaldive {

class Printer {
private:
	static Printer* instance_;
#ifndef _NO_THREADS
	static std::mutex instanceMtx_;
#endif

#ifdef _AMIGA
	std::stringstream buffer_;
#endif

	Printer() {};
	virtual ~Printer() {};
public:
	static Printer& getInstance() {
#ifndef _NO_THREADS
		std::unique_lock<std::mutex> lock(instanceMtx_);
#endif
		if(instance_ == nullptr)
			instance_ = new Printer();

		return *instance_;
	}

	void print() {
#ifdef _AMIGA
		printf("%s", "\n");
#else
		std::cout << std::endl;
#endif
	}

	template<typename T, typename ...TAIL>
	void print(const T &t, TAIL ... tail) {
#ifdef _AMIGA
		buffer_.clear();
		buffer_ << t << ' ';
		printf("%s", buffer_.str().c_str());
		print(tail...);
#else
		std::cout << t << ' ';
		print(tail...);
#endif
	}

	void printErr() {
#ifdef _AMIGA
		printf("%s", "\n");
#else
		std::cerr << std::endl;
#endif
	}

	template<typename T, typename ...TAIL>
	void printErr(const T &t, TAIL ... tail) {
#ifdef _AMIGA
		buffer_.clear();
		buffer_ << t << ' ';
		printf("Error: %s", buffer_.str().c_str());
		print(tail...);
#else
		std::cerr << t << ' ';
		print(tail...);
#endif
	}
};

} /* namespace fractaldive */

#endif /* SRC_PRINTER_HPP_ */
