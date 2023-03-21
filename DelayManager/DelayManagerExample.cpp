// DelayManager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <iostream>
#include <syncstream>
#include <memory>
#include <vector>

#include "DelayManager.hpp"
#include "DelayManagerSafe.hpp"

namespace chron = std::chrono;
static constexpr auto newl = '\n';
static constexpr auto TickInterval = chron::milliseconds(500);
using DelayManagementThreadSafe::DelayManagerSafe;
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
	DelayManagementConceptTest(DelayManagementThreadSafe::DelayManagerSafe<>{1s});
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
	RunMultiThreadedTest();
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

void RunMultiThreadedTest()
{
	//waits for duration and then updates the timer obj with the new interval
	auto RunDelayedTimerUpdate = [](auto& timer, const auto interval, const chron::milliseconds duration = chron::milliseconds(500))
	{
		std::this_thread::sleep_for(duration);
		timer.Reset(interval);
	};
	// delay and update loop
	auto DoDelayAndUpdateLoop = [&](auto& mtt)
	{
		std::osyncstream oss(std::cout);
		std::shared_ptr<Thread_t> loopThread = std::make_shared<Thread_t>([&]() { RunDelayLoopWithRefObj(mtt, TickInterval); });
		oss << "Running multi-threaded delay loop.. From thread: " << loopThread->get_id() << newl;
		//so while the loopThread is running, it will update the delaymanager object with a new duration value
		std::shared_ptr<Thread_t> updateThread = std::make_shared<Thread_t>([&]() { RunDelayedTimerUpdate(mtt, UpdatedDelay, TimeToWait); });
		oss << "Running multi-threaded delay update.. From thread: " << updateThread->get_id() << newl;
		oss.emit();
		return std::make_pair(loopThread, updateThread); // pair of threads
	};
	//buffer to hold the pairs of threads so we can join them all back and wait.
	std::vector<std::pair<SharedThreadType, SharedThreadType>> threadBuf;
	std::vector<std::unique_ptr<DelayManagerSafe<>>> delayBuf;
	for (size_t i{ 0 }; i < ThreadCount; ++i)
	{
		delayBuf.emplace_back(std::make_unique<DelayManagerSafe<>>(FirstDelay, true));
		threadBuf.emplace_back(DoDelayAndUpdateLoop(*delayBuf.back()));
	}
	for (size_t i{ 0 }; i < threadBuf.size(); ++i)
	{
		const auto& bothThreads = threadBuf[i];
		if (bothThreads.first->joinable())
			bothThreads.first->join();
		if (bothThreads.second->joinable())
			bothThreads.second->join();
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