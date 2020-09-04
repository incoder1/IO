#include "stdafx.hpp"
#include "asynch_socket_channel.hpp"

namespace io {

namespace net {

asynch_socket_channel::asynch_socket_channel(::SOCKET socket, const asynch_read_completition_routine& rc, const asynch_write_completition_routine& wc) noexcept:
	asynch_channel( socket_id(socket), rc, wc)
{}

asynch_socket_channel::~asynch_socket_channel() noexcept
{
	::closesocket(native());
}

static char* cstrcast_cast(const uint8_t* px) noexcept {
	return const_cast<char*>(reinterpret_cast<const char*>(px));
}

void asynch_socket_channel::read(uint8_t* into, std::size_t limit, std::size_t start_from) const noexcept
{
	::WSABUF wsab = { static_cast<::DWORD>(limit), cstrcast_cast(into) };
	::LARGE_INTEGER pos;
	pos.QuadPart = start_from;
	win::overlapped *ovlp = new win::overlapped(win::operation::recaive,pos,into);
	::DWORD flags = 0;
	::WSARecv(native(), &wsab, 1, nullptr, &flags, reinterpret_cast<LPWSAOVERLAPPED>(ovlp), nullptr);
}

void asynch_socket_channel::write(const uint8_t* what, std::size_t bytes, std::size_t start_from) const noexcept
{
	::WSABUF wsab = { static_cast<::DWORD>(bytes), cstrcast_cast(what) };
	::LARGE_INTEGER pos;
	pos.QuadPart = start_from;
	win::overlapped *ovlp = new win::overlapped(win::operation::send,pos,  const_cast<uint8_t*>(what) );
	::DWORD flags = 0;
	::WSARecv(native(), &wsab, 1, nullptr, &flags, reinterpret_cast<LPWSAOVERLAPPED>(ovlp), nullptr);
}

bool asynch_socket_channel::cancel_pending() const noexcept
{
}


bool asynch_socket_channel::cancel_all() const noexcept
{
	return false;
}

} // namespace net

} // namespace io
