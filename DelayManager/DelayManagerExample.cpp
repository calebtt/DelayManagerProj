// DelayManager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include "DelayManager.hpp"

int main()
{
	using std::cout;
	using std::endl;
	using sds::Utilities::DelayManager;
	namespace chron = std::chrono;
	auto DoLoop = [](const auto durationValue, const auto interval)
	{
		DelayManager timer(durationValue);
		while (!timer.IsElapsed())
		{
			std::this_thread::sleep_for(interval);
			cout << "Tick..." << endl;
		}
	};
	DoLoop(chron::seconds(5), chron::seconds(1));
	DoLoop(chron::milliseconds(1000), chron::milliseconds(100));
}
