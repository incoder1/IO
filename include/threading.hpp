#ifndef __IO_THREADING_HPP_INCLUDED__
#define __IO_THREADING_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCEs

#if  defined(__IO_WINDOWS_BACKEND__)

#	include "win/criticalsection.hpp"

#elif defined(__IO_POSIX__BACKEND__)

#	include "posix/criticalsection.hpp"

#else

#include <mutex>

namespace io {

	using lock_guard = std::lock_guard;
	typedef std::mutex critical_section;

} // namespace io

#endif // __IO_WINDOWS_BACKEND__

#endif // __IO_THREADING_HPP_INCLUDED__
