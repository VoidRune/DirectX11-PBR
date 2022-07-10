#pragma once
#include <chrono>
using namespace std::chrono;

class Time
{
public:
	Time();
	void reset_time();
	/* Time elapsed since creation or reset_time in seconds */
	double elapsed_sec();
	/* Time elapsed since creation or reset_time in miliseconds */
	double elapsed_mili();
	/* Time elapsed since creation or reset_time in microseconds */
	double elapsed_micro();

	static void UpdateDt();
	static double GetDt();
	static double GetFPS();
private:
	static double deltaTime;
	static time_point<high_resolution_clock> sinceLastUpdate;
	time_point<high_resolution_clock> timeStamp;

};