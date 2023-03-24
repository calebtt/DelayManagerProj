#pragma once
#include <ostream>
#include <chrono>
#include <mutex>
//includes some type aliases and a concept to make it single header, class DelayManager
namespace DelayManagement
{
	//using declarations, aliases
	namespace chron = std::chrono;
	using Nanos_t = chron::nanoseconds;
	using Clock_t = chron::steady_clock;
	using TimePoint_t = chron::time_point <Clock_t, Nanos_t>;

	//concept for DelayManager or something usable as such.
	template<typename T>
	concept IsDelayManager = requires(T& t)
	{
		{ T(std::chrono::seconds(1)) };
		{ t.IsElapsed() } -> std::convertible_to<bool>;
		{ t.Reset(std::chrono::seconds{ 5 }) };
		{ std::cout << t };
	};

	/// <summary> DelayManager manages a non-blocking time delay, it provides functions such as IsElapsed() and Reset(...) </summary>
	class DelayManager final
	{
		TimePoint_t m_start_time{ Clock_t::now() };
		Nanos_t m_delayTime{}; // this should remain nanoseconds to ensure maximum granularity when Reset() with a different type.
		mutable bool m_has_fired{ false };
	public:
		DelayManager() = delete;
		explicit DelayManager(Nanos_t duration) noexcept : m_delayTime(duration) { }
		DelayManager(const DelayManager& other) = default;
		DelayManager(DelayManager&& other) = default;
		DelayManager& operator=(const DelayManager& other) = default;
		DelayManager& operator=(DelayManager&& other) = default;
		~DelayManager() = default;
		/// <summary>
		/// Operator<< overload for ostream specialization,
		///	writes more detailed delay details for debugging.
		///	Thread-safe, provided all writes to the ostream object
		///	are wrapped with osyncstream!
		///	</summary>
		friend std::ostream& operator<<(std::ostream& os, const DelayManager& obj) noexcept
		{
			os << "[DelayManager]" << '\n'
				<< "m_start_time:" << obj.m_start_time.time_since_epoch() << '\n'
				<< "m_delayTime:" << obj.m_delayTime << '\n'
				<< "m_has_fired:" << obj.m_has_fired << '\n'
				<< "[/DelayManager]";
			return os;
		}
		/// <summary>
		/// Check for elapsed.
		/// </summary>
		[[nodiscard]]
		bool IsElapsed() const noexcept
		{
			if (Clock_t::now() > (m_start_time + m_delayTime))
			{
				m_has_fired = true;
				return true;
			}
			return false;
		}
		/// <summary>
		/// Reset timer with chrono duration type.
		/// </summary>
		void Reset(const Nanos_t delay) noexcept
		{
			m_start_time = Clock_t::now();
			m_has_fired = false;
			m_delayTime = { delay };
		}
		/// <summary>
		/// Reset timer to last used duration value for a new start point.
		/// </summary>
		void Reset() noexcept
		{
			m_start_time = Clock_t::now();
			m_has_fired = false;
		}
	};
	
}
