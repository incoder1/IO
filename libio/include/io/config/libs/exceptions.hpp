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
#ifndef __IO_EXCEPTIONS_HPP_INCLUDED__
#define __IO_EXCEPTIONS_HPP_INCLUDED__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>
#include <type_traits>

#ifndef IO_NO_EXCEPTIONS
#	include <stdexcept>
#endif // IO_NO_EXCEPTIONS

namespace io {

	template<typename T>
	class nobadalloc {
		nobadalloc() = delete;
		nobadalloc(const nobadalloc&) = delete;
		nobadalloc& operator=(const nobadalloc&) = delete;
		~nobadalloc() = delete;
	public:

	 template<typename... _Args>
	 static T* construct(std::error_code& ec,_Args&&... __args) noexcept( noexcept( T(std::forward<_Args>(__args)...) ) ) {
			T* result = new ( std::nothrow ) T( std::forward<_Args>(__args)... );
			if(nullptr == result)
				ec = std::make_error_code(std::errc::not_enough_memory);
			return result;
	}

	};


} // namespace io

#endif // __IO_EXCEPTIONS_HPP_INCLUDED__
