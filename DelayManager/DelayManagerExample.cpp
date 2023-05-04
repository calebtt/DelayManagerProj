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


//Program settings
static constexpr size_t ThreadCount{ 20 }; // num threads
static constexpr chron::seconds FirstDelay{ 5 }; // initial delay given to the tick loop
static constexpr chron::seconds UpdatedDelay{ 10 }; // value the delay is updated to, after the time to wait
static constexpr chron::seconds TimeToWait{ 2 }; // time to wait before updating (calling Reset() on the DelayManager)


//by ref
void RunDelayLoopWithRefObj(DelayManagement::IsDelayManager auto& timer, const auto interval);
//performs some concurrent operations on the delaymanager object, multi threaded
void RunMultiThreadedTest();
//single threaded test
void RunSingleThreadedTest();
//helper fn for a spinlock-type loop that copies the delaymanager object into it.
void RunDelayLoopWithCopiedObj(DelayManagement::IsDelayManager auto timer, const auto interval);

void DelayManagementConceptTest(DelayManagement::IsDelayManager auto obj)
{
	//empty
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
	//Concept test, ensuring both/all variants pass the concept check.
	DelayManagementConceptTest(DelayManagement::DelayManager{1s});
	RunResetTest();

	std::osyncstream oss(std::cout);
	oss << "[ENTER] to run single threaded test...\n";
	oss.emit();
	std::getline(std::cin, buffer);
	RunSingleThreadedTest();
	oss << std::format("[ENTER] to continue creating {0} threads with a {1} second delay, and then Reset() the delay to {2} seconds, after {3} seconds.\n"
		, ThreadCount, FirstDelay, UpdatedDelay, TimeToWait);
	oss.emit();
	std::getline(std::cin, buffer);
	oss << "[ENTER] to exit." << newl;
	oss.emit();
	std::getline(std::cin, buffer);
}

void RunDelayLoopWithRefObj(DelayManagement::IsDelayManager auto &timer, const auto interval)
{
	while (!timer.IsElapsed())
	{
		std::this_thread::sleep_for(interval);
		std::osyncstream oss(std::cout);
		oss << "Tick... " << timer << "From Thread Id : " << std::this_thread::get_id() << newl;
		oss.emit();
	}
}

//trying to make it copy the delaymanager on purpose
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