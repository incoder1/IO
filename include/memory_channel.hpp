#ifndef __IO_memory_read_write_channel_HPP_INCLUDED__
#define __IO_memory_read_write_channel_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"
#include "threading.hpp"

namespace io {


class memory_read_write_channel;
DECLARE_IPTR(memory_read_write_channel);

/// Memory buffer synchronous read-write channel
class IO_PUBLIC_SYMBOL memory_read_write_channel final:public read_write_channel {
	memory_read_write_channel(const memory_read_write_channel&) = delete;
	memory_read_write_channel& operator=(const memory_read_write_channel&) = delete;
private:
	friend class nobadalloc<memory_read_write_channel>;
	memory_read_write_channel(uint8_t* const block,const std::size_t capacity) noexcept;
public:

	/// Allocates memory buffer and constructs new memory channel
	/// \param ec operation error code
	/// \param buffer_size memory buffer size
	/// \return a smart reference on new memory channel
	static s_memory_read_write_channel new_channel(std::error_code& ec, std::size_t buff_size) noexcept;

	/// Releases memory block
	virtual ~memory_read_write_channel() noexcept override;

	/// Returns this channel to the initial state
	void clear() noexcept;

	/// Check this channel memory buffer is empty
	/// \return whether this channel is empty
	bool empty() const noexcept;

	/// Check this channel memory buffer is full
	/// \return whether this channel is full
	bool full() const noexcept;

	//! @copydoc read_channel::read(std::error_code,uint8_t*,std::size_t)
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;

	//! @copydoc write_channel::write(std::error_code,const uint8_t*,std::size_t)
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;

private:
	uint8_t *block_;
	const uint8_t* end_;
	mutable uint8_t	*read_pos_;
	mutable uint8_t *write_pos_;
	mutable io::read_write_barrier rwb_;
};



} // namespace io

#endif // __IO_memory_read_write_channel_HPP_INCLUDED__
