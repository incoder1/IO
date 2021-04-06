/*
 *
 * Copyright (c) 2016-2021
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "xml_parse.hpp"
#include "strings.hpp"

namespace io {
namespace xml {

static constexpr const char* PROLOGUE = "xml";
static const char* COMMENT = "<!--";
static const char* CDATA    = "<![CDATA[";
static const char* DOCTYPE  = "<!DOCTYPE";

static constexpr const std::size_t MEDIUM_BUFF_SIZE = 128;
static constexpr const std::size_t HUGE_BUFF_SIZE = 256;

// UNICODE constants in digit forms
static constexpr const char ENDL = 0;
static constexpr const char LEFTB =  60; // '<';
static constexpr const char RIGHTB =  62; // '>';
static constexpr const char SRIGHTB = 93; // ']'
static constexpr const char QNM = 34; // '"'
static constexpr const char APH = 39; // '\''
static constexpr const char SPACE = 32;//' ';
static constexpr const char EM = 33;//'!';
static constexpr const char SOLIDUS = 47;// '/'
static constexpr const char HYPHEN = 45;// '-'
static constexpr const char COLON = 58; // ':'
static constexpr const char ES = 61 ; // '='
static constexpr const char QM = 63; // '?'


static inline bool is_prologue(const char *s) noexcept
{
	return start_with(s, PROLOGUE, 3) && is_space( s[3] );
}

static inline bool is_comment(const char *s) noexcept
{
	return start_with(s, COMMENT, 4);
}

static inline bool is_cdata(const char* s) noexcept
{
	return start_with(s, CDATA, 9);
}

static inline bool is_doc_type(const char *s) noexcept
{
	return start_with(s, DOCTYPE, 9);
}

static std::size_t prefix_delimit(const char* src) noexcept
{
	static const char* DELIMS = "\t\n\v\f\r :/>";
	return io_strcspn(src, DELIMS);
}

static size_t xmlname_strspn(const char *s) noexcept
{
	constexpr const char* sym = "\t\n\v\f\r />";
	return io_strcspn(s, sym);
}

static std::size_t extract_prefix(std::size_t &start, const char* str) noexcept
{
	const char *s = str;
	if( cheq(LEFTB,*s) ) {
		const std::size_t shift = cheq( SOLIDUS, *(s+1) ) ? 2 : 1;
		s += shift;
		start += shift;
	}
	s += prefix_delimit(s);
	if( chnoteq(COLON, *s) ) {
		start = 0;
		return 0;
	}
	return str_size( (str + start), s );
}

static std::size_t extract_local_name(std::size_t& start,const char* str) noexcept
{
	char *s = const_cast<char*>(str);
	start = 0;
	if( is_one_of(*s, LEFTB,COLON,QM) ) {
		++start;
		++s;
	}
	if( cheq(*s, SOLIDUS) ) {
		++start;
		++s;
	}
	s += xmlname_strspn(s);
	std::size_t ret = 0;
	if( io_unlikely( cheq(ENDL, *s) ) )
		start = 0;
	else
		ret = memory_traits::distance(str,s-1) - (start-1);
	return ret;
}


#if defined(__GNUG__) || defined(__ICL) || defined(__clang__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

static bool is_xml_name_char(uint32_t ch) noexcept
{
	switch( ch ) {
	case 0x5F:
	case 0x3A:
	case 0x2D:
	case 0x2E:
	case 0xB7:
	case 0x30 ... 0x39:
	case 0x41 ... 0x5A:
	case 0x61 ... 0x7A:
	case 0xC0 ... 0xD6:
	case 0xD8 ... 0xF6:
	case 0xF8 ... 0x2FF:
	case 0x370 ... 0x37D:
	case 0x37F ... 0x1FFF:
	case 0x200C ... 0x200D:
	case 0x203F ... 0x2040:
	case 0x2070 ... 0x218F:
	case 0x2C00 ... 0x2FEF:
	case 0x0300 ... 0x036F:
	case 0x3001 ... 0xD7FF:
	case 0xF900 ... 0xFDCF:
	case 0xFDF0 ... 0xFFFD:
	case 0x10000 ... 0xEFFFF:
		return true;
	default:
		return false;
	}
}

#pragma GCC diagnostic pop

#else

static constexpr bool is_xml_name_start_char_lo(char32_t ch) noexcept
{
	// _ | :
	return is_one_of(ch, U'_', U':') || is_alpha( ch );
}

template<
 std::make_unsigned<char32_t>::type S,
 std::make_unsigned<char32_t>::type E,
 std::make_unsigned<char32_t>::type D = ((E - S) + 1) >
static constexpr bool between(char32_t ch) noexcept {
	return (static_cast< std::make_unsigned<char32_t>::type >(ch)-S) < D;
}

static constexpr bool is_xml_name_start_char(char32_t ch) noexcept
{
	// Compiler optimize it better then search array
	return is_xml_name_start_char_lo(ch) ||
		   between<0xC0,0xD6>(ch)     ||
		   between<0xD8,0xF6>(ch)     ||
		   between<0xF8,0x2FF>(ch)    ||
		   between<0x370,0x37D>(ch)   ||
		   between<0x37F,0x1FFF>(ch)  ||
		   between<0x200C,0x200D>(ch) ||
		   between<0x2070,0x218F>(ch) ||
		   between<0x2C00,0x2FEF>(ch) ||
		   between<0x3001,0xD7FF>(ch) ||
		   between<0xF900,0xFDCF>(ch) ||
		   between<0xFDF0,0xFFFD>(ch)  ||
		   between<0x10000,0xEFFFF>(ch);
}

static constexpr bool is_xml_name_char(char32_t ch) noexcept
{
	return is_digit(ch) ||
		   // - | . | U+00B7
		   is_one_of(ch,0x2D,0x2E,0xB7) ||
		   is_xml_name_start_char(ch) ||
		   between<0x0300,0x036F>(ch)  ||
		   between<0x203F,0x2040>(ch);
}

#endif // __GNUG__


// Check XML name is correct according XML syntax
static error check_xml_name(const char* tn) noexcept
{
	// name can not start from digit
	if( io_unlikely( is_endl(*tn) || io_isdigit(*tn) ) )
		return error::illegal_name;
	uint32_t utf32c;
	do {
		// decode UTF-8 symbol to UTF-32 to check name
		switch( utf8::mblen(tn) ) {
		case io_likely(1):
			utf32c = static_cast<uint32_t>( *tn );
			++tn;
			break;
		case 2:
			utf32c = utf8::decode2( tn );
			tn += 2;
			break;
		case 3:
			utf32c = utf8::decode3( tn );
			tn += 3;
			break;
		case 4:
			utf32c = utf8::decode4( tn );
			tn += 4;
			break;
		default:
			return error::illegal_name;
		}
		if( !is_xml_name_char(utf32c) )
			return error::illegal_name;
	} while( not_endl(*tn) );
	return error::ok;
}

static error validate_tag_name(const char* name) noexcept
{
	// check XML,xMl,xml etc
	char first[3];
	for(std::size_t i=0; i < 3; i++)
		first[i] = latin1_to_lower(name[i]);
	if( start_with(first, PROLOGUE, 3) )
		return error::illegal_name;
	return check_xml_name(name);
}

static error validate_attribute_name(const char* name) noexcept
{
	return check_xml_name(name);
}

// event_stream_parser
s_event_stream_parser event_stream_parser::open(std::error_code& ec,s_source&& src) noexcept
{
	if(!src) {
		ec = std::make_error_code( std::errc::bad_address );
		return s_event_stream_parser();
	}
	s_string_pool pool = string_pool::create(ec);
	if(!pool)
		return s_event_stream_parser();
	return s_event_stream_parser( nobadalloc<event_stream_parser>::construct( ec, std::move(src), std::move(pool) ) );
}

s_event_stream_parser event_stream_parser::open(std::error_code& ec,s_read_channel&& src) noexcept
{
	s_source xmlsrc = source::create(ec, std::forward<s_read_channel>(src) );
	return !ec ? open(ec, std::move(xmlsrc) ) : s_event_stream_parser();
}

event_stream_parser::event_stream_parser(s_source&& src, s_string_pool&& pool) noexcept:
	object(),
	src_( std::forward<s_source>(src) ),
	state_(),
	current_(event_type::start_document),
	pool_(std::forward<s_string_pool>(pool)),
	validated_(),
	nesting_(0)
{
	constexpr std::size_t VD_INITIAL = 64;
	validated_.reserve( VD_INITIAL );

	// skip any leading spaces if any
	char c;
	do {
		c = next();
	}
	while( is_space(c) && error_state_ok() );

	if( io_unlikely( chnoteq(c,LEFTB) ) ) {
		assign_error(error::illegal_markup);
	}
	else {
		sb_clear();
		scan_buf_[0] = '<';
	}
}

event_stream_parser::~event_stream_parser() noexcept
{}

inline void event_stream_parser::assign_error(error ec) noexcept
{
	state_.current = state_type::eod;
	if(error::ok == state_.ec)
		state_.ec = ec;
}

inline void event_stream_parser::putch(byte_buffer& buf, char ch) noexcept
{
	if( io_unlikely( !buf.put(ch) && ( !buf.ln_grow() || !buf.put(ch) ) ) )
		assign_error(error::out_of_memory);
}

const_string event_stream_parser::precache(const char* str) noexcept
{
	return pool_->get(str);
}


// extract name and namespace prefix if any
qname event_stream_parser::extract_qname(const char* from, std::size_t& len) noexcept
{
	const_string prefix;
	const_string local_name;
	len = 0;
	std::size_t start = 0;
	std::size_t count = extract_prefix( start, from );
	if( count > 0 )
		prefix = pool_->get( (from+start), count);
	len += start+count;
	const char* name = from+len;
	count = extract_local_name(start,name);
	if(count > 0) {
		local_name = pool_->get( (name+start), count);
	}
	else {
		assign_error(error::illegal_name);
		return qname();
	}
	len += start+count;
	const char* left = from + len;
	if( cheq(SOLIDUS,*left) ) {
		++len;
		++left;
	}
	if(cheq(RIGHTB,*left))
		++len;
	return qname( std::move(prefix), std::move(local_name) );
}

state_type event_stream_parser::scan_next() noexcept
{
	if(state_type::eod != state_.current)
		scan();
	return state_.current;
}

byte_buffer event_stream_parser::read_entity() noexcept
{
	byte_buffer ret;
	ret.extend(MEDIUM_BUFF_SIZE);
	if( !ret ) {
		assign_error(error::out_of_memory);
		return byte_buffer();
	}
	ret.put( scan_buf_ );
	sb_clear();
	src_->read_until_char( ret, static_cast<char>(RIGHTB), static_cast<char>(LEFTB) );
	if( src_->eof() ) {
		assign_error( src_->last_error() );
		return byte_buffer();
	}
	ret.flip();
	return ret;
}

#define check_state( _STATE_TYPE, _EMPTY_RET_TYPE)\
	if( state_.current != _STATE_TYPE ) {\
		assign_error(error::invalid_state);\
		return _EMPTY_RET_TYPE();\
	}

#define check_event_parser_state( _EVENT_TYPE, _EMPTY_RET_TYPE )\
    if(state_type::event != state_.current || current_ != _EVENT_TYPE ) { \
        assign_error(error::invalid_state); \
        return _EMPTY_RET_TYPE(); \
    }

document_event event_stream_parser::parse_start_doc() noexcept
{
	static constexpr const char* VERSION  = "version=";
	static constexpr const char* ENCODING = "encoding=";
	static constexpr const char* STANDALONE = "standalone=";
	static constexpr const char* YES = "yes";
	static constexpr const char* NO = "no";
	static constexpr const char* END_PROLOGUE = "?>";

	check_event_parser_state(event_type::start_document, document_event )

	byte_buffer buff( read_entity() );

	if( is_error() )
		return document_event();

	buff.shift(5);

	const_string version, encoding;
	bool standalone = false;

	const char* prologue = buff.position().cdata();
	// extract version
	char* i = io_strstr( const_cast<char*>(prologue), VERSION );

	if(nullptr == i) {
		assign_error(error::illegal_prologue);
		return document_event();
	}
	i += 8; // i + strlen(VERSION)
	int sep = std::char_traits<char>::to_int_type( *i );
	if( !is_one_of(sep,QNM,APH) ) {
		assign_error(error::illegal_prologue);
		return document_event();
	}
	else
		++i;
	char *stop = io_strchr(i, sep);
	if(nullptr == stop )  {
		assign_error(error::illegal_prologue);
		return document_event();
	}
	version = const_string( i, stop);

	if( version.empty() ) {
		assign_error(error::out_of_memory);
		return document_event();
	}

	// extract optional
	i = const_cast<char*>( stop + 1 );

	// extract encoding if exist
	const char* j = io_strstr(i, ENCODING);
	if(nullptr != j) {
		i = const_cast<char*>( j + 9 );
		sep = std::char_traits<char>::to_int_type( *i );
		if( !is_one_of(sep,QNM,APH) ) {
			assign_error(error::illegal_prologue);
			return document_event();
		}
		else
			++i;
		stop  = io_strchr( i, sep );
		if(nullptr == stop ) {
			assign_error(error::illegal_prologue);
			return document_event();
		}
		encoding = const_string(i,stop);
		if( encoding.empty() ) {
			assign_error(error::out_of_memory);
			return document_event();
		}
		i = const_cast<char*> ( stop + 1 );
	}
	// extract standalone if exist
	j = io_strstr(i, STANDALONE);
	if(nullptr != j) {
		// j + strlen(STANDALONE)
		i = const_cast<char*> ( j + 11 );
		sep = std::char_traits<char>::to_int_type( *i );
		if( !is_one_of(sep,QNM,APH) ) {
			assign_error(error::illegal_prologue);
			return document_event();
		}
		else
			++i;
		stop  = io_strchr( i, sep );
		if(nullptr == stop || (str_size(i,stop) > 3) ) {
			assign_error(error::illegal_prologue);
			return document_event();
		}
		standalone =  ( 0 == io_memcmp( i, YES, 3) );
		if( !standalone &&  ( 0 != io_memcmp(i, NO, 2) ) ) {
			assign_error(error::illegal_prologue);
			return document_event();
		}
		i = const_cast<char*> ( stop + 1 );
	}
	// check error in this point
	if( 0 != io_memcmp( skip_spaces(i), END_PROLOGUE, 2) ) {
		assign_error(error::illegal_prologue);
		return document_event();
	}
	return document_event( std::move(version), std::move(encoding), standalone);
}

instruction_event event_stream_parser::parse_processing_instruction() noexcept
{
	check_event_parser_state(event_type::processing_instruction, instruction_event)
	byte_buffer buff = read_entity();
	if( is_error() )
		return instruction_event();
	buff.move(1);
	char *i = const_cast<char*>( buff.position().cdata() );
	std::size_t start = 0;
	std::size_t len = extract_local_name(start,i);
	i += start;
	const_string target(i,len);
	if( target.empty() ) {
		assign_error(error::out_of_memory);
		return instruction_event();
	}
	i += len;
	len = io_strlen(i);
	const_string data( i, len-2);
	if( target.empty() ) {
		assign_error(error::out_of_memory);
		return instruction_event();
	}
	return instruction_event( std::move(target), std::move(data) );
}

void event_stream_parser::skip_dtd() noexcept
{
	if(state_type::dtd != state_.current) {
		assign_error(error::invalid_state);
		return;
	}
	std::size_t brackets = 1;
	do {
		switch( std::char_traits<char>::to_int_type( next() ) ) {
		case EOF:
			assign_error(error::illegal_dtd);
			break;
		case LEFTB:
			++brackets;
			break;
		case RIGHTB:
			--brackets;
			break;
		default:
			break;
		}
	}
	while( brackets > 0);
}

const_string event_stream_parser::read_dtd() noexcept
{
	check_state(state_type::dtd, const_string)
	std::error_code ec;
	byte_buffer dtd = byte_buffer::allocate(ec, MEDIUM_BUFF_SIZE);
	if( ec ) {
		assign_error(error::out_of_memory);
		return const_string();
	}
	dtd.put(scan_buf_);
	sb_clear();
	std::size_t brackets = 1;
	char i;
	do {
		i = next();
		switch( std::char_traits<char>::to_int_type(i) ) {
		case EOF:
			assign_error(error::illegal_dtd);
			return const_string();
		case LEFTB:
			++brackets;
			break;
		case RIGHTB:
			--brackets;
			break;
		default:
			break;
		}
		putch(dtd, i );
	}
	while( brackets > 0 && error_state_ok() );
	dtd.flip();
	return const_string( dtd.position().cdata(), dtd.last().cdata() );
}

void event_stream_parser::skip_comment() noexcept
{
	if(state_type::comment != state_.current) {
		assign_error(error::invalid_state);
		return;
	} else if( scan_failed() ) {
		assign_error(error::illegal_commentary);
		return;
	}
	sb_clear();
	constexpr const uint16_t double_hyphen = pack_word( static_cast<uint16_t>('-'), '-');
	uint16_t hw = 0;
	char c;
	do {
		c = next();
		hw = pack_word(hw, c);
	}
	while( double_hyphen != hw && io_likely( !is_eof(c) && error_state_ok() ) );
	if( chnoteq(RIGHTB, next() ) )
		assign_error(error::illegal_commentary);
}

byte_buffer event_stream_parser::read_until_double_separator(const char separator,const error ec) noexcept
{
	if( scan_failed() ) {
		assign_error(ec);
		return byte_buffer();
	}

	byte_buffer ret;
	ret.extend( HUGE_BUFF_SIZE );
	if( !ret ) {
		assign_error(error::out_of_memory);
		return ret;
	}

	sb_clear();

	src_->read_until_double_char( ret, separator );

	if( ret.empty() || chnoteq(RIGHTB, next() ) ) {
		if( error::out_of_memory == src_->last_error() )
			assign_error(error::out_of_memory);
		else
			assign_error( ec );
		return byte_buffer();
	}
	ret.flip();
	return ret;
}

const_string event_stream_parser::read_comment() noexcept
{
	check_state(state_type::comment, const_string)
	constexpr std::size_t END_LEXEM_LEN = 3; // --> len
	byte_buffer tmp( read_until_double_separator(HYPHEN, error::illegal_commentary) );
	if( tmp.empty() || 0 == io_strcmp("--", tmp.position().cdata() ) )
		return  const_string();
	else
		return const_string( tmp.position().cdata(), tmp.last().cdata()-END_LEXEM_LEN );
}

const_string event_stream_parser::read_chars() noexcept
{
	check_state(state_type::characters, const_string)
	byte_buffer ret;
	ret.extend( HUGE_BUFF_SIZE );
	if( !ret ) {
		assign_error(error::out_of_memory);
		return const_string();
	}
	// just "\s<" in scan stack
	//const char *i = io_strchr(scan_buf_+1, RIGHTB);
	if( is_space(scan_buf_[0]) && cheq(scan_buf_[1],RIGHTB) ) {
		io_memmove(scan_buf_, "<", 2);
		return const_string(scan_buf_, 1);
	}
	// check for <tag></tag>
	char c = next();
	if( cheq(c,LEFTB) ) {
		io_memmove(scan_buf_, "<", 2);
		return const_string();
	}
	else
		ret.put( c );

	src_->read_until_char(ret, '<', '>');
	error errc = src_->last_error();
	if( io_unlikely( error::ok != errc  ) ) {
		if(error::illegal_markup == errc)
			assign_error(error::root_element_is_unbalanced);
		else
			assign_error( errc );
	}
	else if( !ret.empty() ) {
		io_memmove(scan_buf_, "<", 2);
		ret.flip();
		// don't add last <
		return const_string( ret.position().cdata(), ret.length()-1 );
	}
	return const_string();
}

void event_stream_parser::skip_chars() noexcept
{
	if(state_type::characters != state_.current) {
		assign_error(error::invalid_state);
		return;
	}
	for(int i = std::char_traits<char>::to_int_type( next() ); error_state_ok() ; i = std::char_traits<char>::to_int_type( next() ) ) {
		switch(i) {
		case LEFTB:
			sb_clear();
			sb_append( static_cast<char>(LEFTB) );
			return;
		case RIGHTB:
			sb_clear();
			assign_error(error::illegal_chars);
			return;
		case io_unlikely(EOF):
			sb_clear();
			assign_error(error::root_element_is_unbalanced);
			return;
		}
	}

}

const_string event_stream_parser::read_cdata() noexcept
{
	check_state(state_type::cdata, const_string)
	byte_buffer tmp( read_until_double_separator(SRIGHTB, error::illegal_cdata_section) );
	tmp.flip();
	return const_string( tmp.position().cdata(), tmp.last().cdata()-3 );
}

attribute event_stream_parser::extract_attribute(const char* from, std::size_t& len) noexcept
{
	len = 0;
	// skip lead spaces, don't copy them into name
	const char *i = skip_spaces(from);
	if( nullptr == i || is_one_of(*i, SOLIDUS,RIGHTB, ENDL) )
		return attribute();

	const char* start = i;
	i = io_strchr(start, ES);
	if( nullptr == i || !is_one_of( i[1], QNM, APH) ) {
		assign_error(error::illegal_markup);
		return attribute();
	}

	const char val_sep = *(++i);

	const_string np;
	const_string ln;
	// find prefix if any, ans split onto qualified name
	char *tmp = strchrn( start, COLON, str_size(start,i) );
	if(nullptr != tmp) {
		np = pool_->get( start,  str_size(start, tmp) );
		start = tmp + 1;
	}
	ln = pool_->get(start, str_size(start, i-1) );
	// extract attribute value
	++i; // skip ( "|' )
	start = i;
	// find closing value separator
	i = io_strchr(i, val_sep );
	if(nullptr == i) {
		assign_error(error::illegal_attribute);
		return attribute();
	}
	// check for empty attribute value
	// not valid according W3C, but can be present
	// in sort of generated xmls
	const std::size_t val_size =  str_size(start,i);
	// empty value attribute, return
	if( io_unlikely( val_size < 1 ) ) {
		len = str_size(from, i+1);
		return attribute( qname( std::move(np), std::move(ln) ), io::const_string() );
	}
	const_string value(start, val_size);
	if( io_unlikely( value.empty() ) ) {
		assign_error(error::out_of_memory);
		return attribute();
	}
	// normalize attribute value
	// replace any white space characters to space character
	// according to W3C XML spec
	char *v = const_cast<char*>( value.data() );
	constexpr const char* NOT_SPACE_WS = "\t\n\v\f\r";
	do {
	 	v += io_strcspn(v,NOT_SPACE_WS);
	 	if( not_endl(*v) )
        	*v = ' ';
	} while( not_endl(*v) );
	len = str_size(from, ++i);
	return attribute( qname( std::move(np), std::move(ln) ), std::move(value) );
}

bool event_stream_parser::validate_xml_name(const const_string& str, bool attr) noexcept
{
	std::size_t str_hash = str.hash();
	if( validated_.end() == validated_.find( str_hash ) ) {
		const char *s = str.data();
		error err;
		if(attr)
			err = validate_attribute_name( s );
		else
			err = validate_tag_name( s );
		if(error::ok != err ) {
			assign_error( err );
			return false;
		}
		validated_.insert( str_hash );
		return true;
	}
	return true;
}

inline char event_stream_parser::next() noexcept
{
	return src_->next();
}

bool event_stream_parser::validate_attr_name(const qname& name) noexcept
{
	bool ret = validate_xml_name( name.local_name(), true );
	if( ret && name.has_prefix() )
		ret = validate_xml_name(name.prefix(), true);
	return ret;
}

bool event_stream_parser::validate_element_name(const qname& name) noexcept
{
	bool ret = validate_xml_name( name.local_name(), false );
	if( ret && name.has_prefix() )
		ret = validate_xml_name(name.prefix(), false);
	return ret;
}

start_element_event event_stream_parser::parse_start_element() noexcept
{
	check_event_parser_state(event_type::start_element, start_element_event);

	byte_buffer buff = read_entity();
	if( is_error() )
		return start_element_event();
	constexpr std::size_t SELF_CLOSE_LEN = 3; // len of </ from last
	bool empty_element = cheq(SOLIDUS, *(buff.last().cdata()-SELF_CLOSE_LEN) );
	// nesting level for nodes balance
	if( !empty_element )
		++nesting_;
	std::size_t len = 0;
	qname name = extract_qname( buff.position().cdata(), len );
	// check name validity
	if(is_error() || !validate_element_name(name) )
		return start_element_event();

	start_element_event result( std::move(name), empty_element );
	// extract attributes if any
	const char *left =  buff.position().cdata() + len;
	if( is_space(*left) && error_state_ok() ) {
		std::size_t offset;
		attribute attr = extract_attribute(left,offset);
		while( (0 < offset) && error_state_ok() ) {
			// validate attribute name and check for
			// double attributes with the same name check
			// according to W3C XML spec
			if( !validate_attr_name( attr.name() ) || !result.add_attribute( std::move(attr) ) ) {
				assign_error( error::illegal_attribute );
			} else {
				// extract next attribute if there were any
				attr = extract_attribute( (left += offset) ,offset);
			}
		}
	}
	return error_state_ok() ?  std::move(result) : start_element_event();
}

end_element_event event_stream_parser::parse_end_element() noexcept
{
	check_event_parser_state(event_type::end_element, end_element_event )
	qname name;
	if( io_likely(nesting_ > 0) ) {

		if(0 == (--nesting_) )
			state_.current =  state_type::eod;

		byte_buffer buff = read_entity();
		if( error_state_ok() ) {
			std::size_t len = 0;
			name = extract_qname( buff.position().cdata(), len );
		}
	} else {
		assign_error(error::root_element_is_unbalanced);
	}
	return end_element_event( std::move(name) );
}

void event_stream_parser::s_instruction_or_prologue() noexcept
{
	if( 0 != nesting_ ) {
		assign_error(error::illegal_markup);
		return;
	}
	constexpr std::size_t SCAN_START = 2;
	constexpr std::size_t MAX_SCAN = 7;
	for(std::size_t i=SCAN_START; i < MAX_SCAN; i++) {
		scan_buf_[i] = next();
	}
	if( scan_failed() ) {
		assign_error(error::illegal_markup);
	}
	else if( is_prologue(scan_buf_+SCAN_START) ) {
		if( state_type::initial == state_.current ) {
			current_ = event_type::start_document;
			state_.current = state_type::event;
		}
		else {
			assign_error(error::illegal_prologue);
		}
	}
	else {
		current_ = event_type::processing_instruction;
		state_.current = state_type::event;
	}
}

void event_stream_parser::s_comment_cdata_or_dtd() noexcept
{
	scan_buf_[2] = next();
	scan_buf_[3] = next();
	if( scan_failed()  ) {
		assign_error(error::root_element_is_unbalanced);
	}
	else if( is_comment(scan_buf_) ) {
		state_.current =  state_type::comment;
	}
	else {
		constexpr std::size_t SCAN_START = 4;
		constexpr std::size_t MAX_SCAN = 9;
		for(std::size_t i = SCAN_START; i < MAX_SCAN; i++) {
			scan_buf_[i] = next();
		}
		// check for the EOF in the scan buffer
		if( scan_failed() )
			assign_error(error::root_element_is_unbalanced);
		else if( is_cdata(scan_buf_) )
			state_.current = state_type::cdata;
		else if( is_doc_type(scan_buf_) )
			state_.current = state_type::dtd;
		else
			assign_error(error::illegal_markup);
	}
}

void event_stream_parser::s_entity() noexcept
{
	scan_buf_[1] = next();
	const int second = std::char_traits<char>::to_int_type( scan_buf_[1] );
	// scan on exact entity type
	switch( second ) {
	case SOLIDUS:
		// </foo
		state_.current = state_type::event;
		current_ = event_type::end_element;
		break;
	case EM:
		// <!
		s_comment_cdata_or_dtd();
		break;
	case QM:
		// <?
		s_instruction_or_prologue();
		break;
	default:
		// <foo
		if( io_likely( !is_space(second) ) ) {
			state_.current = state_type::event;
			current_ = event_type::start_element;
		}
		else {
			assign_error( error::illegal_markup );
		}
	}
}

void event_stream_parser::scan() noexcept
{
	switch( std::char_traits<char>::to_int_type(*scan_buf_) ) {
	// this is entity (tag or instruction) begin
	case LEFTB:
		// jump to scan for exact entity type
		s_entity();
		break;
	// this is end of document, no more data from source
	case EOF:
		state_.current = state_type::eod;
		// no root tag in this xml
		// i.e. something like <?xml version="1.0"?> <!-- <root>...</root> -->
		if( 0 != nesting_)
			assign_error(error::root_element_is_unbalanced);
		break;
	default:
		// this is characters state, i.e. tag value or between tags spaces
		state_.current = state_type::characters;
	}
}

} // namesapce xml

} // namesapce io
