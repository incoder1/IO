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
#ifndef __IO_WIN_THREAD_POOL_HPP__INCLUDED__
#define __IO_WIN_THREAD_POOL_HPP__INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <functional>

namespace io {

class thread_pool;
DECLARE_IPTR(thread_pool);

typedef std::function<void()> async_task;

/// !\brief Executes each submitted task using one of possibly several pooled threads
class IO_PUBLIC_SYMBOL thread_pool:public io::object {
    thread_pool(const thread_pool&) = delete;
    thread_pool operator=(const thread_pool&) = delete;
private:
	struct task_context {
		explicit task_context(async_task&& task) noexcept:
			task_( std::forward<async_task>(task) )
		{}
		inline void call() noexcept {
			task_();
		}
	private:
		async_task task_;
	};
public:

	/// Creates new pool
	/// \param ec operation error code
	/// \param max_threads maximum threads in this pool
	/// \return pointer on new thread pool reference, or empty intrusive_ptr in case of error
    static s_thread_pool create(std::error_code& ec, unsigned int max_threads) noexcept;

    virtual ~thread_pool() noexcept;

    /// Return maximum threads count int this pool
    inline unsigned int max_threads() noexcept
    {
        return max_threads_;
    }

	/// Submit an asynchronous task to be executed in a pool thread
    /// \param ec operation error code
	/// \param task a task
    void sumbmit(std::error_code& ec,async_task&& routine) noexcept;

    /// Sends stop signal for all pool threads, and waits until all pending operations done
    void join() noexcept;

private:

    explicit thread_pool(::PTP_POOL id, ::DWORD max_threads, ::PTP_CLEANUP_GROUP cleanup_group) noexcept;

    static void CALLBACK routine_wrapper(::PTP_CALLBACK_INSTANCE instance,::PVOID context, ::PTP_WORK work) noexcept;

private:
    ::PTP_POOL id_;
    unsigned int max_threads_;
    ::PTP_CLEANUP_GROUP cleanup_group_;
    ::TP_CALLBACK_ENVIRON cbenv_;
};

} // namespace io


#endif // __IO_WIN_THREAD_POOL_HPP__INCLUDED__
