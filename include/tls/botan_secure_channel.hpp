#ifndef __IO_BOTAN_SECURE_CHANNEL_HPP_ICLUDED__
#define __IO_BOTAN_SECURE_CHANNEL_HPP_ICLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <channels.hpp>
#include <network.hpp>
#include <threading.hpp>

#include <botan/tls_client.h>
#include <botan/tls_callbacks.h>
#include <botan/tls_session_manager.h>
#include <botan/tls_policy.h>
#include <botan/auto_rng.h>
#include <botan/certstor.h>

namespace io {

namespace net {

namespace secure {


class IO_PUBLIC_SYMBOL service {
public:
    static const service* instance(std::error_code& ec) noexcept;
    ~service() noexcept;
    s_read_write_channel new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept;
private:
	service() noexcept
	{}
	static void destroy_botan_atexit() noexcept;
private:
    static std::atomic<service*> _instance;
    static critical_section _mtx;
};


} // namespace secure

} // namespace net

} // namespace io


#endif // __IO_BOTAN_SECURE_CHANNEL_HPP_ICLUDED__
