/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "posix/thread_pool.hpp"

namespace io {

namespace detail {

class strick_lock {
	strick_lock(const strick_lock&) = delete;
	strick_lock& operator=(const strick_lock&) = delete;
public:
	strick_lock(::pthread_mutex_t &mtx) noexcept:
		mtx_(mtx)
	{
		::pthread_mutex_lock(mtx_);
	}
	~strick_lock() noexcept
	{
		::pthread_mutex_unlock(mtx_);
	}
private:
	::pthread_mutex_t &mtx_;
};

blocking_queue::blocking_queue() noexcept:
	mtx_(),
	cv_()
{
	::pthread_mutex_init( &mtx_, nullptr );
	::pthread_cond_init( &cv_, nullptr);
}

blocking_queue::~blocking_queue() noexcept
{
	::pthread_cond_destroy( &cv_ );
	::pthread_mutex_destroy( &mtx_ );
}

async_task blocking_queue::poll()
{
	strick_lock(&mtx_);
	while( queue_.empty() ) {
		::pthread_cond_wait( &cv_, &mtx_ );
	}
	async_task ret = queue_.front();
	queue_.pop_front();
	return ret;
}

void blocking_queue::push(async_task&& el) noexcept
{
	strick_lock(&mtx_);
	queue_.emplace_back( std::forward<async_task&&>(el) );
}

void blocking_queue::offer(async_task&& el) noexcept
{
	push( std::forward<async_task&&>(el) );
	::pthread_cond_broadcast( &cv_ );
}

// pool_thread
pool_thread::pool_thread(thread_pool* owner, void* (*routine)(void*)) noexcept:
	tid_()
{
	::pthread_attr_t attr;
	::pthread_attr_init( &attr );
	::pthread_create(&tid_, &attr, routine, static_cast<void*>(owner) );
	::pthread_attr_destroy( &attr );
}

pool_thread::~pool_thread() noexcept
{
}

void pool_thread::join() noexcept
{
	void* res;
	::pthread_join(tid_, &res);
}

} // namespace detail

s_thread_pool thread_pool::create(std::error_code& ec, unsigned int max_threads) noexcept
{
	thread_pool *ret = new (std::nothrow) thread_pool(max_threads);
	if(nullptr == ret) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return s_thread_pool();
	}
	return s_thread_pool(ret);
}

void* thread_pool::generic_routine(void * px) noexcept
{
	thread_pool* self = static_cast<thread_pool*>(px);
	do {
		auto task = self->queue_.poll();
		task();
	}
	while( self->active_ );
	return nullptr;
}

thread_pool::thread_pool(std::size_t max_threads) noexcept:
	io::object(),
	queue_(),
	pool_(),
	max_threads_(max_threads),
	active_(true)
{
	for(std::size_t i=0; i < max_threads_; i++) {
		pool_.emplace_front( const_cast<thread_pool*>(this), &thread_pool::generic_routine );
	}
}

thread_pool::~thread_pool() noexcept
{}

void thread_pool::sumbmit(std::error_code& ec,async_task&& task) noexcept
{
	queue_.offer( std::forward<async_task>(task) );
}

void thread_pool::join() noexcept
{
	active_.store(false, std::memory_order_release);
	for (auto& t : pool_) {
		queue_.offer( [] {}  );
		t.join();
	}
}

} // namespace io
