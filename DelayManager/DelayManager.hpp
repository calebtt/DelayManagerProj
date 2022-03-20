#pragma once
#include <ostream>
#include <chrono>
#include <syncstream>

namespace sds::Utilities
{
	//using declarations, aliases
	namespace chron = std::chrono;
	using TimeType = chron::nanoseconds;
	using ClockType = chron::high_resolution_clock;
	using TimePointType = chron::time_point <ClockType, TimeType>;
	

	//concept for template type convertible to TimeType
	template<typename T>
	concept IsDuration = std::is_convertible_v<T, TimeType>;

	/// <summary> DelayManager manages a time delay, it provides functions such as IsElapsed() and Reset(...) </summary>
	/// <typeparam name="DurationType">Chrono type convertible to nanoseconds</typeparam>
	template<typename DurationType>
	class DelayManager
	{
		TimePointType m_start_time{ ClockType::now() };
		TimeType m_nanos{};
		bool m_has_fired{ false };
	public:
		DelayManager() = delete;
		explicit DelayManager(DurationType duration) noexcept requires IsDuration<DurationType> : m_nanos(duration) { }
		DelayManager(const DelayManager& other) = default;
		DelayManager(DelayManager&& other) = default;
		DelayManager& operator=(const DelayManager& other) = default;
		DelayManager& operator=(DelayManager&& other) = default;
		~DelayManager() = default;
		/// <summary>Operator<< overload for ostream specialization,
		///	writes more detailed delay details for debugging.
		///	Thread-safe, provided all writes to the ostream object
		///	are wrapped with osyncstream!</summary>
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
		bool IsElapsed() noexcept
		{
			if (ClockType::now() > (m_start_time + m_nanos))
			{
				m_has_fired = true;
				return true;
			}
			return false;
		}
		/// <summary>Reset delay for elapsing.</summary>
		void Reset(DurationType nanosec_delay) noexcept requires IsDuration<DurationType>
		{
			m_start_time = ClockType::now();
			m_has_fired = false;
			m_nanos = nanosec_delay;
		}
	};
}
