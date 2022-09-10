/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_SRW_LOCK_HPP_INCLUDED__
#define __IO_SRW_LOCK_HPP_INCLUDED__

#ifdef HAS_PRAGMA_ONCE
#	pragma once
#endif // HAS_PRAGMA_ONCE

#include "winconf.hpp"

namespace io {

/// Windows SWR lock slim reader/writer barrier implementation
class read_write_barrier {
	read_write_barrier(const read_write_barrier&) = delete;
	read_write_barrier& operator=(const read_write_barrier&) = delete;
public:
	read_write_barrier() noexcept:
		lock_()
	{
		::InitializeSRWLock(&lock_);
	}
	~read_write_barrier() noexcept = default;
private:
	friend class read_lock;
	friend class write_lock;
	::SRWLOCK lock_;
};

class read_lock {
	read_lock(const read_lock&)	= delete;
	read_lock& operator=(const read_lock&) = delete;
public:
	explicit read_lock(read_write_barrier& br) noexcept:
		barrier_(br)
	{
		::AcquireSRWLockShared(&barrier_.lock_);
	}
	~read_lock() noexcept
	{
		::ReleaseSRWLockShared(&barrier_.lock_);
	}
private:
	read_write_barrier& barrier_;
};

class write_lock {
	write_lock(const read_lock&)	= delete;
	write_lock& operator=(const write_lock&) = delete;
public:
	explicit write_lock(read_write_barrier& br) noexcept:
		barrier_(br)
	{
		::AcquireSRWLockExclusive(&barrier_.lock_);
	}
	~write_lock() noexcept
	{
		::ReleaseSRWLockExclusive(&barrier_.lock_);
	}
private:
	read_write_barrier& barrier_;
};

} // namespace io

#endif // __IO_SRW_LOCK_HPP_INCLUDED__
