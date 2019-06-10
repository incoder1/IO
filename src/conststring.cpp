#include "stdafx.hpp"
#include "conststring.hpp"

namespace io {

void const_string::intrusive_add_ref(detail::sso_variant_t& var) noexcept
{
	std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(var.long_buf.char_buf);
	detail::atomic_traits::inc(p);
}

std::size_t const_string::intrusive_release(detail::sso_variant_t& var) noexcept
{
	std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(var.long_buf.char_buf);
	return detail::atomic_traits::dec(p);
}


const_string::const_string(const char* str, std::size_t length) noexcept:
	data_( {false,0,nullptr} )
{
	assert(nullptr != str &&  length > 0  && length < SIZE_MAX );
	const std::size_t size = ( '\0' != str[length] ) ? length + 1 : length;
	if(size < detail::SSO_MAX) {
		// Optimize short string, i.e. str size is less then sizeof(char*)+sizeof(std::size_t)
		detail::init_short(data_, length);
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


const_string::~const_string() noexcept
{
	// check for sso, end empty string
	// decrement atomic intrusive reference counter
	// release long buffer if needed
	if( !empty() && !sso() && 0 == intrusive_release(data_) )
		memory_traits::free(data_.long_buf.char_buf);
}


const char* const_string::data() const noexcept
{
	static constexpr const char* EMPTY = "";
	if ( empty() )
		return EMPTY;
	else if( sso() )
		return data_.short_buf.char_buf;
	else
		return reinterpret_cast<char*>( data_.long_buf.char_buf + sizeof(std::size_t) );
}

static bool carr_empty(const char* rhs, std::size_t len) noexcept {
	return 0 == len || nullptr == rhs || '\0' == *rhs;
}

bool const_string::equal(const char* rhs, std::size_t len) const noexcept
{
	if( io_likely( !empty() && !carr_empty(rhs, len) ) ) {
		if( size() == len )
			return 0 == traits_type::compare( data(), rhs, len );
		else if( empty() && carr_empty(rhs, len) )
			return true;
	}
	return false;
}

int const_string::compare(const const_string& rhs) const noexcept
{
	if( !sso() && !rhs.sso() && data_.long_buf.char_buf == rhs.data_.long_buf.char_buf)
		return 0;
	else if( io_likely( size() == rhs.size() ) )
		if( empty() && rhs.empty() )
			return 0;
		else
			return traits_type::compare( data(), rhs.data(), size() );
	else if( size() < rhs.size() )
		return -1;
	return 1;
}


} // namespace io
