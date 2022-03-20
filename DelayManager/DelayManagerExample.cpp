// DelayManager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include "DelayManager.hpp"

using sds::Utilities::DelayManager;

template<typename T>
void CopyDelay(DelayManager<T> timer, const auto interval)
{
	while (!timer.IsElapsed())
	{
		std::this_thread::sleep_for(interval);
		std::cout << "Tick..." << std::endl;
	}
}

int main()
{
	using std::cout;
	using std::endl;
	namespace chron = std::chrono;
	auto DoLoop = [](const auto durationValue, const auto interval)
	{
		DelayManager timer(durationValue);
		CopyDelay(timer, interval);
	};
	DoLoop(chron::seconds(5), chron::seconds(1));
	DoLoop(chron::milliseconds(1000), chron::milliseconds(100));
}
