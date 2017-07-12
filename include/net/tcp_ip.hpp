#ifndef __IO_TCPIP_HPP_INCLUDED__
#define __IO_TCPIP_HPP_INCLUDED__

#include "../config.hpp"
#include "../channels.hpp"

namespace io {

typedef struct __network_address_struct
{
    const char* hostname;
    uint16_t port;
} network_address;

class IO_PUBLIC_SYMBOL network_factory:public object
{
	network_factory(const network_factory&) = delete;
	network_factory& operator=(const network_factory&) = delete;
protected:
	constexpr network_factory() noexcept:
		object()
	{}
public:
    virtual s_read_write_channel tcp_sync_channel(std::error_code& ec, const network_address address) const noexcept = 0;
};

DECLARE_IPTR(network_factory);

const network_factory* IO_PUBLIC_SYMBOL get_network_factory(std::error_code& ec) noexcept;

} // namesapce io


#endif // __IO_TCPIP_HPP_INCLUDED__
