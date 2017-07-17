#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <pthread.h>

namespace io {

class critical_section
{
	critical_section(const critical_section&) = delete;
	critical_section& operator=(const critical_section&) = delete;
public:
	critical_section() noexcept
	{
		pthread_spin_init(&sl_, 0);
	}
	~critical_section() noexcept
	{
		pthread_spin_destroy(&sl_);
	}
	__forceinline void lock() noexcept
	{
		pthread_spin_lock(&sl_);
	}
	__forceinline bool try_lock() noexcept {
		return 0 == pthread_spin_trylock(&sl);
	}
	__forceinline void unlock() noexcept {
		pthread_spin_unlock(&sl_);
	}
private:
	::pthread_spinlock_t sl_;
};

class lock_guard
{
	lock_guard(const lock_guard& ) = delete;
	lock_guard& operator=(const lock_guard& ) = delete;
public:
	lock_guard(critical_section& cs) noexcept:
		cs_(cs)
	{
		cs_.lock();
	}
	~lock_guard() noexcept {
		cs_.unlock();
	}
private:
	critical_section& cs_;
};

} // namespace io

#endif // CRITICALSECTION_H
