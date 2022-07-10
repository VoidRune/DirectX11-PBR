#include "Time.h"

double Time::deltaTime = 0.0;
time_point<high_resolution_clock> Time::sinceLastUpdate;

Time::Time()
{
	reset_time();
}

void Time::reset_time()
{
	timeStamp = high_resolution_clock::now();
}

double Time::elapsed_sec()
{
	return 0.000001 * (time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count() - time_point_cast<microseconds>(timeStamp).time_since_epoch().count());
}
double Time::elapsed_mili()
{
	return 0.001 * (time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count() - time_point_cast<microseconds>(timeStamp).time_since_epoch().count());
}
double Time::elapsed_micro()
{
	return time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count() - time_point_cast<microseconds>(timeStamp).time_since_epoch().count();
}

void Time::UpdateDt()
{
	auto now = high_resolution_clock::now();
	deltaTime = time_point_cast<microseconds>(now).time_since_epoch().count() - time_point_cast<microseconds>(sinceLastUpdate).time_since_epoch().count();
	sinceLastUpdate = now;
}

double Time::GetDt()
{
	return deltaTime * 0.000001;
}
double Time::GetFPS()
{
	return 1.0 / deltaTime * 1000000.0;
}