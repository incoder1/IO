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

#pragma once

#include "winconf.hpp"

namespace io {

/// Windows Vista+ SWR lock slim reader/writer barrier implementation
class read_write_barrier
{
	read_write_barrier(const read_write_barrier&) = delete;
	read_write_barrier& operator=(const read_write_barrier&) = delete;
public:
	read_write_barrier() noexcept:
		barier_()
	{
		::InitializeSRWLock(&barier_);
	}

	~read_write_barrier() noexcept = default;

	inline void read_lock() noexcept
	{
		::AcquireSRWLockShared(&barier_);
	}
	inline void read_unlock() noexcept
	{
		::ReleaseSRWLockShared(&barier_);
	}
	inline void write_lock() noexcept
	{
		::AcquireSRWLockExclusive(&barier_);
	}
	inline void write_unlock() noexcept
	{
        ::ReleaseSRWLockExclusive(&barier_);
	}
private:
	::SRWLOCK barier_;
};

class read_lock
{
	read_lock(const read_lock&)	= delete;
	read_lock& operator=(const read_lock&) = delete;
public:
	explicit read_lock(read_write_barrier& br) noexcept:
		barrier_(br)
	{
		barrier_.read_lock();
	}
	~read_lock() noexcept
	{
		barrier_.read_unlock();
	}
private:
	read_write_barrier& barrier_;
};

class write_lock
{
	write_lock(const read_lock&)	= delete;
	write_lock& operator=(const write_lock&) = delete;
public:
	explicit write_lock(read_write_barrier& br) noexcept:
		barrier_(br)
	{
		barrier_.write_lock();
	}
	~write_lock() noexcept
	{
		barrier_.write_unlock();
	}
private:
	read_write_barrier& barrier_;
};

} // namespace io

#endif // __IO_SRW_LOCK_HPP_INCLUDED__
