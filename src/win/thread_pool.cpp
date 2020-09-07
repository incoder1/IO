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
inline void InitializeThreadpoolEnvironment(PTP_CALLBACK_ENVIRON CallbackEnviron) noexcept
{
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
    CallbackEnviron->Version = 3;
#else
    CallbackEnviron->Version = 1;
#endif
    CallbackEnviron->Pool = nullptr;
    CallbackEnviron->CleanupGroup = nullptr;
    CallbackEnviron->CleanupGroupCancelCallback = nullptr;
    CallbackEnviron->RaceDll = nullptr;
    CallbackEnviron->ActivationContext = nullptr;
    CallbackEnviron->FinalizationCallback = nullptr;
    CallbackEnviron->u.Flags = 0;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
    CallbackEnviron->CallbackPriority = TP_CALLBACK_PRIORITY_NORMAL;
    CallbackEnviron->Size = sizeof(TP_CALLBACK_ENVIRON);
#endif
}

inline void DestroyThreadpoolEnvironment( PTP_CALLBACK_ENVIRON CallbackEnviron) noexcept
{
    UNREFERENCED_PARAMETER(CallbackEnviron);
}
#endif // __GNUG__

namespace io {

void CALLBACK thread_pool::routine_wrapper(::PTP_CALLBACK_INSTANCE instance,::PVOID context, ::PTP_WORK work) noexcept
{
	std::shared_ptr<routine_context> ctx(static_cast<routine_context*>(context));
	ctx->call();
}

s_thread_pool thread_pool::create(std::error_code& ec, unsigned int min_threads, unsigned int max_threads) noexcept
{
    ::PTP_POOL id = ::CreateThreadpool(nullptr);
    if( nullptr == id) {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        return s_thread_pool();
    }

    if( FALSE == ::SetThreadpoolThreadMinimum(id, min_threads) ) {
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
        return s_thread_pool();
    }
    return s_thread_pool(ret);
}

thread_pool::thread_pool(::PTP_POOL id, ::DWORD max_threads, ::PTP_CLEANUP_GROUP cleanup_group) noexcept:
    object(),
    id_(id),
    max_threads_(max_threads),
    cleanup_group_(cleanup_group),
	cbenv_(),
	works_()
{
	InitializeThreadpoolEnvironment(&cbenv_);
}

thread_pool::~thread_pool() noexcept
{
    // Clean up the cleanup group.
    ::CloseThreadpoolCleanupGroupMembers(cleanup_group_, FALSE, this);
    ::CloseThreadpoolCleanupGroup(cleanup_group_);
    ::DestroyThreadpoolEnvironment(&cbenv_);
    // Close thread pool
    ::CloseThreadpool(id_);
}


void thread_pool::sumbmit(std::error_code& ec,thread_routine&& routine) noexcept
{
	routine_context *context = new (std::nothrow) routine_context( std::forward<thread_routine>(routine) );
	::PTP_WORK work = ::CreateThreadpoolWork(&thread_pool::routine_wrapper, context, &cbenv_);
	::SubmitThreadpoolWork(work);
	works_.push_front( work );
}

void thread_pool::join() noexcept
{
	while( !works_.empty() ) {
		::PTP_WORK work = works_.front();
		::WaitForThreadpoolWorkCallbacks(work, FALSE );
		::CloseThreadpoolWork(work);
		works_.pop_front();
	}
}

} // namespace io
