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
#include "files.hpp"

#include <cwctype>

namespace io {

namespace win {

// synch_file_channel
synch_file_channel::synch_file_channel(::HANDLE hnd) noexcept:
	random_access_channel(),
	hch_(hnd)
{}

synch_file_channel::~synch_file_channel() noexcept
{
	::FlushFileBuffers( hch_ );
}

std::size_t synch_file_channel::read(std::error_code& err,uint8_t* const buff, std::size_t bytes) const noexcept
{
	return hch_.read(err, buff, bytes);
}

std::size_t synch_file_channel::write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
{
	return hch_.write(err, buff, size);
}

std::size_t synch_file_channel::forward(std::error_code& err,std::size_t size) noexcept
{
	return hch_.seek(err, detail::whence_type::current, static_cast<int64_t>(size) );
}

std::size_t synch_file_channel::backward(std::error_code& err, std::size_t size) noexcept
{
	return hch_.seek(err, detail::whence_type::current, -( static_cast<int64_t>(size) ) );
}

std::size_t synch_file_channel::from_begin(std::error_code& err, std::size_t size) noexcept
{
	return hch_.seek(err, detail::whence_type::begin, static_cast<int64_t>(size) );
}

std::size_t synch_file_channel::from_end(std::error_code& err, std::size_t size) noexcept
{
	return hch_.seek(err, detail::whence_type::end, -(static_cast<int64_t>(size)) );
}

std::size_t synch_file_channel::position(std::error_code& err) noexcept
{
	return hch_.seek(err, detail::whence_type::current, 0);
}


} // namespace win


// file

void file::posix_to_windows(wchar_t* path) noexcept
{
    for(wchar_t *c = path; L'\0' != *c; ++c) {
        if(L'/' == *c)
        	*c = L'\\';
    }
    // full name from root, i.e. /c/Program Files/foo
    if('\\' == path[0] ) {
    	// set drive latter, if exist
		wchar_t drive = std::towupper( path[1] );
		if( drive >= L'A' && drive <= L'Z' ) {
			path[0] = drive;
			path[1] = L':';
		}
    }
}

file::file(const char* name) noexcept:
	name_()
{
	typedef std::char_traits<char> c8tr;
	assert(nullptr != name);
	const int alen = c8tr::length(name);
	assert( alen <= MAX_PATH );
	if(alen != 0) {
		int wlen = ::MultiByteToWideChar(CP_UTF8, 0, name, alen, nullptr, 0);
		if(0 != wlen) {
			scoped_arr<wchar_t> wname( static_cast<std::size_t>(wlen+1) );
			// check for out of memory
			if( io_unlikely( wname ) ) {
				::MultiByteToWideChar(CP_UTF8, 0, name, alen, wname.get(), wlen);
				posix_to_windows( wname.get() );
				name_ = std::move(wname);
			}
		}
	}
}

file::file(const wchar_t* name) noexcept:
	name_( std::char_traits<wchar_t>::length(name) + 1 )
{
	if(name_)
		std::char_traits<wchar_t>::copy(name_.get(), name, name_.len()-1 );
	posix_to_windows( name_.get() );
}


static inline win::synch_file_channel* new_channel(std::error_code& ec, ::HANDLE hnd) noexcept
{
	return nobadalloc<win::synch_file_channel>::construct( ec, hnd);
}

static inline win::synch_file_channel* create_file_channel(std::error_code& err, ::HANDLE hnd, write_open_mode mode)
{
	win::synch_file_channel *result = new_channel(err, hnd);
	if(nullptr != result) {
		if(mode == write_open_mode::append) {
			result->from_end(err, 0);
		}
	}
	return result;
}


bool file::exist() const noexcept
{
	if( !name_ )
		return false;
	::WIN32_FIND_DATAW fdata;
	::HANDLE handle = ::FindFirstFileW(name_.get(), &fdata);
	bool result = handle != INVALID_HANDLE_VALUE;
	if(result) {
		::FindClose(handle);
	}
	return result;
}

std::string file::path() const
{
	int asize = ::WideCharToMultiByte(
					CP_UTF8, 0,
					name_.get(), name_.len()-1,
					nullptr, 0,
					nullptr, nullptr);
	if( io_likely( asize > 0 ) ) {
		char* ret = static_cast<char*>( io_alloca(asize) );
		io_zerro_mem(ret, asize);
		::WideCharToMultiByte(
			CP_UTF8, 0,
			name_.get(), name_.len()-1,
			ret, asize,
			nullptr, nullptr);
		return std::string(ret, ret + asize);
	}
	return std::string();
}

bool file::create()  noexcept
{
	if(!name_)
		return false;
	::HANDLE hnd = ::CreateFileW(
					   name_.get(),
					   GENERIC_READ | GENERIC_WRITE, 0,
					   nullptr,
					   CREATE_NEW,
					   FILE_ATTRIBUTE_NORMAL, 0);
	bool result = INVALID_HANDLE_VALUE != hnd;
	if(result) {
		::CloseHandle(hnd);
	}
	return result;
}

std::size_t file::size() const noexcept
{
	if( exist() ) {
		::HANDLE hnd = ::CreateFileW(
						   name_.get(),
						   GENERIC_READ, 0,
						   nullptr,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL, 0);
		::LARGE_INTEGER ret;
		::GetFileSizeEx(hnd, &ret);
		::CloseHandle(hnd);
		return static_cast<std::size_t>(ret.QuadPart);
	}
	return 0;
}

s_read_channel file::open_for_read(std::error_code& ec) const noexcept
{
	if( !name_ ) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_read_channel();
	}
	::HANDLE hnd = ::CreateFileW(
					   name_.get(),
					   GENERIC_READ, 0,
					   nullptr,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hnd) {
		ec.assign( ::GetLastError(), std::system_category() );
		return s_read_channel(nullptr);
	}
	win::synch_file_channel* ch = new_channel(ec,hnd);
	return nullptr != ch ? s_read_channel( ch ) : s_read_channel();
}

s_write_channel file::open_for_write(std::error_code& ec,write_open_mode mode) const noexcept
{
	if( !name_ ) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_write_channel();
	}
	::HANDLE hnd = ::CreateFileW(
					   name_.get(),
					   GENERIC_WRITE, 0,
					   nullptr,
					   static_cast<::DWORD>(mode),
					   FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hnd) {
		ec.assign( ::GetLastError(), std::system_category() );
		return s_write_channel();
	}
	win::synch_file_channel* ch = create_file_channel(ec, hnd, mode);
	return nullptr != ch ? s_write_channel( ch ) : s_write_channel();
}

s_random_access_channel file::open_for_random_access(std::error_code& ec,write_open_mode mode) const noexcept
{
	if(!name_) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_random_access_channel();
	}
	::HANDLE hnd = ::CreateFileW(
					   name_.get(),
					   GENERIC_READ | GENERIC_WRITE, 0,
					   nullptr,
					   static_cast<::DWORD>(mode),
					   FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hnd) {
		ec.assign( ::GetLastError(), std::system_category() );
		return s_random_access_channel();
	}
	win::synch_file_channel* ch = create_file_channel(ec, hnd, mode);
	return nullptr != ch ? s_random_access_channel( ch ) : s_random_access_channel();
}

} // namespace io
