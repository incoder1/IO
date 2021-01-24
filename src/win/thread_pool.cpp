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
#include "stdafx.hpp"
#include "thread_pool.hpp"

#ifdef __GNUG__

inline void InitializeThreadpoolEnvironment(PTP_CALLBACK_ENVIRON cbe) noexcept
{
	TpInitializeCallbackEnviron (cbe);
}

inline void SetThreadpoolCallbackCleanupGroup(PTP_CALLBACK_ENVIRON cbe, PTP_CLEANUP_GROUP cg, PTP_CLEANUP_GROUP_CANCEL_CALLBACK cgccb) noexcept
{
	TpSetCallbackCleanupGroup( cbe, cg, cgccb );
}

inline void DestroyThreadpoolEnvironment(PTP_CALLBACK_ENVIRON cbe) noexcept
{
	TpDestroyCallbackEnviron(cbe);
}

#endif // __GNUG__

namespace io {

void CALLBACK thread_pool::routine_wrapper(::PTP_CALLBACK_INSTANCE instance,::PVOID context, ::PTP_WORK work) noexcept
{
    std::unique_ptr<task_context> ctx(static_cast<task_context*>(context));
    ctx->call();
}

s_thread_pool thread_pool::create(std::error_code& ec, unsigned int max_threads) noexcept
{
    ::PTP_POOL id = ::CreateThreadpool(nullptr);
    if( nullptr == id) {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        return s_thread_pool();
    }

    if( FALSE == ::SetThreadpoolThreadMinimum(id, 1) ) {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        ::CloseThreadpool(id);
        return s_thread_pool();
    }

    ::SetThreadpoolThreadMaximum(id, max_threads );

    ::PTP_CLEANUP_GROUP cleanup_group = ::CreateThreadpoolCleanupGroup();
    if( nullptr == cleanup_group) {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        ::CloseThreadpool(id);
        return s_thread_pool();
    }

    thread_pool *ret = new ( std::nothrow ) thread_pool(id, max_threads, cleanup_group);
    if( nullptr == ret) {
        ec = std::make_error_code(std::errc::not_enough_memory);
        ::CloseThreadpoolCleanupGroup(cleanup_group);
        ::CloseThreadpool(id);
        return s_thread_pool();
    }
    return s_thread_pool(ret);
}

thread_pool::thread_pool(::PTP_POOL id, ::DWORD max_threads, ::PTP_CLEANUP_GROUP cleanup_group) noexcept:
    object(),
    id_(id),
    max_threads_(max_threads),
    cleanup_group_(cleanup_group),
    cbenv_()
{
    InitializeThreadpoolEnvironment(&cbenv_);
    SetThreadpoolCallbackCleanupGroup(&cbenv_, cleanup_group_, [](PVOID object_context, PVOID cleanup_context) {});
}

thread_pool::~thread_pool() noexcept
{
    // Clean up the cleanup_group
    ::CloseThreadpoolCleanupGroupMembers(cleanup_group_, FALSE, this);
    ::CloseThreadpoolCleanupGroup(cleanup_group_);
    ::DestroyThreadpoolEnvironment(&cbenv_);
    // Close thread pool
    ::CloseThreadpool(id_);
}

void thread_pool::sumbmit(std::error_code& ec,async_task&& routine) noexcept
{
    task_context *tc = new (std::nothrow) task_context( std::forward<async_task>(routine) );
    if(nullptr == tc) {
        ec = std::make_error_code(std::errc::not_enough_memory);
    }
    else {
        ::PTP_WORK work = ::CreateThreadpoolWork(&thread_pool::routine_wrapper, tc, &cbenv_);
        ::SubmitThreadpoolWork(work);
    }
}

void thread_pool::join() noexcept
{
	// Close members and wait for all pending operations
	::CloseThreadpoolCleanupGroupMembers(cleanup_group_, FALSE, this);
}

} // namespace io
