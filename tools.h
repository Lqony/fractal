#pragma once


#include <time.h>
#include <chrono>

inline uint64_t get_timestamp_ms()
{
	using namespace std::chrono;
	return time_point_cast<milliseconds>(steady_clock::now()).time_since_epoch().count();
};