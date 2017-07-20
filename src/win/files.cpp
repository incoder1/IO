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
#include "../stdafx.hpp"
#include "files.hpp"

namespace io { namespace win {

// synch_file_channel
synch_file_channel::synch_file_channel(::HANDLE hnd) noexcept:
	random_access_channel(),
	hch_(hnd)
{}

synch_file_channel::~synch_file_channel() noexcept
{
	::FlushFileBuffers( hch_.hnd() );
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


file file::get(std::error_code& ec,const char* name) noexcept {
	if(nullptr == name || '\0' == *(name) ) {
		return file(nullptr);
	}
	// this size is in bytes
	std::size_t len = std::char_traits<char>::length(name) + 1;
	char16_t *uname = static_cast<char16_t*> ( memory_traits::malloc( len * sizeof(char16_t) ) );
	if(nullptr == uname) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return file();
	}
	std::char_traits<char16_t>::assign(uname, u'\0', len);
	transcode(ec, reinterpret_cast<const uint8_t*>(name), len, uname, len);
	return file( reinterpret_cast<wchar_t*>(uname) );
}

file file::get(std::error_code& ec,const wchar_t* name) noexcept
{
	if(nullptr == name) {
		return file(nullptr);
	}
	typedef std::char_traits<wchar_t> wtr;
	std::size_t len = wtr::length(name) + 1;
	wchar_t *ncpy = static_cast<wchar_t*>( memory_traits::malloc( len * sizeof(wchar_t) ) );
	if(nullptr == ncpy) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return file();
	}
	wtr::move(ncpy, name, len);
	return file(ncpy);
}


static inline win::synch_file_channel* new_channel(std::error_code& ec, ::HANDLE hnd) noexcept {
win::synch_file_channel *result = nullptr;
#ifndef IO_NO_EXCEPTIONS
	try {
#endif // IO_NO_EXCEPTIONS
	result = new win::synch_file_channel(hnd);
#ifndef IO_NO_EXCEPTIONS
	} catch (std::exception& exc) {
#else
	if(nullptr == result) {
#endif // IO_NO_EXCEPTIONS
		ec = std::make_error_code(std::errc::not_enough_memory);
		return result;
	}
	return result;
}

static inline win::synch_file_channel* create_file_channel(std::error_code& err, ::HANDLE hnd, write_open_mode mode) {
	win::synch_file_channel *result = new_channel(err, hnd);
	if(nullptr != result) {
		if(mode == write_open_mode::append) {
			result->from_end(err, 0);
		}
	}
	return result;
}


bool file::exist() noexcept
{
	if(name_.empty())
		return false;
   ::WIN32_FIND_DATAW fdata;
   ::HANDLE handle = ::FindFirstFileW(name_.data(), &fdata);
   bool result = handle != INVALID_HANDLE_VALUE;
   if(result) {
       ::FindClose(handle);
   }
   return result;
}

bool file::create() noexcept
{
	if(name_.empty())
		return false;
	::HANDLE hnd = ::CreateFileW(
						name_.c_str(),
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

s_read_channel file::open_for_read(std::error_code& ec) noexcept {
	if(name_.empty()) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_read_channel();
	}
	::HANDLE hnd = ::CreateFileW(
						name_.c_str(),
						GENERIC_READ, 0,
						nullptr,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hnd) {
		ec.assign( ::GetLastError() , std::system_category() );
		return s_read_channel(nullptr);
	}
	win::synch_file_channel* ch = new_channel(ec,hnd);
	return nullptr != ch ? s_read_channel( ch ) : s_read_channel();
}

s_write_channel file::open_for_write(std::error_code& ec,write_open_mode mode) noexcept
{
	if(name_.empty()) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_write_channel();
	}
	::HANDLE hnd = ::CreateFileW(
						name_.c_str(),
						GENERIC_WRITE, 0,
						nullptr,
						static_cast<::DWORD>(mode),
						FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hnd) {
		ec.assign( ::GetLastError() , std::system_category() );
		return s_write_channel();
	}
	win::synch_file_channel* ch = create_file_channel(ec, hnd, mode);
	return nullptr != ch ? s_write_channel( ch ) : s_write_channel();
}

s_random_access_channel file::open_for_random_access(std::error_code& ec,write_open_mode mode) noexcept
{
	if(name_.empty()) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_random_access_channel();
	}
	::HANDLE hnd = ::CreateFileW(
						name_.c_str(),
						GENERIC_READ | GENERIC_WRITE, 0,
						nullptr,
						static_cast<::DWORD>(mode),
						FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hnd) {
		ec.assign( ::GetLastError() , std::system_category() );
		return s_random_access_channel();
	}
	win::synch_file_channel* ch = create_file_channel(ec, hnd, mode);
	return nullptr != ch ? s_random_access_channel( ch ) : s_random_access_channel();
}

} // namespace io
