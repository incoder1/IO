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
#include <forward_list>

namespace io {

class thread_pool;
DECLARE_IPTR(thread_pool);

typedef std::function<void()> thread_routine;

class IO_PUBLIC_SYMBOL thread_pool:public io::object {
    thread_pool(const thread_pool&) = delete;
    thread_pool operator=(const thread_pool&) = delete;
private:
	struct routine_context {
		explicit routine_context(thread_routine&& routine) noexcept:
			routine_( std::forward<thread_routine>(routine) )
		{}
		void call() noexcept {
			routine_();
		}
	private:
		thread_routine routine_;
	};
public:
    static s_thread_pool create(std::error_code& ec, unsigned int min_threads, unsigned int max_threads) noexcept;

    virtual ~thread_pool() noexcept;

    inline unsigned int min_threads() noexcept
    {
        return min_threads_;
    }

    inline unsigned int max_threads() noexcept
    {
        return max_threads_;
    }

    void sumbmit(std::error_code& ec,thread_routine&& routine) noexcept;

    void join() noexcept;

private:

    explicit thread_pool(::PTP_POOL id, ::DWORD max_threads, ::PTP_CLEANUP_GROUP cleanup_group) noexcept;

    static void CALLBACK routine_wrapper(::PTP_CALLBACK_INSTANCE instance,::PVOID context, ::PTP_WORK work) noexcept;

private:
    ::PTP_POOL id_;
    unsigned int min_threads_;
    unsigned int max_threads_;
    ::PTP_CLEANUP_GROUP cleanup_group_;
    ::TP_CALLBACK_ENVIRON cbenv_;
    std::forward_list< ::PTP_WORK > works_;
};

} // namespace io


#endif // __IO_WIN_THREAD_POOL_HPP__INCLUDED__
