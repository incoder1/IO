#include "stdafx.hpp"
#include "asynch_channel.hpp"

namespace io {

namespace win {

asynch_channel::asynch_channel(::HANDLE hres, const asynch_callback& rc, const asynch_callback& wc) noexcept:
	asynch_read_write_channel(rc,wc),
	hch_(hres)
{

}

asynch_channel::~asynch_channel() noexcept
{}

void asynch_channel::read(std::size_t bytes, std::size_t pos) const noexcept
{
	std::error_code ec;
	uint8_t *buff = static_cast<uint8_t*>( memory_traits::malloc(bytes) );
	if( nullptr == buff ) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		on_read_finished(ec, pos, byte_buffer() );
		return;
	}
	LARGE_INTEGER p;
	p.QuadPart = pos;
	::OVERLAPPED ovlp;
	ovlp.Offset = p.LowPart;
	ovlp.OffsetHigh = p.HighPart;
	hch_.asynch_read(&ovlp, buff, bytes);
}

void asynch_channel::write(byte_buffer&& buff, std::size_t pos) const noexcept
{
}

bool asynch_channel::cancel_pending() const noexcept
{
	return TRUE == ::CancelIo( hch_.hnd() );
}

bool asynch_channel::cancel_all() const noexcept
{
	return TRUE == ::CancelIoEx( hch_.hnd(), nullptr);
}



} // namespace io {

} // namespace win {

