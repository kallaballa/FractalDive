/*
 * printer.cpp
 *
 *  Created on: May 9, 2020
 *      Author: elchaschab
 */

#include "printer.hpp"

namespace fractaldive {

Printer* Printer::instance_ = nullptr;
#ifndef _NO_THREADS
std::mutex Printer::instanceMtx_;
#endif

} /* namespace fractaldive */
