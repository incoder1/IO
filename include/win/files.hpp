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
#ifndef __IO_WINDOWS_FILES_HPP_INCLUDED__
#define __IO_WINDOWS_FILES_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <channels.hpp>
#include <conststring.hpp>

#include "handlechannel.hpp"

namespace io {

namespace win {

class IO_PUBLIC_SYMBOL synch_file_channel final:public random_access_channel
{
public:
	synch_file_channel(::HANDLE hnd) noexcept;

	virtual ~synch_file_channel() noexcept override;

	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;

	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;

	virtual std::size_t forward(std::error_code& ec,std::size_t size) noexcept override;

	virtual std::size_t backward(std::error_code& ec, std::size_t size) noexcept override;

	virtual std::size_t from_begin(std::error_code& ec, std::size_t size) noexcept override;

	virtual std::size_t from_end(std::error_code& ec, std::size_t size) noexcept override;

	virtual std::size_t position(std::error_code& ec) noexcept override;

private:
	detail::handle_channel hch_;
};


} // namespace win

/// \brief file writing open modes
enum class write_open_mode: ::DWORD
{
	/// Open an existing file and move file pointer to the end of it
	append = OPEN_ALWAYS,
	/// Creates new file if and only if it is not exist
	create_if_not_exist = CREATE_NEW,
	/// Creates new file, override it if it exist
	overwrite = CREATE_ALWAYS
};

/// \brief File system file operations interface, windows implementation
class IO_PUBLIC_SYMBOL file
{
	file(const file&) = delete;
	file& operator=(const file&) = delete;
	file() = delete;
public:

	explicit file(const char* name) noexcept;

	explicit file(const wchar_t* name) noexcept;

	file(file&& oth) noexcept:
		name_( std::move(oth.name_) )
	{}

	file& operator=(file&& rhs) noexcept {
		file( std::forward<file>(rhs) ).swap( *this );
		return *this;
	}

	inline void swap(file& oth) noexcept {
		name_.swap( oth.name_ );
	}

	~file() noexcept = default;

	/// Returns true when file with this path exist
	/// \return whether file exist
	bool exist() const noexcept;

	/// Creates new file if it doesn't exist
	/// \return whether file was created
	bool create() noexcept;

	/// Returns UCS-2 encoded file path
	inline std::wstring wpath() const  {
        return std::wstring( name_.get() );
	}

	/// Returns file size in byte
	/// \return file size in bytes, 0 if file not exist
    std::size_t size() const noexcept;

	/// Returns UTF-8 encoded file path
	IO_NO_INLINE std::string path() const {
		int asize = ::WideCharToMultiByte(
							CP_UTF8, 0,
							name_.get(), name_.len()-1,
							nullptr, 0,
							nullptr, nullptr);
		if(asize) {
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

	/// Opens blocking read channel from this file
	/// \param ec
	///    operation error code, contains error when file is not exist or can not be opened
	///    or out of memory state
	/// \throw never throws
	s_read_channel open_for_read(std::error_code& ec) noexcept;

	/// Opens blocking write channel from this file
	/// \param ec
	///    operation error code, contains error when file can not be opened
	///    or out of memory state
	/// \param mode
	///    writting mode \see write_open_mode
	/// \throw never throws
	s_write_channel open_for_write(std::error_code& ec, write_open_mode mode) noexcept;

	/// Opens blocking read/write and random access channel from this file
	/// \param ec
	///    operation error code, contains error when can not be opened
	///    or out of memory state
	/// \param mode
	///    writting mode \see write_open_mode
	/// \throw never throws
	s_random_access_channel open_for_random_access(std::error_code& ec, write_open_mode mode) noexcept;
private:
	scoped_arr<wchar_t> name_;
};

} // namespce io

#endif // __IO_WINDOWS_FILES_HPP_INCLUDED__
