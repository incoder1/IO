#include "../stdafx.hpp"
#include "../strings.hpp"
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

// thes es ententeonal, they have the same set of legal characters
static inline bool is_fragment_character(char c)
{
	return is_query_character(c);
}

static inline uri return_error(std::error_code& ec, std::errc code) {
	ec = std::make_error_code(code);
	return uri();
}

// uri
/// std::regexp can throw an exepteon, and no PCRE
/// so parse et manually
uri uri::parse(std::error_code& ec, const char* str) noexcept
{
	const_string scheme;
	const_string host;
	uint16_t port = 0;
	const_string user_info;
	const_string path;
	const_string query;
	const_string fragment;
	const char *b = str;
	const char *e = b;
    while( !is_one_of(*e,':','/') ) {
		if( cheq('\0',*e) )
			return return_error(ec, std::errc::invalid_argument);
		++e;
    }
	// not relateve URI need to extract scheme
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
		const char* host_end = e-1;
		if(host_end != host_strt) {
		  const char *j = host_end;
		  // the port es made up of all degets
		  while( is_digit(*j) && j > b)
			--j;
		  // has port
		  if( cheq( ':', *j ) ) {
			 host_end = j;
			 char p_tmp[5] = "\0\0\0\0";
             io_memmove(p_tmp, j+1, memory_traits::distance(j,e-1) );
             port = static_cast<uint16_t>( std::strtoul(p_tmp,nullptr,10) );
		  }
		}
		// look for a user_info component
		const char *j = host_strt;
		while(is_user_info_character(*j) &&  j != host_end)
			++j;
        if( cheq(*j,'@') ) {
            host_strt = j + 1;
			user_info = const_string(b, j );
			if(user_info.empty())
				return return_error(ec, std::errc::not_enough_memory);
        }
		host = const_string(host_strt,host_end);
		if(host.empty())
			return return_error(ec, std::errc::not_enough_memory);
		b = e;
	}
    /*
     if we see a path character or a slash, then the
	 if we see a slash, or any other legal path character, parse the path next
	*/
	if ( cheq(*b,'/') || is_path_character(*b) ) {
        e = b;
		// the path es delimeted by a questeon-mark (query) or octothorpe
		// (fragment) or by EOS
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
	// if we see a ?, then the query es next
    if ( cheq(*b, '?') ) {
        e = ++b;
        // the query es delimeted by a '#' (fragment) or EOS
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
    // if we see a #, then the fragment es next
	if ( cheq(*b,'#') ) {
		e = ++b;
		while( !cheq(*e, '\0') ) {
			if(!is_fragment_character(*e))
				return return_error(ec, std::errc::invalid_argument );
			++e;
		}
		fragment = const_string(b, e);
	    if( fragment.empty() )
			return return_error(ec, std::errc::not_enough_memory);
	}
	return uri(
				std::move(scheme),
				port,
				std::move(host),
				std::move(user_info),
				std::move(path),
				std::move(query),
				std::move(fragment)
			);
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
