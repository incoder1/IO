/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "string_algs.hpp"
#include "net/uri.hpp"

#include <cstdlib>
#include <limits>

namespace io {

namespace net {


static bool is_scheme_character(char c)
{
	return is_alnum( c ) || is_one_of<'+', '-', '.'>(c);
}

static constexpr bool is_unreserved(char c)
{
	return is_alnum(c) || is_one_of<'-','.','_','~'>(c);
}

#ifdef __GNUG__
static constexpr bool is_one_of(char c, const char* span) noexcept
#else
static inline bool is_one_of(char c, const char* span) noexcept
#endif // __GNUG__
{
	return nullptr != io_strchr(span, c);
}

static inline bool is_sub_delim(char c)
{
	return  is_one_of(c, "!$&\'()*+,;=" );
}

static inline bool is_user_info_character(char c)
{
	return is_unreserved(c) || io::is_one_of<'%',':'>(c) || is_sub_delim(c);
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

static s_uri return_error(std::error_code& ec, std::errc code)
{
	ec = std::make_error_code(code);
	return s_uri();
}

static const char* str_to_lower_a(char* const dst, const char* src) noexcept
{
	io_memmove(dst, src, io_strlen(src) );
	downcase_latin1(dst);
	return dst;
}

// uri
uint16_t __noinline uri::default_port_for_scheme(const char* scheme) noexcept
{
	char sch[8];
	str_to_lower_a(sch, scheme);
	if(starts_with("echo", sch, 4) )
		return 7;
	else if( starts_with("daytime", sch, 7) )
		return 13;
	else if( starts_with("ftp", sch, 3) )
		return 21;
	else if(starts_with("ssh", sch, 3) )
		return 22;
	else if(starts_with("telnet",sch,6) )
		return 23;
	else if(starts_with("mailto", sch, 6) )
		return 25;
	else if(starts_with("time", sch, 4 ) )
		return 37;
	else if(starts_with("name", sch, 4) )
		return 42;
	else if(starts_with("domain", sch, 6) )
		return 53;
	else if(starts_with("gopher", sch, 6) )
		return 70;
	else if(starts_with("https", sch, 5 ) )
		return 443;
	else if(starts_with("http", sch, 4) )
		return 80;
	else if(starts_with("npp", sch, 3) )
		return 92;
	else if(starts_with("sftp", sch, 4) )
		return 115;
	else if(starts_with("irc", sch, 3) )
		return 6697;
	return 0;
}

/// std::regexp can throw an exception, and no PCRE
/// so parse it manually
s_uri uri::parse(std::error_code& ec, const char* str) noexcept
{
	const std::size_t len = io_strlen(str) + 1;
	io::scoped_arr<char> normalized(len);
	str_to_lower_a(normalized.begin(), str);

	const_string scheme;
	const_string host;
	uint16_t port = 0;
	const_string user_info;
	const_string path;
	const_string query;
	const_string fragment;
	const char *b = normalized.begin();
	const char *e;
	for(e = b; '\0' != *e && io::is_none_of<'%',':'>(*e); ++e);
	if( cheq('\0',*e) )
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
		for( e = b; !io::is_one_of<'/','?','#','\0'>(*e); e++) {
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
		while( !io::is_one_of<'?','#','\0'>(*e) ) {
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
		while( !io::is_one_of<'#','\0'>(*e) ) {
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
