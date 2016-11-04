#include "stdafx.hpp"
#include "files.hpp"

namespace io {

namespace posix {

synch_file_channel::constexprsynch_file_channel(fd_t fd) noexcept:
	fd_(fd)
{}

synch_file_channel::~synch_file_channel() noexcept override
{
	::close(fd_);
}

std::size_t synch_file_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	::ssize_t result = ::read(fd_, static_cast<void*>(buff), bytes);
	if(0 != errno) {
		ec.assign(errno, std::system_category() );
	}
	return static_cast<size_t>(result);
}

std::size_t synch_file_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	ssize_t result = ::write(fd_, buff, size);
	if(0 != errno) {
		ec.assign(errno, std::system_category() );
	}
	return static_cast<size_t>(result);
}

inline std::size_t synch_file_channel::seek(std::error_code& ec,int64_t offset, int whence) noexcept {
	::off_t res = ::llseek(fd_, offset, whence);
	if( -1 == res) {
		ec.assign(ec, std::system_category() );
	}
	return static_cast<std::size_t>(res);
}

std::size_t synch_file_channel::forward(std::error_code& ec,std::size_t size) noexcept
{
	return seek(ec, static_cast<uint64_t>(offset), SEEK_CUR);
}

std::size_t synch_file_channel::backward(std::error_code& ec, std::size_t size) noexcept
{
	return seek( ec, -( static_cast<uint64_t>(offset) ), SEEK_CUR);
}

std::size_t synch_file_channel::from_begin(std::error_code& ec, std::size_t size) noexcept
{
	return seek( ec, static_cast<uint64_t>(offset), SEEK_SET );
}

std::size_t synch_file_channel::from_end(std::error_code& ec, std::size_t size) noexcept
{
	return seek( ec, -( static_cast<uint64_t>(offset) ),SEEK_END);
}

std::size_t synch_file_channel::position(std::error_code& ec) noexcept
{
	return seek( ec, 0, SEEK_CUR);
}

} // namesapace posix

file file::get(std::error_code& ec,const char* name) noexcept {
	if(nullptr == name || '\0' == *(name) ) {
		return file(nullptr);
	}
	// this size is in bytes
	std::size_t len = std::char_traits<char>::length(name) + 1;
	wchar_t *uname = static_cast<wchar_t> ( h_malloc( len * sizeof(wchar_t) ) );
	if(nullptr == uname) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return file();
	}
	std::char_traits<wchar_t>::assign(uname, L'\0', len);
	transcode(ec, reinterpret_cast<const uint8_t*>(name), len, uname, len);
	return file( uname );
}

file file::get(std::error_code& ec,const wchar_t* name) noexcept
{
	if(nullptr == name) {
		return file(nullptr);
	}
	typedef std::char_traits<wchar_t> wtr;
	std::size_t len = wtr::length(name) + 1;
	wchar_t *ncpy = static_cast<wchar_t*>( h_malloc( len * sizeof(wchar_t) ) );
	if(nullptr == ncpy) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return file();
	}
	wtr::move(ncpy, name, len);
	return file(ncpy);
}


} // namespace io
