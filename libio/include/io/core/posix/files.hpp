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
#ifndef __IO_POSIX_FILES_HPP_INCLUDED__
#define __IO_POSIX_FILES_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <cerrno>
#include <string>

#include <io/textapi/charset_converter.hpp>

#ifdef __LP64__

extern "C" __off64_t lseek64 (int __fd, __off64_t __offset, int __whence) __THROW;

typedef ::__off64_t file_offset_t;

static inline file_offset_t lseek_syscall(int __fd, __off64_t __offset, int __whence) noexcept {
	return ::lseek64(__fd, __offset, __whence );
}

#else

extern "C" off_t lseek(int fd, __off_t offset, int whence);
typedef ::__off_t file_offset_t;

static inline file_offset_t lseek_syscall(int __fd, __off_t __offset, int __whence) noexcept {
	return ::lseek(__fd, __offset, __whence );
}

#endif // __LP64__

namespace io {

namespace posix {

typedef int fd_t;

class IO_PUBLIC_SYMBOL synch_file_channel final:public random_access_channel
{
public:
	synch_file_channel(os_descriptor_t fd) noexcept;

	virtual ~synch_file_channel() noexcept override;

	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;

	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;

	virtual std::size_t forward(std::error_code& ec,std::size_t size) noexcept override;

	virtual std::size_t backward(std::error_code& ec, std::size_t size) noexcept override;

	virtual std::size_t from_begin(std::error_code& ec, std::size_t size) noexcept override;

	virtual std::size_t from_end(std::error_code& ec, std::size_t size) noexcept override;

	virtual std::size_t position(std::error_code& ec) noexcept override;
private:
	inline std::size_t seek(std::error_code& ec,int64_t offset, int whence) noexcept;
private:
	os_descriptor_t fd_;
};

} // namespace posix

/// \brief file writing open modes
enum class write_open_mode
{
	/// Open an existing file and move file pointer to the end of it
	append,
	/// Creates new file if and only if it is not exist
	create_if_not_exist,
	/// Creates new file, override it if it exist
	overwrite
};


/// \brief File system file operations interface, POSIX implementation
class IO_PUBLIC_SYMBOL file
{
public:

	static constexpr char separator() noexcept {
		return '/';
	}

	explicit file(const std::string& name);

	explicit file(const std::wstring& name);

	file(const file& c):
		 name_( c.name_ )
	{}

	file& operator=(const file& rhs)
	{
		file( rhs ).swap( *this );
		return *this;
	}

    file(file&& oth) noexcept:
		name_( std::move(oth.name_) )
	{}

	file& operator=(file&& rhs) noexcept
	{
		file( std::forward<file>(rhs) ).swap( *this );
		return *this;
	}

	~file() noexcept = default;

	inline void swap(file& oth) noexcept {
		name_.swap( oth.name_ );
	}

	/// Returns true when file with this path exist
	/// \return whether file exist
	bool exist() const noexcept;

	/// Creates new file if it doesn't exist
	/// \return whether file was created
	bool create() noexcept;

	/// Returns file size in byte
	/// \return file size in bytes, 0 if file not exist
	std::size_t size() const noexcept;

	/// Returns UCS-4 encoded file path
	inline std::wstring wpath() const {
		std::string p = path();
		return transcode_to_ucs( p.data(), p.length() );
	}

	/// Returns UTF-8 encoded file path
	std::string path() const;

	/// Opens blocking read channel from this file
	/// \param ec
	///    operation error code, contains error when file is not exist or can not be opened
	///    or out of memory state
	/// \throw never throws
	s_read_channel open_for_read(std::error_code& ec) const noexcept;

	/// Opens blocking write channel from this file
	/// \param ec
	///    operation error code, contains error when file can not be opened
	///    or out of memory state
	/// \param mode
	///    writing mode \see write_open_mode
	/// \throw never throws
	s_write_channel open_for_write(std::error_code& ec, write_open_mode mode) const noexcept;

	/// Opens blocking read/write and random access channel from this file
	/// \param ec
	///    operation error code, contains error when can not be opened
	///    or out of memory state
	/// \param mode
	///    writing mode \see write_open_mode
	/// \throw never throws
	s_random_access_channel open_for_random_access(std::error_code& ec, write_open_mode mode) const noexcept;
private:
	std::string name_;
};

} // namespace io


#endif // __IO_POSIX_FILES_HPP_INCLUDED__
