#ifndef __IO_WIN_ASYNCH_CHANNEL_HPP_INCLUDED__
#define __IO_WIN_ASYNCH_CHANNEL_HPP_INCLUDED__

#include <config.hpp>
#include <channels.hpp>

#include "winconf.hpp"
#include "handlechannel.hpp"

namespace io {

class asynch_io_service;

namespace win {

class IO_PUBLIC_SYMBOL asynch_channel final:public asynch_read_write_channel {
public:
	asynch_channel(::HANDLE hres, const asynch_callback& rc, const asynch_callback& wc) noexcept;
	virtual ~asynch_channel() noexcept override;
	virtual void read(std::size_t bytes, std::size_t pos) const noexcept override;
	virtual void write(byte_buffer&& buff, std::size_t pos) const noexcept override;
	virtual bool cancel_pending() const noexcept override;
	virtual bool cancel_all() const noexcept override;
protected:
	friend class asynch_io_service;
public:
	detail::handle_channel hch_;
};

} // namespace win

} // namespace io

#endif // __IO_WIN_ASYNCH_CHANNEL_HPP_INCLUDED__
