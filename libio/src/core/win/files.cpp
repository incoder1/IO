/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "io/core/win/files.hpp"

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

void file::posix_to_windows(std::wstring& path) noexcept
{
	std::replace( path.begin(), path.end(), L'/', L'\\');
	// full name from root, i.e. /c/Program Files/foo
	if('\\' == path[0] ) {
		// set drive latter, if exist
		wchar_t drive = std::towupper( path[1] );
		if( std::iswalpha(drive) ) {
			path[0] = drive;
			path[1] = L':';
		}
	}
}

file::file(const std::wstring& name):
	name_( name )
{
	assert( !name_.empty() );
	posix_to_windows( name_ );

	// get absolute path name if needed
	if( L':' != name_[1] ) {
		std::wstring ret;
		bool existen_file = exist();
		// open or create a file, to obtain full_path
		::HANDLE hnd = ::CreateFileW(
						name_.data(),
						existen_file ? GENERIC_READ : GENERIC_WRITE,
						existen_file ? FILE_SHARE_READ : 0,
						nullptr,
						existen_file ? OPEN_EXISTING : CREATE_NEW,
						FILE_ATTRIBUTE_NORMAL, nullptr);
		wchar_t full_path[MAX_PATH+1] = { L'\0' };
		std::size_t path_size = ::GetFinalPathNameByHandleW( hnd, full_path, MAX_PATH, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS );
		if(0 != path_size) {
			name_.clear();
			if( 0 == io_memcmp(full_path,L"\\\\?\\",8) )
				name_.append(full_path, 4, path_size-4);
			else
				name_.append(full_path, 0, path_size);
			name_.shrink_to_fit();
		}
		// Close file handle
		::CloseHandle(hnd);
		// delete the file if it wasn't exist before
		if(!existen_file)
			::DeleteFileW( full_path );
	}
}

file::file(const std::string& name):
	file( transcode_to_ucs( name.data() ) )
{
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
	if( name_.empty() )
		return false;
	::WIN32_FIND_DATAW fdata;
	::HANDLE handle = ::FindFirstFileW(name_.data(), &fdata);
	bool result = handle != INVALID_HANDLE_VALUE;
	if(result) {
		::FindClose(handle);
	}
	return result;
}

bool file::create()  noexcept
{
	if( exist() || name_.empty() )
		return false;
	::HANDLE hnd = ::CreateFileW(
						name_.data(),
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
							name_.data(),
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
	if( name_.empty() ) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_read_channel();
	}
	::HANDLE hnd = ::CreateFileW(
						name_.data(),
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
	if( name_.empty() ) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_write_channel();
	}
	::HANDLE hnd = ::CreateFileW(
						name_.data(),
						GENERIC_WRITE, FILE_SHARE_READ,
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
	if( name_.empty() ) {
		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return s_random_access_channel();
	}
	::HANDLE hnd = ::CreateFileW(
						name_.data(),
						GENERIC_READ | GENERIC_WRITE,  FILE_SHARE_READ,
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
