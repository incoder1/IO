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
#include "strings.hpp"
#include "net/uri.hpp"

#include <cstdlib>
#include <limits>

namespace io {

namespace net {

static inline bool is_scheme_character(const char c) noexcept
{
	return is_alnum( c ) || is_one_of(c, '+', '-', '.');
}

static inline bool is_unreserved(const char c) noexcept
{
	return is_alnum(c) || is_one_of(c,'-','.','_','~');
}

static inline bool is_gen_delim(const char c) noexcept
{
	return is_one_of(c,":/?#[]@",7);
}

static bool is_sub_delim(const char c) noexcept
{
	return  is_one_of(c, "!$&\'()*+,;=", 12);
}

static inline bool is_reserved(const char c) noexcept
{
	return is_one_of(c,":/?#[]@;!$&\'()*+,;=",20);
}

static inline bool is_user_info_character(const char c) noexcept
{
	return is_unreserved(c) || is_one_of(c,'%',':') || is_sub_delim(c);
}

static inline bool is_authorety_character(const char c) noexcept
{
	return is_unreserved(c) || is_one_of(c, "!$&\'()*+,;=%@:[]", 17);
}

static inline bool is_path_character(char c) noexcept
{
	return is_unreserved(c) || is_one_of(c,"!$&\'()*+,;=%/:@",16);
}

static inline bool is_query_character(const char c) noexcept
{
	return cheq(c,'?') || is_path_character(c);
}

// this is international, they have the same set of legal characters
static inline bool is_fragment_character(const char c) noexcept
{
	return is_query_character(c);
}

static s_uri return_error(std::error_code& ec, std::errc code)
{
	ec = std::make_error_code(code);
	return s_uri();
}

static __forceinline bool is_uppercase_a(const char ch) noexcept
{
#ifdef io_isupper
	return io_isupper(ch);
#else
	return ( ch <= 'Z'  ) && ( ch >= 'A' );
#endif
}

static __forceinline char to_lower_a(const char ch) noexcept
{
#ifdef io_tolower
	return io_tolower(ch);
#else
	return is_uppercase_a(ch) ? 'z' - ('Z' - ch) : ch;
#endif // io_tolower
}

static const char* str_to_lower_a(char* const dst, const char* src) noexcept
{
	const char *c = src;
	char *d = dst;
	while( '\0' != *c) {
		*d = to_lower_a( *c );
		++c;
		++d;
	}
	*d = '\0';
	return dst;
}

static __forceinline  bool streq(const char* lsh, const char* rhs) noexcept
{
	return 0 == io_strcmp(lsh, rhs);
}

// uri
uint16_t IO_NO_INLINE uri::default_port_for_scheme(const char* scheme) noexcept
{
	// noexcept == no unordered_map
	char sch[8];
	str_to_lower_a(sch, scheme);
	if( streq("echo", sch) )
		return 7;
	else if( streq("daytime", sch) )
		return 13;
	else if( streq("ftp", sch) )
		return 21;
	else if( streq("ssh", sch) )
		return 22;
	else if( streq("telnet", sch) )
		return 23;
	else if( streq("mailto", sch) )
		return 25;
	else if(streq("time", sch) )
		return 37;
	else if(streq("name", sch) )
		return 42;
	else if(streq("domain", sch) )
		return 53;
	else if(streq("gopher", sch) )
		return 70;
	else if(streq("https", sch) )
		return 443;
	else if( streq("http", sch) )
		return 80;
	else if( streq("npp", sch) )
		return 92;
	else if( streq("sftp", sch) )
		return 115;
	else if( streq("irc", sch) )
		return 6697;
	else
		return 0;
}

/// std::regexp can throw an exception, and no PCRE
/// so parse it manually
s_uri uri::parse(std::error_code& ec, const char* str) noexcept
{
	char *normalized;
	const std::size_t len = io_strlen(str) + 1;
	if(len <= UCHAR_MAX) {
		normalized = static_cast<char*>( io_alloca( len ) );
	} else {
		normalized = memory_traits::calloc_temporary<char>(len);
		if(nullptr == normalized) {
			ec = std::make_error_code( std::errc::not_enough_memory );
			return s_uri();
		}
	}
	str_to_lower_a(normalized, str);

	const_string scheme;
	const_string host;
	uint16_t port = 0;
	const_string user_info;
	const_string path;
	const_string query;
	const_string fragment;
	const char *b = normalized;

	const char *e = io_strpbrk(b,":/");
	if(nullptr == e)
		return return_error(ec, std::errc::invalid_argument);

	// not relative URI need to extract scheme
	if ( cheq(*e,':') ) {
		const char* j = b;
		while( j < e ) {
			if( ! is_scheme_character(*j) )
				return return_error(ec, std::errc::invalid_argument);
			++j;
		}
		scheme = const_string(b,j);
		if(scheme.empty())
			return return_error(ec, std::errc::not_enough_memory);
		e = j + 1;
		if( !cheq(*e, '/') )
			return return_error(ec, std::errc::invalid_argument);
		b = e;
	}
	// parse the authorety portion
	if( 0 == io_memcmp(b, "//", 2) ) {
		b += 2;
		for( e = b; !is_one_of(*e,'/','?','#','\0'); e++) {
			if( ! is_authorety_character(*e) )
				return return_error(ec, std::errc::invalid_argument);
		}
		const char* host_strt = b;
		const char* host_end = e;
		if(host_end != host_strt) {
			// digits only
			const char *j = host_end-1;
			for( ; io_isdigit(*j) && j > b; j--);
			// has port
			if( cheq( ':', *j ) ) {
				host_end = j;
				port = static_cast<uint16_t>( std::strtoul(j+1,const_cast<char**>(&e),10) );
			}
		}
		// look for a user_info component
		const char *j = host_strt;
		while( j != host_end && is_user_info_character(*j))
			++j;

		if( cheq(*j,'@') ) {
			host_strt = j + 1;
			user_info = const_string(b, j );
			if( user_info.empty() )
				return return_error(ec, std::errc::not_enough_memory);
		}
		host = const_string(host_strt,host_end);
		if(host.empty())
			return return_error(ec, std::errc::not_enough_memory);
		b = e;
	}
	// got path
	if ( cheq(*b,'/') || is_path_character(*b) ) {
		e = b;
		// ? # or '\0'
		while( !is_one_of(*e, '?','#','\0') ) {
			if (!is_path_character(*e))
				return return_error(ec, std::errc::invalid_argument );
			++e;
		}
		if(b != e)
			path = const_string( b, e );
		else
			path = const_string("/");
		if(path.empty())
			return return_error(ec, std::errc::not_enough_memory);
		b = e;
	}
	// got query
	if ( cheq(*b, '?') ) {
		e = ++b;
		// '#' or '\0'
		while( !is_one_of(*e,'#','\0') ) {
			if (!is_query_character(*e))
				return return_error(ec, std::errc::invalid_argument );
			++e;
		}
		query = const_string(b, e);
		if(query.empty())
			return return_error(ec, std::errc::not_enough_memory);
		b = e;
	}
	// got fragment
	if ( cheq(*b,'#') ) {
		e = ++b;
		while( !cheq(*e, '\0') ) {
			if( !is_fragment_character(*e) )
				return return_error(ec, std::errc::invalid_argument );
			++e;
		}
		fragment = const_string(b, e);
		if( fragment.empty() )
			return return_error(ec, std::errc::not_enough_memory);
	}
	if(0 == port)
		port = default_port_for_scheme( scheme.data() );
	if(path.empty())
		path = const_string("/");

	if(len > UCHAR_MAX)
		memory_traits::free_temporary(normalized);

	uri *ret = nobadalloc<uri>::construct(ec,
										  std::move(scheme),
										  port,
										  std::move(host),
										  std::move(user_info),
										  std::move(path),
										  std::move(query),
										  std::move(fragment)
										 );
	return nullptr != ret ? s_uri(ret) : s_uri();
}

uri::uri(
	const_string&& scheme,
	uint16_t port,
	const_string&& host,
	const_string&& user_info,
	const_string&& path,
	const_string&& query,
	const_string&& fragment) noexcept:
	port_(port),
	scheme_(scheme),
	host_( std::forward<const_string>(host) ),
	user_info_( std::forward<const_string>(user_info) ),
	path_( std::forward<const_string>(path) ),
	query_( std::forward<const_string>(query) ),
	fragment_( std::forward<const_string>(fragment) )
{}


std::size_t uri::hash() const noexcept
{
	std::size_t ret = 0;
	hash_combine( ret, scheme_.hash() );
	hash_combine( ret, host_.hash() );
	hash_combine( ret, user_info_.hash() );
	hash_combine( ret, path_.hash() );
	hash_combine( ret, query_.hash() );
	hash_combine( ret, fragment_.hash() );
	hash_combine( ret, port_ );
	return ret;
}


}  // namespace net

} // namespace io
