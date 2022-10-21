#pragma once
#include <ostream>
#include <chrono>
#include <syncstream>
#include <mutex>
//includes some type aliases and a concept to make it single header, class DelayManager
namespace DelayManagement
{
	//using declarations, aliases
	namespace chron = std::chrono;
	using Time_t = chron::nanoseconds;
	using Clock_t = chron::steady_clock;
	using TimePoint_t = chron::time_point <Clock_t, Time_t>;

	//concept for DelayManager or something usable as such.
	template<typename T>
	concept IsDelayManager = requires(T & t)
	{
		{ T(std::chrono::seconds(1)) };
		{ t.IsElapsed() } -> std::convertible_to<bool>;
		{ t.Reset(std::chrono::seconds{ 5 }) };
		{ std::cout << t };
	};

	//concept for template type convertible to Time_t
	template<typename T>
	concept IsDuration = std::is_convertible_v<T, Time_t>;

	/// <summary> DelayManager manages a non-blocking time delay, it provides functions such as IsElapsed() and Reset(...) </summary>
	/// <typeparam name="Duration_t">Chrono type convertible to <c>std::chrono::nanoseconds</c> </typeparam>
	template<IsDuration Duration_t = chron::nanoseconds>
	class DelayManager final
	{
		TimePoint_t m_start_time{ Clock_t::now() };
		Time_t m_nanos{};
		bool m_has_fired{ false };
	public:
		DelayManager() = delete;
		explicit DelayManager(Duration_t duration) noexcept : m_nanos(duration) { }
		DelayManager(const DelayManager& other) = default;
		DelayManager(DelayManager&& other) = default;
		DelayManager& operator=(const DelayManager& other) = default;
		DelayManager& operator=(DelayManager&& other) = default;
		~DelayManager() = default;
		/// <summary> Operator<< overload for ostream specialization,
		///	writes more detailed delay details for debugging.
		///	Thread-safe, provided all writes to the ostream object
		///	are wrapped with osyncstream! </summary>
		friend std::ostream& operator<<(std::ostream& os, const DelayManager& obj) noexcept
		{
			std::osyncstream ss(os);
			ss << "[DelayManager]" << std::endl
				<< "m_start_time:" << obj.m_start_time.time_since_epoch() << std::endl
				<< "m_nanos:" << obj.m_nanos << std::endl
				<< "m_has_fired:" << obj.m_has_fired << std::endl
				<< "[/DelayManager]";
			return os;
		}
		/// <summary>Check for elapsed.</summary>
		[[nodiscard]]
		bool IsElapsed() noexcept
		{
			if (Clock_t::now() > (m_start_time + m_nanos))
			{
				m_has_fired = true;
				return true;
			}
			return false;
		}
		/// <summary>Reset delay for elapsing.</summary>
		void Reset(const Duration_t nanosec_delay) noexcept
		{
			m_start_time = Clock_t::now();
			m_has_fired = false;
			m_nanos = nanosec_delay;
		}
	};
	
}
