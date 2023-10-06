/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_RWLOCK_HPP_INCLUDED__
#define __IO_RWLOCK_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifndef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "posixconf.hpp"
#include <pthread.h>

namespace io {

class read_write_barrier
{
	read_write_barrier(const read_write_barrier&) = delete;
	read_write_barrier& operator=(const read_write_barrier&) = delete;
public:
	read_write_barrier() noexcept:
        barrier_()
	{
		::pthread_rwlockattr_t attr;
		::pthread_rwlockattr_init(&attr);
		::pthread_rwlock_init(&barrier_,&attr);
		::pthread_rwlockattr_destroy(&attr);
	}
	~read_write_barrier() noexcept
	{
		::pthread_rwlock_destroy(&barrier_);
	}
	inline void read_lock() noexcept
	{
		::pthread_rwlock_rdlock(&barrier_);
	}
	inline void read_unlock() noexcept
	{
		::pthread_rwlock_unlock(&barrier_);
	}
	inline void write_lock() noexcept
	{
		::pthread_rwlock_wrlock(&barrier_);
	}
	inline void write_unlock() noexcept
	{
		::pthread_rwlock_unlock(&barrier_);
	}
private:
	::pthread_rwlock_t barrier_;
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


#endif // __IO_RWLOCK_HPP_INCLUDED__
