#ifndef __IO_WINDOWS_FILES_HPP_INCLUDED__
#define __IO_WINDOWS_FILES_HPP_INCLUDED__

#include <config.hpp>
#include <channels.hpp>
#include <charsetcvt.hpp>

#include "handlechannel.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

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
	detail::handle_synch_channel hch_;
};

} // namespace win

/// \brief file writting open modes constans
enum class write_open_mode: ::DWORD
{
	/// Open an existing file and move file pointer to the end of it
	append = OPEN_ALWAYS,
	/// Creates new file if and only if it is not exist
	create_if_not_exist = CREATE_NEW,
	/// Creates new file, override it if it exist
	overwrite = CREATE_ALWAYS
};

/// \brief Filesystem file operations interface, windows implementation
class IO_PUBLIC_SYMBOL file
{
	file(const file&) = delete;
	file& operator=(const file&) = delete;
private:
	explicit constexpr file() noexcept:
		name_(nullptr)
	{}
	explicit constexpr file(wchar_t* name) noexcept:
		name_(name)
	{}
public:
	/// Returns file object by OS specific file path
	/// \param ec
	///		operation error code, contains error when out of memory
	/// 		or character set transcoding error
	/// \param name a UTF-8 or national code page file path
	/// \return a file object
	/// \throw never throws
	static file get(std::error_code& ec,const char* name) noexcept;
	/// Returns file object by OS specific file path
	/// \param ec
	///		operation error code, contains error when out of memory
	/// 		or character set transcoding error
	/// \param name a UTF-16LE encoded file path
	/// \return a file object
	/// \throw never throws
	static file get(std::error_code& ec,const wchar_t* name) noexcept;

	file(file&& oth) noexcept:
		name_(oth.name_)
	{
		oth.name_ = nullptr;
	}

	file& operator=(file&& rhs) noexcept {
		file( std::forward<file>(rhs) ).swap( *this );
		return *this;
	}

	inline void swap(file& oth) noexcept {
		std::swap(name_, oth.name_);
	}

	~file() noexcept;

	/// Returns true when file with this path exist
	/// \return whether file exist
	bool exist() noexcept;

	/// Creates new file if it doesn't exist
	/// \return whether file was created
	bool create() noexcept;

	/// Returns UTF-16LE encoded file path
	/// \return file path
	inline const wchar_t* name() noexcept {
		return nullptr != name_ ? name_ : L"";
	}

	/// Returns UTF-8 encoded file path
	/// \return file path
	inline std::string name(std::error_code& ec) noexcept {
		typedef std::char_traits<wchar_t> wcht;
		const std::size_t len = wcht::length(name_);
		std::string res( len, '\0');
		transcode(ec, reinterpret_cast<char16_t*>(name_), len,
					  reinterpret_cast<uint8_t*>(&res[0]), len );
		return res;
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
	wchar_t* name_;
};

} // namespce io

#endif // __IO_WINDOWS_FILES_HPP_INCLUDED__
