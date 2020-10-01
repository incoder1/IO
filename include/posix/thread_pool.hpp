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
#ifndef __IO_POSIX_THREAD_POOL_HPP_INCLUDED__
#define __IO_POSIX_THREAD_POOL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <pthread.h>

#include <atomic>
#include <deque>
#include <functional>
#include <forward_list>

#include "object.hpp"

namespace io {

class thread_pool;
DECLARE_IPTR(thread_pool);

typedef std::function<void()> async_task;

namespace detail {

class blocking_queue {
	blocking_queue(const blocking_queue&) = delete;
	blocking_queue& operator=(const blocking_queue&) = delete;
public:
	blocking_queue() noexcept;
	~blocking_queue() noexcept;

	async_task poll();

	void offer(async_task&& el) noexcept;

private:
	::pthread_mutex_t mtx_;
	::pthread_cond_t cv_;
	std::deque<async_task, io::h_allocator<async_task> > queue_;
};

class pool_thread {
	pool_thread(const pool_thread&) = delete;
	pool_thread& operator=(const pool_thread&) = delete;
public:
	pool_thread(thread_pool* owner, void* (*routine)(void*)) noexcept;
	~pool_thread() noexcept;
	void join() noexcept;
private:
	::pthread_t tid_;
};

} // namespace detail

/// !\brief Executes each submitted task using one of possibly several pooled threads
class IO_PUBLIC_SYMBOL thread_pool:public io::object {
    thread_pool(const thread_pool&) = delete;
    thread_pool operator=(const thread_pool&) = delete;
public:

	/// Creates new pool
	/// \param ec operation error code
	/// \param max_threads maximum threads in this pool
	/// \return pointer on new thread pool reference, or empty intrusive_ptr in case of error
	static s_thread_pool create(std::error_code& ec, unsigned int max_threads) noexcept;

	virtual ~thread_pool() noexcept override;

	/// Return maximum threads count int this pool
    inline unsigned int max_threads() noexcept
    {
        return max_threads_;
    }

    /// Submit an asynchronous task to be executed in a pool thread
    /// \param ec operation error code
	/// \param task a task
    void sumbmit(std::error_code& ec,async_task&& task) noexcept;

    /// Sends stop signal for all pool threads, and waits until all pending operations done
    void join() noexcept;

private:
	thread_pool(std::size_t max_threads) noexcept;
	static void* generic_routine(void* self) noexcept;
private:
	detail::blocking_queue queue_;
	std::forward_list< detail::pool_thread, io::h_allocator<detail::pool_thread> > pool_;
	std::size_t max_threads_;
	std::atomic_bool active_;
};

} // namespace io

#endif // __IO_POSIX_THREAD_POOL_HPP_INCLUDED__
