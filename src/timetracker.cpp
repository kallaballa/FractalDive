/*
 * timetracker.cpp
 *
 *  Created on: May 22, 2020
 *      Author: elchaschab
 */

#include "timetracker.hpp"

namespace fractaldive {

TimeTracker* TimeTracker::instance_;

TimeTracker::TimeTracker() : enabled_(false) {
}

TimeTracker::~TimeTracker() {
}

} /* namespace fractaldive */
