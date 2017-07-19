/*
 *
 * Copyright (c) 2016
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

static const char* PROLOGUE = "xml";
static const char* CDATA = "[CDATA[";
static const char* DOCTYPE  = "DOCTYPE";

static const std::size_t SMALL_BUFF_SIZE = 32;
static const std::size_t MID_BUFF_SIZE = 64;
static const std::size_t HUGE_BUFF_SIZE = 128;

// unicode constants in digit forms, to handle endians
static constexpr int LEFTB =  60; // '<';
static constexpr int RIGHTB =  62; // '>';
static constexpr int SRIGHTB = 93; // ']'
static constexpr int QNM = 34; // '"'
static constexpr int QM = 63; // '?'
static constexpr int EM = 33;//'!';
static constexpr int SPACE = 32;//' ';
static constexpr int SOLIDUS = 47;// '/'
static constexpr int HYPHEN = 45;// '-'
static constexpr int COLON = 58; // ':'
static constexpr int ENDL = 0;
static constexpr int iEOF = std::char_traits<char32_t>::to_int_type( std::char_traits<char32_t>::eof() );

static inline bool is_prologue(const char *s) noexcept
{
	return start_with(s, PROLOGUE, 3) && is_whitespace( *(s+3) );
}

static inline bool is_comment(const char *s) noexcept
{
	// start_with(s, "--", 2);
	return cheq(*s,HYPHEN) && cheq(*(s+1),HYPHEN);
}

static inline bool is_cdata(const char* s) noexcept
{
	return start_with(s, CDATA, 7);
}

static inline bool is_doc_type(const char *s) noexcept
{
	return start_with(s, DOCTYPE, 7);
}

static std::size_t extract_prefix(std::size_t& start,const char* str) noexcept {
	start = 0;
	char *s = const_cast<char*>(str);
	if( cheq(LEFTB,*s) )
		return 0;
	++start;
	++s;
	if(cheq(SOLIDUS,*s)) {
		++s;
		++start;
	}
	s += io_strcspn(s,"\t\n\v\f\r :/>");
	if( !cheq(*s,COLON)  ) {
		start = 0;
		return 0;
	}
	return str_size(str, (s-1) ) - (start-1);
}

static std::size_t extract_local_name(std::size_t& start,const char* str) noexcept {
	start = 0;
	char *s = const_cast<char*>(str);
	if( !is_one_of(*s, LEFTB,COLON,QM) )
		return 0;
	++start;
	++s;
	if(cheq(SOLIDUS,*s)) {
		++s;
		++start;
	}
	s +=  xmlname_strspn(s);
	if( cheq(ENDL, *s) ) {
		start = 0;
		return 0;
	}
	return memory_traits::distance(str,s-1) - (start-1);
}


static error check_xml_name(const char* tn) noexcept {
	if( is_digit(*tn) )
		return error::illegal_name;
	// need to convert into USC-4 in this point
	std::size_t len = io_strlen(tn);
	uint32_t *ch32 = nullptr;
	if(0 == len) {
		return error::illegal_name;
	} else if(len < UCHAR_MAX) {
		// stack in this point
		ch32 = static_cast<uint32_t*>( io_alloca( len*4 ) );
	} else {
		// a huge name, more then 255 bytes
		ch32 = static_cast<uint32_t*>( memory_traits::malloc( len*4 ) );
		if(nullptr == ch32) {
			return error::out_of_memory;
		}
	}
	std::error_code ec;
	std::size_t ulen = transcode(ec,
				reinterpret_cast<const uint8_t*>(tn),len,
				reinterpret_cast<char32_t*>(ch32),len);
	if( ec || !ulen ) {
		// huge name, release memory for UCS-4
		if(len >= 256)
			memory_traits::free(ch32);
		return error::illegal_name;
	}
	// name start char
	if( !is_xml_name_start_char(*ch32) )
		return error::illegal_name;
	// rest of name
	for(std::size_t i = 1; i < ulen; i++) {
		if( !is_xml_name_char( *(ch32+i) ) )
			return error::illegal_name;
	}
	if(len >= UCHAR_MAX)
		memory_traits::free(ch32);
	return error::ok;
}

static error validate_tag_name(const char* name) noexcept {
	 // check XML,xMl,xml etc
	char first[3];
	for(std::size_t i=0; i < 3; i++) {
		first[i] = latin1_to_lower(name[i]);
	}
	if( start_with(first, "xml", 3) ) {
		return error::illegal_name;
	}
	return check_xml_name(name);
}

static inline error validate_attribute_name(const char* name) noexcept {
	return check_xml_name(name);
}

// event_stream_parser
event_stream_parser::event_stream_parser(const s_source& src, s_string_pool&& pool):
	object(),
	src_(src),
	state_({error::ok,state_type::initial}),
	current_(event_type::start_document),
	pool_(std::forward<s_string_pool>(pool)),
	validated_(),
	nesting_(0)
{
}

event_stream_parser::~event_stream_parser() noexcept
{
}

void event_stream_parser::assign_error(error ec) noexcept
{
	state_.current = state_type::eod;
	if(error::ok == state_.ec) {
		state_.ec = ec;
	}
}

// extract name and namespace prefix if any
qname event_stream_parser::extract_qname(const char* from, std::size_t& len) noexcept {
	cached_string prefix;
	cached_string local_name;
	len = 0;
	std::size_t start = 0;
	std::size_t count = extract_prefix(start,from);
	if( count > 0 ) {
		prefix = pool_->get( from+start, count);
	}
	len += start+count;
	const char* name = from+len;
	count = extract_local_name(start,name);
	if(count > 0) {
		local_name = pool_->get(name+start, count);
	} else {
		assign_error(error::illegal_name);
		return qname();
	}
	len += start+count;
	char* left = const_cast<char*>( from + len);
	if( cheq(SOLIDUS,*left) ) {
		++len;
		++left;
	}
	if(cheq(RIGHTB,*left)) {
		++len;
	}
	return qname( std::move(prefix), std::move(local_name) );
}

state event_stream_parser::scan_next() noexcept
{
	if(state_type::eod != state_.current) {
		scan();
	}
	return state_;
}

char event_stream_parser::skip_to_symbol(const char symbol) noexcept
{
	char i = next();
	while( !cheq( symbol, i) && !is_eof(i) ) {
		i = next();
	}
	return i;
}

byte_buffer event_stream_parser::read_entity() noexcept
{
	std::error_code ec;
	byte_buffer result = byte_buffer::allocate(ec, MID_BUFF_SIZE);
	if( !check_buffer(result) ) {
		return result;
	}
	result.put(scan_buf_, sb_len(scan_buf_) );
	sb_clear(scan_buf_);
	constexpr int_fast32_t EOS = std::char_traits<int_fast32_t>::eof();
	int_fast32_t ch;
	bool done = false;
	do {
		ch = static_cast<int_fast32_t>( next() );
		switch(ch) {
		case LEFTB:
		case EOS:
			assign_error(error::illegal_markup);
			done = true;
			break;
		case RIGHTB:
			putch(result, static_cast<char>(ch) );
			done = true;
			break;
		default:
			putch(result, static_cast<char>(ch) );
		}
		if(is_error())
			break;
	} while( !done );
	if(done) {
		result.flip();
		return result;
	}
	return byte_buffer();
}

document_event event_stream_parser::parse_start_doc() noexcept
{
	if(state_type::event != state_.current || current_ != event_type::start_document)  {
		assign_error(error::invalid_state);
		return document_event();
	}
	byte_buffer buff( read_entity() );
	if( !check_buffer(buff) ) {
		return document_event();
	}
	buff.shift(5);
	const_string version, encoding;
	bool standalone = false;
	const char* prologue = buff.position().cdata();
    // extract version
    char* i = io_strstr(prologue, "version=\"");
    if(nullptr == i) {
		assign_error(error::illegal_prologue);
		return document_event();
    }
    i += 9;
    const char* stop = i;
    while( !cheq(QNM, *stop) )
		++stop;
    if(nullptr == stop || cheq('\0',*stop) ) {
		assign_error(error::illegal_prologue);
		return document_event();
    }
    version = const_string( i, stop);
    if( version.empty() ) {
		assign_error(error::out_of_memory);
		return document_event();
    }
    // extract optionalal
    i = const_cast<char*>( stop + 1 );
    // extract encoding if exist
    const char* j = io_strstr(i, "encoding=\"");
    if(nullptr != j) {
		i = const_cast<char*>( j + 10 );
		stop  = tstrchr( i, QNM );
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
    j = io_strstr(i, "standalone=\"");
	if(nullptr != j) {
		i = const_cast<char*> ( j + 12 );
		stop  = tstrchr( i, QNM);
		if(nullptr == stop ) {
			assign_error(error::illegal_prologue);
			return document_event();
		}
		if(str_size(i,stop) > 3) {
			assign_error(error::illegal_prologue);
			return document_event();
		}
        standalone =  ( 0 == io_memcmp( i, "yes", 3) );
        if( !standalone ) {
			if( 0 != io_memcmp(i,"no",2) ) {
				assign_error(error::illegal_prologue);
				return document_event();
			}
        }
        i = const_cast<char*> ( stop + 1 );
	}
	// check error in this point
	if( 0 != io_memcmp( find_first_symbol(i) ,"?>", 2) ) {
		assign_error(error::illegal_prologue);
		return document_event();
	}
	return document_event( std::move(version), std::move(encoding), standalone);
}

instruction_event event_stream_parser::parse_processing_instruction() noexcept
{
	if(state_type::event != state_.current || current_ != event_type::processing_instruction)
	{
		assign_error(error::invalid_state);
		return instruction_event();
	}
	byte_buffer buff = read_entity();
	if( !check_buffer(buff) ) {
		return instruction_event();
	}
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
	int i;
	std::size_t brackets = 1;
	do {
		i = next();
		switch(i) {
		case iEOF:
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
	} while( brackets > 0);
}

const_string event_stream_parser::read_dtd() noexcept
{
	if(state_type::dtd != state_.current) {
		assign_error(error::invalid_state);
		return const_string();
	}
	std::error_code ec;
	byte_buffer dtd = byte_buffer::allocate(ec, MID_BUFF_SIZE);
	if( ! check_buffer(dtd) ) {
		return const_string();
	}
	dtd.put(scan_buf_, sb_len(scan_buf_) );
	sb_clear(scan_buf_);
	std::size_t brackets = 1;
	int i;
	do {
		i = next();
		switch(i) {
		case iEOF:
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
		putch(dtd, static_cast<char>(i) );
	} while( brackets > 0 && !is_error() );
	dtd.flip();
	return const_string( dtd.position().cdata(), dtd.last().cdata() );
}

void event_stream_parser::skip_comment() noexcept
{
	if(state_type::comment != state_.current) {
		assign_error(error::invalid_state);
		return;
	}
	if( !sb_check(scan_buf_) ) {
		assign_error(error::illegal_commentary);
		return;
	}
	sb_clear(scan_buf_);
	static constexpr uint_fast16_t _ptrn = (HYPHEN << 8) | HYPHEN;
	uint_fast16_t i = 0;
	char c;
	do {
		c = next();
		if( is_eof(c) )
			break;
		i = (i << 8) | c;
	} while( !is_error() && _ptrn != i );
	i = next();
	if( !cheq(RIGHTB,i) ) {
		assign_error(error::illegal_commentary);
	}
}

byte_buffer event_stream_parser::read_until_double_separator(const int separator,error ec) noexcept
{
	if( !sb_check(scan_buf_) ) {
		assign_error(ec);
		return byte_buffer();
	}
	sb_clear(scan_buf_);
	std::error_code errc;
	byte_buffer buff = byte_buffer::allocate(errc, HUGE_BUFF_SIZE);
	if(!check_buffer(buff)) {
		return byte_buffer();
	}
	const uint_fast16_t pattern =  (separator << 8) | separator;
	uint_fast16_t i = 0;
	char c;
	do {
		c = next();
		if( is_eof(c) )
			break;
		i = (i << 8) | c;
		if(i == pattern)
			break;
		putch(buff, c);
	} while( !is_error() );
	buff.flip();
	if( !cheq(RIGHTB, next() ) ) {
		assign_error(ec);
		return byte_buffer();
	}
	return std::move(buff);
}

const_string event_stream_parser::read_comment() noexcept
{
	if(state_type::comment != state_.current) {
		assign_error(error::invalid_state);
		return const_string();
	}
	byte_buffer tmp( read_until_double_separator(HYPHEN, error::illegal_commentary) );
	tmp.flip();
	return const_string( tmp.position().cdata(), tmp.last().cdata()-2 );
}

const_string event_stream_parser::read_chars() noexcept
{
	if(state_type::characters != state_.current) {
		assign_error(error::invalid_state);
		return const_string();
	}
	std::error_code ec;
	byte_buffer result = byte_buffer::allocate(ec,HUGE_BUFF_SIZE);
	if(!check_buffer(result)) {
		return const_string();
	}
	if( !sb_check(scan_buf_) ) {
		assign_error(error::root_element_is_unbalanced);
		return const_string();
	}
	result.put(scan_buf_, sb_len(scan_buf_) );
	int_fast32_t i;
	constexpr int_fast32_t end_of_stream = std::char_traits<int_fast32_t>::eof();
	bool done = false;
	do {
		i = static_cast<int_fast32_t>( next() );
		switch(i) {
		case LEFTB:
			done = true;
			break;
		case RIGHTB:
			sb_clear( scan_buf_ );
			assign_error(error::illegal_chars);
			return const_string();
		case end_of_stream:
			sb_clear( scan_buf_ );
			assign_error(error::root_element_is_unbalanced);
			return const_string();
		default:
			putch(result, static_cast<char>(i) );
		}
	} while( !done && !is_error());
	sb_clear( scan_buf_ );
	if(is_error()) {
		return const_string();
	}
	sb_append( scan_buf_, static_cast<char>(LEFTB) );
	result.flip();
	return const_string( result.position().cdata(), result.last().cdata() );
}

const_string event_stream_parser::read_cdata() noexcept {
	if(state_type::cdata != state_.current) {
		assign_error(error::invalid_state);
		return const_string();
	}
	byte_buffer tmp( read_until_double_separator(SRIGHTB, error::illegal_cdata_section) );
	tmp.flip();
	return const_string( tmp.position().cdata(), tmp.last().cdata()-2 );
}

attribute event_stream_parser::extract_attribute(const char* from, std::size_t& len) noexcept
{
	len = 0;
	// skip leadin spaces, not copy them into name
	char *i = find_first_symbol(from);
	if( nullptr == i || is_one_of(*i, SOLIDUS,RIGHTB) ) {
		return attribute();
	}
	char* start = i;
	i = const_cast<char*>( io::strstr2b( start, "=\"") );
	if(nullptr == i) {
		return attribute();
	}
	std::size_t copy_len = str_size(start, i);
	cached_string name = pool_->get( start, copy_len );
	// 2 symbols
	i += 2;
	start = i;
	i = tstrchr(i, static_cast<char>(QNM) );
	if(nullptr == i) {
		assign_error(error::illegal_name);
		return attribute();
	}
	byte_buffer val;
	if( str_size(start,i) > 0) {
		if( !val.extend( str_size(start,i+1) ) ) {
			assign_error(error::out_of_memory);
			return attribute();
		}
		// normalize
		for(char *ch = start; ch != i; ch++) {
			if( between('\t','\r',*ch) ) {
				putch(val, ' ');
			} else {
				putch(val,*ch);
			}
			if(is_error())
				return attribute();
		}
		val.flip();
		++i;
	}
	len = str_size(from, i);
  	return attribute( std::move(name), const_string(val.position().cdata(), val.last().cdata()) );
}

bool event_stream_parser::validate_xml_name(const cached_string& str, bool attr) noexcept
{
#ifndef IO_NO_EXCEPTIONS
	try {
#endif // IO_NO_EXCEPTIONS
	if( validated_.end() == validated_.find( str.fast_hash() ) ) {
		error err = attr ? validate_attribute_name(str.data()) : validate_tag_name(str.data());
		if(error::ok != err ) {
			assign_error( err );
			return false;
		}
		validated_.emplace( str.fast_hash() );
		return true;
	}
#ifndef IO_NO_EXCEPTIONS
	} catch(...) {
		assign_error(error::out_of_memory);
		return false;
	}
#endif // IO_NO_EXCEPTIONS
	return true;
}

start_element_event event_stream_parser::parse_start_element() noexcept
{
	if(state_type::event != state_.current || current_ != event_type::start_element) {
		assign_error(error::invalid_state);
		return start_element_event();
	}
	byte_buffer buff = read_entity();
	if( !check_buffer(buff) )
		return start_element_event();
	bool empty_element = cheq(SOLIDUS, *(buff.last().cdata()-3));
	if( !empty_element )
		++nesting_;
	std::size_t len = 0;
	qname name = extract_qname( buff.position().cdata(), len );
	if(is_error())
		return start_element_event();
	// check name validity
	if( name.has_prefix() && !validate_xml_name( name.prefix(), false ) )
		return start_element_event();
	if( !validate_xml_name( name.local_name(), false ) )
		return start_element_event();
	start_element_event result( std::move(name), empty_element );
	if( is_error() )
		return result;
	char *left = const_cast<char*>( buff.position().cdata() + len );
	if( is_whitespace(*left) ) {
		std::size_t offset = 0;
		attribute attr = extract_attribute(left,offset);
		while(offset != 0) {
			if( !validate_xml_name( attr.name(), true ) )
				return start_element_event();
			result.add_attribute( std::move(attr) );
			left += offset;
			attr = extract_attribute(left,offset);
		}
	}
	return  result;
}

end_element_event event_stream_parser::parse_end_element() noexcept
{
	if(state_type::event != state_.current || current_ != event_type::end_element)  {
		assign_error(error::invalid_state);
		return end_element_event();
	}
	if(nesting_ == 0) {
		assign_error(error::root_element_is_unbalanced);
		return end_element_event();
	}
	--nesting_;
	if(0 == nesting_) {
		state_.current =  state_type::eod;
	}
	byte_buffer buff = read_entity();
	if( !check_buffer(buff) ) {
		return end_element_event();
	}
	std::size_t len = 0;
	qname name = extract_qname( buff.position().cdata(), len );
	if( is_error() ) {
		return end_element_event();
	}
	return end_element_event( std::move(name) );
}

void event_stream_parser::s_instruction_or_prologue() noexcept
{
	if(0 != nesting_) {
		assign_error(error::parse_error);
		return;
	}
	char st[5] = "\0\0\0\0";
	for(std::size_t i=0; i < 4; i++) {
		st[i] = next();
		if( is_eof( st[i] ) ) {
			assign_error(error::parse_error);
			return;
		}
	}
	sb_append( scan_buf_, st);
	if(is_prologue(st)) {
		if(state_type::initial != state_.current) {
			assign_error(error::parse_error);
			return;
		}
		current_ = event_type::start_document;
		state_.current = state_type::event;
	} else {
		current_ = event_type::processing_instruction;
		state_.current = state_type::event;
	}
}

void event_stream_parser::s_comment_cdata_or_dtd() noexcept
{
	char st[8];
	io_zerro_mem(st, 8);
	st[0] = next();
	st[1] = next();
	if( is_comment(st) ) {
		state_.current =  state_type::comment;
		return;
	}
	for(std::size_t i=2; i < 7; i++) {
		st[i] = next();
		if( is_eof(st[i]) ) {
			assign_error(error::root_element_is_unbalanced);
			return;
		}
	}
	sb_append(scan_buf_, st);
	if( is_cdata(st) ) {
		state_.current = state_type::cdata;
	} else if(is_doc_type(st)) {
		state_.current = state_type::dtd;
	} else {
		// TODO: unknown instruction
		assign_error(error::parse_error);
	}
}

void event_stream_parser::s_start_or_end_element() noexcept
{
	if(1 == sb_len(scan_buf_) ) {
		char ch = next();
		if( is_eof(ch) ) {
			assign_error(error::parse_error);
			return;
		}
		sb_append(scan_buf_, ch );
	}
	if( is_whitespace(scan_buf_[1]) ) {
		assign_error(error::parse_error);
		return;
	}
	if(cheq(SOLIDUS,scan_buf_[1])) {
		state_.current = state_type::event;
		current_ = event_type::end_element;
	} else {
		state_.current = state_type::event;
		current_ = event_type::start_element;
	}
}

void event_stream_parser::s_entity() noexcept
{
	switch( std::char_traits<char>::to_int_type( scan_buf_[1] ) ) {
	case QM:
		s_instruction_or_prologue();
		break;
	case EM:
		s_comment_cdata_or_dtd();
		break;
	case iEOF:
		assign_error(error::root_element_is_unbalanced);
		break;
	default:
		s_start_or_end_element();
		break;
	}
}

void event_stream_parser::scan() noexcept
{
	if( 0 == nesting_ ) {
		char ch = skip_to_symbol('<');
		if( is_eof(ch) ) {
			assign_error(error::parse_error);
			return;
		}
		sb_clear(scan_buf_);
		*scan_buf_ = std::char_traits<char>::to_char_type( LEFTB );
	}
	sb_append(scan_buf_, next() );
	switch( std::char_traits<char>::to_int_type(*scan_buf_) ) {
	case iEOF:
		state_.current = state_type::eod;
		if(0 != nesting_)
			assign_error(error::root_element_is_unbalanced);
		break;
	case LEFTB:
		s_entity();
		break;
	default:
		state_.current = state_type::characters;
	}
}

} // namesapce xml

} // namesapce io
