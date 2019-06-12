#include "stdafx.hpp"
#include "conststring.hpp"

namespace io {


const_string::const_string(const char* str, std::size_t length) noexcept:
	data_( {false,0,nullptr} )
{
	assert(nullptr != str &&  length > 0  && length < SIZE_MAX );
	std::size_t size = length;
	if( '\0' != str[length-1] )
		++size;
	if(size < detail::SSO_MAX) {
		// Optimize short string, i.e. str size is less then sizeof(char*)+sizeof(std::size_t)
		data_.short_buf.sso = true;
		data_.short_buf.size = length;
		traits_type::assign(data_.short_buf.char_buf, '\0', detail::SSO_MAX);
		traits_type::copy(data_.short_buf.char_buf, str, length);
	}
	else {
		// allocate string + reference counter
		uint8_t* px = memory_traits::malloc_array<uint8_t>( size + sizeof(std::size_t) );
		if( nullptr != px ) {
			// set initial intrusive atomic reference count
			std::size_t *rc = reinterpret_cast<std::size_t*>(px);
			*rc = 1;
			// copy string data
			char *b = reinterpret_cast<char*>( px + sizeof(std::size_t) );
			traits_type::copy(b, str, length);
			data_.long_buf.size = length;
			data_.long_buf.char_buf = px;
		}
	}
}


} // namespace io
