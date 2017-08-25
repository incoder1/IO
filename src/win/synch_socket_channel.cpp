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
#include "stdafx.hpp"
#include "synch_socket_channel.hpp"


// Local defs for MinGW where fwpmk.h is missing

#ifndef FWPM_CONDITION_IP_LOCAL_ADDRESS
// d9ee00de-c1ef-4617-bfe3-ffd8f5a08957
DEFINE_GUID(
    FWPM_CONDITION_IP_LOCAL_ADDRESS,
    0xd9ee00de,
    0xc1ef,
    0x4617,
    0xbf, 0xe3, 0xff, 0xd8, 0xf5, 0xa0, 0x89, 0x57
);
#endif

#ifndef FWPM_CONDITION_IP_REMOTE_ADDRESS
// b235ae9a-1d64-49b8-a44c-5ff3d9095045
DEFINE_GUID(
    FWPM_CONDITION_IP_REMOTE_ADDRESS,
    0xb235ae9a,
    0x1d64,
    0x49b8,
    0xa4, 0x4c, 0x5f, 0xf3, 0xd9, 0x09, 0x50, 0x45
);
#endif // FWPM_CONDITION_IP_REMOTE_ADDRESS

#ifndef FWPM_LAYER_OUTBOUND_TRANSPORT_V4
// 09e61aea-d214-46e2-9b21-b26b0b2f28c8
DEFINE_GUID(
    FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
    0x09e61aea,
    0xd214,
    0x46e2,
    0x9b, 0x21, 0xb2, 0x6b, 0x0b, 0x2f, 0x28, 0xc8
);
#endif // FWPM_LAYER_OUTBOUND_TRANSPORT_V4

#ifndef FWPM_LAYER_OUTBOUND_TRANSPORT_V6
// e1735bde-013f-4655-b351-a49e15762df0
DEFINE_GUID(
    FWPM_LAYER_OUTBOUND_TRANSPORT_V6,
    0xe1735bde,
    0x013f,
    0x4655,
    0xb3, 0x51, 0xa4, 0x9e, 0x15, 0x76, 0x2d, 0xf0
);
#endif // FWPM_LAYER_OUTBOUND_TRANSPORT_V6

#ifndef FWPM_CONDITION_IP_LOCAL_PORT
// 0c1ba1af-5765-453f-af22-a8f791ac775b
DEFINE_GUID(
    FWPM_CONDITION_IP_LOCAL_PORT,
    0x0c1ba1af,
    0x5765,
    0x453f,
    0xaf, 0x22, 0xa8, 0xf7, 0x91, 0xac, 0x77, 0x5b
);
#endif // FWPM_CONDITION_IP_LOCAL_PORT

#ifndef FWPM_CONDITION_IP_REMOTE_PORT
// c35a604d-d22b-4e1a-91b4-68f674ee674b
DEFINE_GUID(
    FWPM_CONDITION_IP_REMOTE_PORT,
    0xc35a604d,
    0xd22b,
    0x4e1a,
    0x91, 0xb4, 0x68, 0xf6, 0x74, 0xee, 0x67, 0x4b
);
#endif // FWPM_CONDITION_IP_REMOTE_PORT

#ifndef FWPM_CONDITION_IP_PROTOCOL
// 3971ef2b-623e-4f9a-8cb1-6e79b806b9a7
DEFINE_GUID(
    FWPM_CONDITION_IP_PROTOCOL,
    0x3971ef2b,
    0x623e,
    0x4f9a,
    0x8c, 0xb1, 0x6e, 0x79, 0xb8, 0x06, 0xb9, 0xa7
);
#endif // FWPM_CONDITION_IP_PROTOCOL

#ifndef FWPM_CONDITION_IP_LOCAL_ADDRESS_TYPE
// 6ec7f6c4-376b-45d7-9e9c-d337cedcd237
DEFINE_GUID(
    FWPM_CONDITION_IP_LOCAL_ADDRESS_TYPE,
    0x6ec7f6c4,
    0x376b,
    0x45d7,
    0x9e, 0x9c, 0xd3, 0x37, 0xce, 0xdc, 0xd2, 0x37
);
#endif // FWPM_CONDITION_IP_LOCAL_ADDRESS_TYPE

#ifndef FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V4
// 4b46bf0a-4523-4e57-aa38-a87987c910d9
DEFINE_GUID(
    FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V4,
    0x4b46bf0a,
    0x4523,
    0x4e57,
    0xaa, 0x38, 0xa8, 0x79, 0x87, 0xc9, 0x10, 0xd9
);
#endif // FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V4

#ifndef FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V4
// 70a4196c-835b-4fb0-98e8-075f4d977d46
DEFINE_GUID(
    FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V4,
    0x70a4196c,
    0x835b,
    0x4fb0,
    0x98, 0xe8, 0x07, 0x5f, 0x4d, 0x97, 0x7d, 0x46
);
#endif // FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V4

#ifndef FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V6
// 38d87722-ad83-4f11-a91f-df0fb077225b
DEFINE_GUID(
    FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V6,
    0x38d87722,
    0xad83,
    0x4f11,
    0xa9, 0x1f, 0xdf, 0x0f, 0xb0, 0x77, 0x22, 0x5b
);
#endif // FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V6

#ifndef FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V6
// f1835363-a6a5-4e62-b180-23db789d8da6
DEFINE_GUID(
    FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V6,
    0xf1835363,
    0xa6a5,
    0x4e62,
    0xb1, 0x80, 0x23, 0xdb, 0x78, 0x9d, 0x8d, 0xa6
);
#endif // FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V6

namespace io {

namespace net {

synch_socket_channel::synch_socket_channel(::SOCKET socket) noexcept:
	read_write_channel(),
	socket_(socket)
{}

synch_socket_channel::~synch_socket_channel() noexcept
{
	::closesocket(socket_);
}

std::size_t synch_socket_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	::WSABUF wsab;
	wsab.len = static_cast<::u_long>(bytes);
	wsab.buf = const_cast<char*>(reinterpret_cast<const char*>(buff));
	::DWORD ret, flags = 0;
	if(SOCKET_ERROR == ::WSARecv(socket_, &wsab, 1, &ret, &flags, nullptr, nullptr) ) {
		ec = std::make_error_code( win::wsa_last_error_to_errc() );
		return 0;
	}
	return static_cast<::std::size_t>(ret);
}

std::size_t synch_socket_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	::WSABUF wsab;
	wsab.len = static_cast<::u_long>(size);
	wsab.buf = const_cast<char*>(reinterpret_cast<const char*>(buff));
	::DWORD ret;
	if(SOCKET_ERROR == ::WSASend(socket_, &wsab, 1, &ret, 0, nullptr, nullptr) ) {
		ec = std::make_error_code( win::wsa_last_error_to_errc() );
		return 0;
	}
	return static_cast<::std::size_t>(ret);
}


} // namespace net

} // namespace io

