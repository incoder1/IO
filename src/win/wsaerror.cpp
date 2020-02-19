#include "stdafx.hpp"
#include "wsaerror.hpp"

namespace io {

namespace net {

// error_category
const char* error_category::name() const noexcept
{
    return "Network error";
}

std::error_condition error_category::default_error_condition (int err) const noexcept
{
	return std::error_condition(err, *this);
}

bool error_category::equivalent(const std::error_code& code, int condition) const noexcept
{
	return std::error_condition(code.value(), std::system_category() ).value() == condition;
}

std::string  error_category::message(int err_code) const
{
	static constexpr ::DWORD FTM_FLAGS = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	static constexpr ::DWORD FMT_EN_US_LID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	char buff[512] = {'\0'};
	if(0 == ::FormatMessageA(FTM_FLAGS, nullptr, err_code, FMT_EN_US_LID, buff, sizeof(buff), nullptr) ) {
		std::snprintf(buff, sizeof(buff), "Network socket error no: %d ", err_code );
	}
	return std::string(buff);
}

// free functions


#ifndef  _MSC_VER
	_GLIBCXX_CONST const error_category& network_category() noexcept
#else
	const error_category& network_category() noexcept
#endif
{
	static error_category _result;
	return _result;
}

std::error_code make_wsa_last_error_code() noexcept
{
   int last_errors = ::WSAGetLastError();
   return std::error_code( last_errors , network_category() );
}

} // namespace net

} // namespace io
