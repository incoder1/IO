#ifndef __IO_TLS_ASYNCH_CHANNEL_HPP_INCLUDED__
#define __IO_TLS_ASYNCH_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"
#include "session.hpp"


namespace io {

namespace net {

namespace tls {

class asynch_channel: public io::asynch_channel
{
	public:
		asynch_channel(const s_asynch_completion_routine& routines, const asynch_io_context* context) noexcept;
		virtual ~asynch_channel();
	protected:
};



} // namespace io

} // namespace net

} // namespace tls


#endif // __IO_TLS_ASYNCH_CHANNEL_HPP_INCLUDED__
