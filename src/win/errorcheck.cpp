/*
 *
 * Copyright (c) 2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "errorcheck.hpp"

#include <exception>
#include <cwchar>

#ifndef CP_WINUNICODE
#	define CP_WINUNICODE 1200
#endif // CP_WINUNICODE 1200

#ifndef CP_UTF8
#	define CP_UTF8 65001
#endif // CP_UTF8


#ifdef _MSC_VER
#	include <Strsafe.h>
#endif

namespace io {


class output_swap {
	output_swap(const output_swap&) = delete;
	output_swap operator=(output_swap&) = delete;
private:
	static inline ::WORD current_stream_attr() noexcept {
		::CONSOLE_SCREEN_BUFFER_INFO info;
		::GetConsoleScreenBufferInfo( ::GetStdHandle(STD_ERROR_HANDLE), &info);
		return info.wAttributes;
	}
public:
	output_swap( ) noexcept:
		prevCP_( ::GetConsoleCP() ),
		prevAttr_( current_stream_attr() )
	{
		::SetConsoleTextAttribute( ::GetStdHandle(STD_ERROR_HANDLE), 0x0C);
		::SetConsoleCP( CP_WINUNICODE );
		::SetConsoleOutputCP( CP_WINUNICODE );
	}
	~output_swap() noexcept {
		::SetConsoleTextAttribute( ::GetStdHandle(STD_ERROR_HANDLE), prevAttr_);
		::SetConsoleCP( prevCP_ );
		::SetConsoleOutputCP( prevCP_ );
	}
private:
	::DWORD prevCP_;
	::DWORD prevAttr_;
};

static void print_error_message(int errcode,const char* message) noexcept
{
	std::size_t len = io_strlen(message) + 33;
#ifdef _MSC_VER
	wchar_t *msg = static_cast<wchar_t*> ( io_alloca( len * sizeof(wchar_t) ) );
	len = ::StringCchPrintfW( msg, len, L"error code: %d %Z \n", errcode, message);
#else
	char* tmp = static_cast<char*> ( io_alloca( len ) );
	io_memset( tmp, 0, len);
	__builtin_snprintf(tmp, len, "error code: %d %s \n", errcode, message);
	std::size_t wlen = ::MultiByteToWideChar( CP_UTF8, 0, tmp, -1, NULL, 0 );
	wchar_t *msg = static_cast<wchar_t*>( io_alloca( wlen ) );
	io_memset( msg, 0, len);
	::MultiByteToWideChar( CP_UTF8, 0, tmp, -1, msg, wlen);
	len = wlen;
#endif
	output_swap oswap;
	::DWORD written;
	if( ! ::WriteConsoleW( ::GetStdHandle(STD_ERROR_HANDLE), msg, len, &written, nullptr ) ) {
		MessageBoxExW(NULL, msg, NULL, MB_OK | MB_ICONERROR, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) );
	}
}

extern "C" {

void IO_PANIC_ATTR exit_with_current_error()
{
	::DWORD lastErr =::GetLastError();
	if( NO_ERROR != lastErr ) {

		output_swap oswap;

		wchar_t msg[512];

		::DWORD len = ::FormatMessageW(
						  FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						  NULL, lastErr,
						  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						  (::LPWSTR) &msg,
						  256, NULL );

		::DWORD written;
		if( !::WriteFile( ::GetStdHandle(STD_ERROR_HANDLE), msg, len, &written, nullptr ) ) {
			MessageBoxExW(NULL, msg, NULL, MB_OK | MB_ICONERROR, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) );
		}
	}
	std::exit( lastErr );
}

void IO_PANIC_ATTR exit_with_error_message(int exitcode, const char* message)
{
	print_error_message(exitcode, message);
	std::exit( exitcode );
}

void IO_PANIC_ATTR panic(int errcode, const char* message)
{
	exit_with_error_message(errcode, message);
}

} // extern "C"

namespace detail {

void IO_PUBLIC_SYMBOL ios_check_error_code(const char* msg, std::error_code const &ec )
{
	if( io_likely( !ec ) )
		return;
#ifdef IO_NO_EXCEPTIONS
	std::string m = ec.message();
	std::size_t size = io_strlen(msg) + m.length() + 1;
	char *errmsg = static_cast<char*>( io_alloca( size) );
	io_zerro_mem(errmsg, size);
	io_strcpy(errmsg, msg);
	io_strcpy( ( (errmsg) + io_strlen(msg) ), m.data() );
	panic( ec.value(), errmsg );
#else
	throw std::ios_base::failure( msg, ec );
#endif
}

} // namespace detail

} // namespace io


