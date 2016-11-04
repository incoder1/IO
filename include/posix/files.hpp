#ifndef __IO_POSIX_FILES_HPP_INCLUDED__
#define __IO_POSIX_FILES_HPP_INCLUDED__

#include <config.hpp>
#include <text.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace posix {

typedef int fd_t;

class IO_PUBLIC_SYMBOL synch_file_channel final:public random_access_channel
{
public:
	constexpr synch_file_channel(fd_t fd) noexcept;

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
	/// \param name a USC-4 encoded file path
	/// \return a file object
	/// \throw never throws
	static file get(std::error_code& ec,const wchar_t* name) noexcept;

	file(file&& oth) noexcept:
		name_(oth.name_)
	{
		if(nullptr != oth.name_) {
			oth.name_ = nullptr;
		}
	}

	file& operator=(file&& rhs)
	{
		file tmp( std::forward<file>(rhs) );
		std::swap(name_, tmp.name_);
		return *this;
	}

	~file() noexcept;

	/// Returns true when file with this path exist
	/// \return whether file exist
	bool exist() noexcept;

	/// Creates new file if it doesn't exist
	/// \return whether file was created
	bool create() noexcept;

	/// Returns UCS-4 encoded file path
	inline const wchar_t* name() noexcept {
		return nullptr != name_ ? name_ : L"";
	}

	inline std::string name(std::error_code& ec) {
		typedef std::char_traits<wchar_t> wcht;
		const std::size_t len = wcht::length(name_);
		std::string res( (len*sizeof(char32_t))+1, '\0');
		res.resize( transcode(ec, reinterpret_cast<char32_t*>(name_), len,
					  reinterpret_cast<uint8_t*>(&res[0]), (len*sizeof(char32_t)) ) + 1 );
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
	const wchar_t* name_;
};

} // namespace io


#endif // __IO_POSIX_FILES_HPP_INCLUDED__
