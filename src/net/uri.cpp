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

namespace io {

namespace net {

static inline bool is_scheme_character(char c)
{
	return is_alnum( c ) || is_one_of(c, '+', '-', '.');
}

static inline bool is_unreserved(char c)
{
	return is_alnum(c) || is_one_of(c,'-','.','_','~');
}

static inline bool is_gen_delim(char c)
{
	return is_one_of(c,":/?#[]@");
}

static bool is_sub_delim(char c)
{
	return  is_one_of(c, "!$&\'()*+,;=" );
}

static inline bool is_reserved(char c)
{
	return is_one_of(c,":/?#[]@;!$&\'()*+,;=");
}

static inline bool is_user_info_character(char c)
{
	return is_unreserved(c) || is_sub_delim(c) || is_one_of(c,'%',':');
}

static inline bool is_authorety_character(char c)
{
	return is_unreserved(c) || is_one_of(c, "!$&\'()*+,;=%@:[]");
}

static inline bool is_path_character(char c)
{
	return is_unreserved(c) || is_one_of(c,"!$&\'()*+,;=%/:@");
}

static inline bool is_query_character(char c)
{
	return cheq(c,'?') || is_path_character(c);
}

// this is international, they have the same set of legal characters
static inline bool is_fragment_character(char c)
{
	return is_query_character(c);
}

static inline s_uri return_error(std::error_code& ec, std::errc code)
{
	ec = std::make_error_code(code);
	return s_uri();
}

static char to_lower_a(const char ch) noexcept
{
#ifdef io_tolower
	return io_tolower(ch);
#else
	return is_uppercase_a(ch) ? 'z' - ('Z' - ch) : ch;
#endif // io_tolower
}

static constexpr inline bool is_uppercase_a(const char ch) noexcept
{
#ifdef io_isupper
	return io_isupper(ch);
#else
	return ( ch <= 'Z'  ) && ( ch >= 'A' );
#endif
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

// uri
uint16_t IO_NO_INLINE uri::default_port_for_scheme(const char* scheme) noexcept
{
	char sch[8];
	typedef std::char_traits<char> c8t;
	str_to_lower_a(sch, scheme);
	if(0 == c8t::compare("echo", sch, 4) )
		return 7;
	else if(0 == c8t::compare("daytime", sch, 7) )
		return 13;
	else if(0 == c8t::compare("ftp", sch, 3) )
		return 21;
	else if(0 == c8t::compare("ssh", sch, 3) )
		return 22;
	else if( 0 == c8t::compare("telnet",sch,6) )
		return 23;
	else if(0 == c8t::compare("mailto", sch, 6) )
		return 25;
	else if(0 == c8t::compare("time", sch, 4 ) )
		return 37;
	else if(0 == c8t::compare("name", sch, 4) )
		return 42;
	else if(0 == c8t::compare("domain", sch, 6) )
		return 53;
	else if(0 == c8t::compare("gopher", sch, 6) )
		return 70;
	else if(0 == c8t::compare("https", sch, 5 ) )
		return 443;
	else if( 0 == c8t::compare("http", sch, 4) )
		return 80;
	else if( 0 == c8t::compare("npp", sch, 3) )
		return 92;
	else if( 0 == c8t::compare("sftp", sch, 4) )
		return 115;
	else if( 0 == c8t::compare("irc", sch, 3) )
		return 6697;
	return 0;
}


/// std::regexp can throw an exception, and no PCRE
/// so parse it manually
s_uri uri::parse(std::error_code& ec, const char* str) noexcept
{
	char *normalized;
	const std::size_t len = io_strlen(str) + 1;
	if(len <= UCHAR_MAX)
		normalized = static_cast<char*>( io_alloca( len ) );
	else
		normalized = memory_traits::calloc_temporary<char>(len);
	str_to_lower_a(normalized, str);

	const_string scheme;
	const_string host;
	uint16_t port = 0;
	const_string user_info;
	const_string path;
	const_string query;
	const_string fragment;
	const char *b = normalized;
	const char *e = b;
	while( !is_one_of(*e,':','/') ) {
		if( cheq('\0',*e) )
			return return_error(ec, std::errc::invalid_argument);
		++e;
	}
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
			for( ; is_digit(*j) && j > b; j--);
			// has port
			if( cheq( ':', *j ) ) {
				host_end = j;
				port = static_cast<uint16_t>( std::strtoul(j+1,const_cast<char**>(&e),10) );
			}
		}
		// look for a user_info component
		const char *j = host_strt;
		for( ; is_user_info_character(*j) && j != host_end; j++);
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
		path = const_string( b, e );
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



}  // namespace net

} // namespace io
