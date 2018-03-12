#ifndef __IO_memory_read_write_channel_HPP_INCLUDED__
#define __IO_memory_read_write_channel_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"
#include "threading.hpp"

namespace io {

/// \brief Memory buffer read channel
class IO_PUBLIC_SYMBOL memory_read_channel final: public read_channel
{
private:
	friend class nobadalloc<memory_read_channel>;
	memory_read_channel(byte_buffer&& data) noexcept;
public:
	/// Open a memory buffer channel
	/// \param ec operation error code
	/// \param buff memory to read from
	/// \return read channel smart reference
	static s_read_channel open(std::error_code& ec, byte_buffer&& buff) noexcept;
	virtual ~memory_read_channel() noexcept = default;
	//! @copydoc read_channel::read(std::error_code,uint8_t*,std::size_t)
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
private:
	mutable byte_buffer data_;
	mutable critical_section mtx_;
};

class memory_write_channel;
DECLARE_IPTR(memory_write_channel);

/// \brief Memory buffer write channel.
/// Buffer starts from the initial size and exponentially grow until memory available
class IO_PUBLIC_SYMBOL memory_write_channel final:public write_channel
{
private:
	friend class nobadalloc<memory_write_channel>;
	memory_write_channel(byte_buffer&& data) noexcept;
public:

	/// Opens new memory write channel
	/// \param ec operation error code
	/// \param initial_size initial buffer size
	/// \return memory buffer write channel smart reference
	static s_memory_write_channel open(std::error_code& ec, std::size_t initial_size) noexcept;


	/// Opens new memory write channel with default initial buffer size (system page size)
	/// \param ec operation error code
	/// \return memory buffer write channel smart reference
	static inline s_memory_write_channel open(std::error_code& ec) noexcept
	{
		return open( ec, memory_traits::page_size() );
	}

	virtual ~memory_write_channel() noexcept = default;

	//! @copydoc write_channel::write(std::error_code,const uint8_t*,std::size_t)
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;

	/// Returns deep copy of underlying memory buffer
	/// \param ec operation error code
	/// \return deep copy of underlying memory buffer
	byte_buffer data(std::error_code& ec) const noexcept;

private:
	mutable byte_buffer data_;
	mutable critical_section mtx_;
};

} // namespace io

#endif // __IO_memory_read_write_channel_HPP_INCLUDED__
