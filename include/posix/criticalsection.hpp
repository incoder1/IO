/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_CRITICALSECTION_HPP_INCLUDED__
#define __IO_CRITICALSECTION__HPP_INCLUDED__

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
	critical_section() noexcept:
        sl_()
	{
		::pthread_spin_init(&sl_, 0);
	}
	~critical_section() noexcept
	{
		::pthread_spin_destroy(&sl_);
	}
	inline void lock() noexcept
	{
		::pthread_spin_lock(&sl_);
	}
	inline bool try_lock() noexcept {
		return 0 == ::pthread_spin_trylock(&sl_);
	}
	inline void unlock() noexcept {
		::pthread_spin_unlock(&sl_);
	}
private:
	::pthread_spinlock_t sl_;
};

class lock_guard
{
	lock_guard(const lock_guard& ) = delete;
	lock_guard& operator=(const lock_guard& ) = delete;
public:
	explicit lock_guard(critical_section& cs) noexcept:
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

#endif // __IO_CRITICALSECTION__HPP_INCLUDED__
