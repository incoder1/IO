/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "files.hpp"

#include <unistd.h>
#include <fcntl.h>

namespace io {

namespace posix {


synch_file_channel* new_sync_file_channel(std::error_code& ec, fd_t fd) noexcept
{
	return nobadalloc<synch_file_channel>::construct(ec, fd);
}

synch_file_channel::synch_file_channel(fd_t fd) noexcept:
	random_access_channel(),
	fd_(fd)
{}

synch_file_channel::~synch_file_channel() noexcept
{
	::close(fd_);
}

std::size_t synch_file_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	::ssize_t result = ::read(fd_, static_cast<void*>(buff), bytes);
	if(result < 0)
		ec.assign(errno, std::system_category() );
	return static_cast<size_t>(result);
}

std::size_t synch_file_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	::ssize_t result = ::write(fd_, buff, size);
	if(result < 0)
		ec.assign(errno, std::system_category() );
	return static_cast<size_t>(result);
}

inline std::size_t synch_file_channel::seek(std::error_code& ec,file_offset_t offset, int whence) noexcept
{
	::file_offset_t res = ::lseek_syscall(fd_, offset, whence);
	if( -1 == res)
		ec.assign( errno, std::system_category() );
	return static_cast<std::size_t>(res);
}

std::size_t synch_file_channel::forward(std::error_code& ec,std::size_t offset) noexcept
{
	return seek(ec, static_cast<file_offset_t>(offset), SEEK_CUR);
}

std::size_t synch_file_channel::backward(std::error_code& ec, std::size_t offset) noexcept
{
	return seek( ec, -( static_cast<file_offset_t>(offset) ), SEEK_CUR);
}

std::size_t synch_file_channel::from_begin(std::error_code& ec, std::size_t offset) noexcept
{
	return seek( ec, static_cast<file_offset_t>(offset), SEEK_SET );
}

std::size_t synch_file_channel::from_end(std::error_code& ec, std::size_t offset) noexcept
{
	return seek( ec, -( static_cast<file_offset_t>(offset) ),SEEK_END);
}

std::size_t synch_file_channel::position(std::error_code& ec) noexcept
{
	return seek( ec, 0, SEEK_CUR);
}

} // namesapace posix

bool file::exist() noexcept
{
	return -1 != ::access( name_.c_str(), F_OK );
}

static constexpr int DEFAULT_FILE_PERMS = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

bool file::create() noexcept
{
	if(name_.empty() || exist() )
		return false;
    int fd = ::open( name_.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
            DEFAULT_FILE_PERMS);
    if(-1 != fd) {
        ::close(fd);
        return true;
    }
    return false;

}

file file::get(std::error_code& ec,const char* name) noexcept
{
	if(nullptr == name || '\0' == *(name) )
		return file(std::string());
	return file( name );
}

file file::get(std::error_code& ec,const wchar_t* wname) noexcept
{
	if(nullptr == wname)
		return file(std::string());
	scoped_arr<uint8_t> u8n(std::char_traits<wchar_t> ::length(wname)*sizeof(char));
	transcode(ec, reinterpret_cast<const char32_t*>(wname), u8n.len(),  u8n.get(), u8n.len());
	if(ec)
		return file(std::string());
	return file( std::string(reinterpret_cast<const char*>(u8n.get()) )  );
}

s_read_channel file::open_for_read(std::error_code& ec) noexcept
{
	if(name_.empty()) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_read_channel();
	}
	int fd = ::open( name_.c_str(), O_RDONLY, O_SYNC);
	if(-1 == fd) {
		ec.assign( errno, std::system_category() );
		return s_read_channel();
	}
	return s_read_channel( posix::new_sync_file_channel( ec, fd ) );
}

s_write_channel file::open_for_write(std::error_code& ec,write_open_mode mode) noexcept
{
	if(name_.empty()) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_write_channel();
	}
	int fd = -1;
	switch(mode) {
	case  write_open_mode::append:
		fd = ::open( name_.c_str(), O_WRONLY | O_APPEND | O_SYNC);
		break;
	case  write_open_mode::create_if_not_exist:
	case  write_open_mode::overwrite:
        if(!exist())
            fd = ::open( name_.c_str(), (O_WRONLY | O_CREAT | O_TRUNC | O_SYNC), DEFAULT_FILE_PERMS);
        else
            fd = ::open( name_.c_str(), (O_WRONLY | O_TRUNC | O_SYNC) );
		break;
	}
	if(-1 == fd) {
		ec.assign( errno, std::system_category() );
		return s_write_channel();
	}
	return s_write_channel( posix::new_sync_file_channel( ec, fd ) );
}

s_random_access_channel file::open_for_random_access(std::error_code& ec,write_open_mode mode) noexcept
{
	if(name_.empty()) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_random_access_channel();
	}
	int fd = ::open( name_.c_str(), O_RDWR, O_SYNC);
	if(-1 == fd) {
		ec.assign( errno, std::system_category() );
		return s_random_access_channel();
	}
	return s_random_access_channel( posix::new_sync_file_channel( ec, fd ) );
}


} // namespace io
