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
#ifndef __IO_WIN_CRITICAL_SECTION_HPP_INCLUDED__
#define __IO_WIN_CRITICAL_SECTION_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

class critical_section
{
	critical_section(const critical_section&) = delete;
	critical_section& operator=(const critical_section&) = delete;
private:
	static constexpr ::DWORD SPIN_COUNT = 4000;
public:
	constexpr critical_section() noexcept:
		cs_()
	{
		::InitializeCriticalSectionAndSpinCount(&cs_, SPIN_COUNT);
	}
	__forceinline void lock() noexcept {
		::EnterCriticalSection(&cs_);
	}
	__forceinline void unlock() noexcept {
		::LeaveCriticalSection(&cs_);
	}
	__forceinline bool try_lock() noexcept {
		return TRUE == ::TryEnterCriticalSection(&cs_);
	}
	inline ~critical_section() noexcept
	{
		::DeleteCriticalSection(&cs_);
	}
private:
	::CRITICAL_SECTION cs_;
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


#endif // __IO_WIN_CRITICAL_SECTION_HPP_INCLUDED__
