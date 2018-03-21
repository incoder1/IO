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
#ifndef __IO_POSIX_FILES_HPP_INCLUDED__
#define __IO_POSIX_FILES_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <cerrno>
#include <text.hpp>
#include <string>

#ifdef __LP64__

extern "C" __off64_t lseek64 (int __fd, __off64_t __offset, int __whence) __THROW;

typedef ::__off64_t file_offset_t;

static inline file_offset_t lseek_syscall(int __fd, __off64_t __offset, int __whence) {
	return ::lseek64(__fd, __offset, __whence );
}

#else

extern "C" off_t lseek(int fd, __off_t offset, int whence);
typedef ::__off_t file_offset_t;

static inline file_offset_t lseek_syscall(int __fd, __off_t __offset, int __whence) {
	return ::lseek(__fd, __offset, __whence );
}

#endif // __LP64__

namespace io {

namespace posix {

typedef int fd_t;

class IO_PUBLIC_SYMBOL synch_file_channel final:public random_access_channel
{
public:
	synch_file_channel(fd_t fd) noexcept;

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
	fd_t fd_;
};


} // namespace posix

/// \brief file writting open modes constans
enum class write_open_mode
{
	/// Open an existing file and move file pointer to the end of it
	append,
	/// Creates new file if and only if it is not exist
	create_if_not_exist,
	/// Creates new file, override it if it exist
	overwrite
};


/// \brief Filesystem file operations interface, POSIX implementation
class IO_PUBLIC_SYMBOL file
{

public:

	explicit file(const char* name) noexcept;

	explicit file(const wchar_t* name) noexcept;

    file(file&& oth) noexcept:
		name_( std::move(oth.name_) )
	{}

	file& operator=(file&& rhs)
	{
		file tmp( std::forward<file>(rhs) );
		std::swap(name_, tmp.name_);
		return *this;
	}

	~file() noexcept = default;

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
	inline std::wstring wpath()  {
        return transcode_to_ucs( name_.get(), std::strlen( name_.get() ) );
	}

	inline std::string path() {
		return std::string(name_.get());
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
	scoped_arr<char> name_;
};

} // namespace io


#endif // __IO_POSIX_FILES_HPP_INCLUDED__
