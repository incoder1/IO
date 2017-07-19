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

namespace io {

static inline ::WORD current_stream_attr() noexcept
{
	::CONSOLE_SCREEN_BUFFER_INFO info;
	::GetConsoleScreenBufferInfo( ::GetStdHandle(STD_ERROR_HANDLE), &info);
	return info.wAttributes;
}

class output_swap {
	output_swap(const output_swap&) = delete;
	output_swap operator=(output_swap&) = delete;
public:
	output_swap( ) noexcept:
		prevCP_( ::GetConsoleCP() ),
		prevAttr_( current_stream_attr() )
	{
		::SetConsoleTextAttribute( ::GetStdHandle(STD_ERROR_HANDLE), 0x0C);
		::SetConsoleCP( CP_WINUNICODE );
		::SetConsoleOutputCP( CP_WINUNICODE );
	}
	inline void pop( ) noexcept
	{
		::SetConsoleTextAttribute( ::GetStdHandle(STD_ERROR_HANDLE), prevAttr_);
		::SetConsoleCP( prevCP_ );
		::SetConsoleOutputCP( prevCP_ );
	}
private:
	::DWORD prevCP_;
	::DWORD prevAttr_;
};

extern "C" void IO_PANIC_ATTR exit_with_current_error()
{
	::DWORD lastErr =::GetLastError();

	if(lastErr) {

		output_swap oswap;

		wchar_t msg[512];

		::DWORD len = ::FormatMessageW(
		                  FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		                  NULL, lastErr,
		                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		                  (::LPWSTR) &msg,
		                  256, NULL );

		::DWORD written;
		if( !::WriteConsoleW( ::GetStdHandle(STD_ERROR_HANDLE), msg, len, &written, nullptr ) )
		{
			MessageBoxExW(NULL, msg, NULL, MB_OK | MB_ICONERROR , MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) );
		}
		oswap.pop();
	}
	std::exit( lastErr );
}

static void print_error_message(int errcode,const char* message) noexcept
{
	std::size_t len = io_strlen(message) + 33;
	output_swap oswap;
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
	::DWORD written;
	if( ! ::WriteConsoleW( ::GetStdHandle(STD_ERROR_HANDLE), msg, len, &written, nullptr ) )
	{
        MessageBoxExW(NULL, msg, NULL, MB_OK | MB_ICONERROR , MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) );
	}
	oswap.pop();
}

extern "C" void IO_PANIC_ATTR exit_with_error_message(int exitcode, const char* message)
{
	print_error_message(exitcode, message);
	std::exit( exitcode );
}

namespace detail {

extern "C" void IO_PANIC_ATTR panic(int errcode, const char* message)
{
	print_error_message(errcode, message);
	std::terminate();
}

} // namespace detail

} // namespace io

