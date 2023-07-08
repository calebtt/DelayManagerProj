// DelayManager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <iostream>
#include <syncstream>
#include <memory>
#include <vector>

#include "DelayManager.hpp"

namespace chron = std::chrono;
static constexpr auto newl = '\n';
using DelayManagement::DelayManager;
using Thread_t = std::jthread;
using SharedThreadType = std::shared_ptr<Thread_t>;

void RunDelayLoopWithCopiedObj(DelayManagement::IsDelayManager auto timer, const auto interval)
{
	while (!timer.IsElapsed())
	{
		std::this_thread::sleep_for(interval);
		std::osyncstream oss(std::cout);
		oss << "Tick..." << timer << newl;
		oss.emit();
	}
}

//performs some operations on the delaymanager object single threaded
void RunSingleThreadedTest()
{
	auto DoLoop = [](const auto durationValue, const auto interval)
	{
		DelayManager timer(durationValue);
		RunDelayLoopWithCopiedObj(timer, interval);
	};
	DoLoop(chron::seconds(5), chron::seconds(1));
	DoLoop(chron::milliseconds(1000), chron::milliseconds(250));
}

void RunResetTest()
{
	using namespace std::chrono_literals;
	DelayManagement::DelayManager dm{ std::chrono::milliseconds{1000} };
	dm.Reset(std::chrono::nanoseconds{ 100 });
	std::this_thread::sleep_for(std::chrono::seconds(2));
	assert(dm.IsElapsed());
	dm.Reset(std::chrono::seconds{ 10 });
	assert(!dm.IsElapsed());
	dm.Reset();
	dm.Reset(1s);
}
/* ENTRY POINT */
int main()
{
	using namespace std::chrono_literals;
	std::string buffer;

	RunResetTest();

	std::osyncstream oss(std::cout);
	oss << "[ENTER] to run single threaded test...\n";
	oss.emit();
	std::getline(std::cin, buffer);

	RunSingleThreadedTest();

	oss.emit();
	oss << "[ENTER] to exit." << newl;
	oss.emit();
	std::getline(std::cin, buffer);
}
