#pragma once
#include <ostream>
#include <chrono>
#include <syncstream>
#include <mutex>

//includes some type aliases and a concept to make it single header, class DelayManagerSafe
namespace DelayManagementThreadSafe
{
	//using declarations, aliases
	namespace chron = std::chrono;
	using TimeType = chron::nanoseconds;
	using ClockType = chron::high_resolution_clock;
	using TimePointType = chron::time_point<ClockType, TimeType>;
	using MutexType = std::recursive_mutex;
	using ScopedLockType = std::scoped_lock<MutexType>;

	//concept for template type convertible to TimeType
	template<typename T>
	concept IsDuration = std::is_convertible_v<T, TimeType>;

	/// <summary> DelayManagerSafe manages a time delay, it provides functions such as IsElapsed() and Reset(...)
	/// This is the thread-safe variant of this class. </summary>
	/// <typeparam name="DurationType">Chrono type convertible to nanoseconds</typeparam>
	template<typename DurationType = chron::nanoseconds>
	class DelayManagerSafe final
	{
		TimePointType m_start_time{ ClockType::now() };
		TimeType m_nanos{};
		bool m_has_fired{ false };
		std::unique_ptr<MutexType> m_invariant_mutex{};
	private:
		/// <summary> Helper to lock a mutex, if there is a mutex. Blocking. </summary>
		/// <throws> may throw a std::system_error when errors occur, including errors from the OS. </throws>
		void StartCriticalSection(const DelayManagerSafe &other)
		{
			if (other.m_invariant_mutex)
				other.m_invariant_mutex->lock();
		}
		/// <summary> Helper to unlock a mutex, if there is a mutex. </summary>
		///	<throws> may throw a std::system_error when errors occur, including errors from the OS. </throws>
		void StopCriticalSection(const DelayManagerSafe& other)
		{
			if (other.m_invariant_mutex)
				other.m_invariant_mutex->unlock();
		}
		/// <summary> Copies data members from other, with mutex protection if it exists. Blocking.</summary>
		/// <param name="other"></param>
		void CopyDataMembersIn(const DelayManagerSafe &other)
		{
			StartCriticalSection(other);
			StartCriticalSection(*this);
			m_start_time = other.m_start_time;
			m_nanos = other.m_nanos;
			m_has_fired = other.m_has_fired;
			StopCriticalSection(*this);
			StopCriticalSection(other);
		}
	public:
		//Ctors
		DelayManagerSafe() = delete;
		explicit DelayManagerSafe(DurationType duration, const bool isThreadSafetyRequired = false) noexcept requires IsDuration<DurationType>
		: m_nanos(duration)
		{
			//existence of a value in the smart pointer from here on means thread safety is requested
			if (isThreadSafetyRequired)
				m_invariant_mutex = std::make_unique<MutexType>();
		}

		/* Copy and move operations section.. Because a mutex can't just be copied. */
		DelayManagerSafe(const DelayManagerSafe& other)
		{
			CopyDataMembersIn(other);
		}
		DelayManagerSafe(DelayManagerSafe&& other) noexcept
		{
			CopyDataMembersIn(other);
		}
		DelayManagerSafe& operator=(const DelayManagerSafe& other)
		{
			if (this == &other)
				return *this;
			CopyDataMembersIn(other);
			return *this;
		}
		DelayManagerSafe& operator=(DelayManagerSafe&& other) noexcept
		{
			if (this == &other)
				return *this;
			CopyDataMembersIn(other);
			return *this;
		}
		//Dtor
		~DelayManagerSafe()
		{
			//acquire a scoped (RAII) mutex lock before destruction,
			//destructing a locked mutex is undefined behavior
			if (m_invariant_mutex)
			{
				ScopedLockType endLock(*m_invariant_mutex);
				return;
			}
			return;
		}

		/// <summary>Operator<< overload for ostream specialization,
		///	writes more detailed delay details for debugging.
		///	Thread-safe, provided all writes to the ostream object
		///	are wrapped with osyncstream!</summary>
		friend std::ostream& operator<<(std::ostream& os, const DelayManagerSafe& obj) noexcept
		{
			StartCriticalSection(obj);
			std::osyncstream ss(os);
			ss << "[DelayManagerSafe]" << std::endl
				<< "m_start_time:" << obj.m_start_time.time_since_epoch() << std::endl
				<< "m_nanos:" << obj.m_nanos << std::endl
				<< "m_has_fired:" << obj.m_has_fired << std::endl
				<< "[/DelayManagerSafe]";
			StopCriticalSection(obj);
			return os;
		}
		/// <summary>Check for elapsed.</summary>
		bool IsElapsed() noexcept
		{
			StartCriticalSection(*this);
			bool retVal{ false };
			if (ClockType::now() > (m_start_time + m_nanos))
			{
				m_has_fired = true;
				retVal = true;
			}
			StopCriticalSection(*this);
			return retVal;
		}
		/// <summary>Reset delay for elapsing.</summary>
		void Reset(DurationType nanosec_delay) noexcept requires IsDuration<DurationType>
		{
			StartCriticalSection(*this);
			m_start_time = ClockType::now();
			m_has_fired = false;
			m_nanos = nanosec_delay;
			StopCriticalSection(*this);
		}
	};
}
