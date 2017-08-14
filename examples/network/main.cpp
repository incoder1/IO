#if defined(_WIN32) || defined(_WIN64)

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
#	undef _WIN32_WINNT
#	define _WIN32_WINNT _WIN32_WINNT_VISTA
#else
#	define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif // _WIN32_WINNT

#endif // _WIN32

#include <iostream>

#include <network.hpp>
#include <errorcheck.hpp>

int main()
{
	using namespace io::net;
	std::error_code ec;
	const socket_factory* sf = socket_factory::instance(ec);
	io::check_error_code(ec);
	s_socket tpc_socket = sf->client_socket(ec, "google.com", 80);
	io::check_error_code(ec);
	io::s_read_write_channel rwch = tpc_socket->connect(ec);
	io::check_error_code(ec);
    return 0;
}
