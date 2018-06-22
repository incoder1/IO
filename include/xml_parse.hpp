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
#ifndef __IO_XML_PARSE_HPP_INCLUDED__
#define __IO_XML_PARSE_HPP_INCLUDED__

#include "config.hpp"
#include "xml_source.hpp"
#include "xml_event.hpp"

#include <unordered_set>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace xml {

/// \brief XML parsing states constants
enum class state_type {
	/// initial state
	initial,
	/// end of document state, also occurs in case of parsing error including out of memory error
	eod,
	/// Parser detected commentary
	comment,
	/// Parser detected characters
	characters,
	/// Parser detected <!CDATA[[]]> section
	cdata,
	/// Parser detected DTD declaration (embedded or link to external)
	dtd,
	/// Parser detected an XML entity (i.e. tag, prologue or processing instruction)
	event
};


class event_stream_parser;

DECLARE_IPTR(event_stream_parser);

/// \brief XML streaming API for XML parsing (StAX) LL parser
/// Unlike SAX/SAX2 this is controlled parser, i.e. no any callbacks required
/// You can build your own SAX/DOM implementation on top of it.
/// NOTE! This is XML parser only, not a full XML processor i.e. no XML validation except for incorrect XML markup.
/// Limitations:
/// <ul>
/// <li>DTD including embedded and external is ignored, but can be extracted for later usage</li>
/// <li>XML attribute values are always interpreted as #CDATA attribute, with white spaces normalization
///  e.g. &nbsp; must be converted manually to ' ' </li>
/// <li>No XML validation neither DTD neither XSD schema since parsing</li>
/// <ul>
class IO_PUBLIC_SYMBOL event_stream_parser:public object {
private:

	/// \brief XML parser state
	struct state {
		/// Current XML parsing error code
		error ec;
		/// Current XML parsing state
		state_type current;
		constexpr state(error errcd, state_type state) noexcept:
			ec(errcd),
			current(state)
		{}
		constexpr state() noexcept:
			state(error::ok,state_type::initial)
		{}
	};

	// we need 9 bytes only, but will use 16 for better align
	static constexpr std::size_t MAX_SCAN_BUFF_SIZE = 16;

	typedef std::unordered_set<
	std::size_t,
		std::hash<std::size_t>,
		std::equal_to<std::size_t>,
		io::h_allocator<std::size_t> > validated_set;

	friend class nobadalloc<event_stream_parser>;
	event_stream_parser(const event_stream_parser&) = delete;
	event_stream_parser& operator=(const event_stream_parser&) = delete;
	event_stream_parser(const s_source& src, s_string_pool&& pool) noexcept;
public:

	/// Constructs new XML parser from an XML source
	/// \param ec contains system error code when parser can not be constructed,
	/// 		for example in case of out of memory or nullptr pointed source
	/// \param source an XML source data
	static s_event_stream_parser open(std::error_code& ec,s_source&& src) noexcept;


	/// Destroy parser and releases associated resources
	virtual ~event_stream_parser() noexcept override;

	/// Scan XML source from current position to find next XML entity or characters
	/// \return parser state after scanning
	state_type scan_next() noexcept;

	/// Checks parser in error state
	/// \return true if parser in error state, false otherwise
	inline bool is_error() const noexcept {
		return error::ok != state_.ec;
	}

	inline void get_last_error(std::error_code& ec) const noexcept {
		ec = std::make_error_code( state_.ec );
	}

	/// Returns current XML event, if any
	/// \return current XML event
	inline event_type current_event() const noexcept {
		return current_;
	}

	/// Current XML source line
	/// \return source line
	inline std::size_t row() const noexcept {
		return src_->row();
	}

	/// Current XML source character column
	/// \return character column
	inline std::size_t col() const noexcept {
		return src_->col();
	}

	/// Parse XML prologue declaration into document_event structure
	/// \return extracted document_event
	document_event parse_start_doc() noexcept;

	/// Parse XML processing instruction declaration into instruction_event structure
	/// \return extracted instruction_event
	instruction_event parse_processing_instruction() noexcept;

	/// Parse tag start declaration into start_element_event structure
	/// \return extracted start_element_event
	start_element_event parse_start_element() noexcept;

	/// Parse tag close declaration into end_element_event structure
	/// \return extracted end_element_event
	end_element_event parse_end_element() noexcept;

	/// Extracts raw unformated XML DTD declaration into memory buffer
	/// \return extracted DTD
	const_string read_dtd() noexcept;

	/// Skips XML DTD declaration
	void skip_dtd() noexcept;

	/// Extracts raw unformated XML commentary into memory buffer
	/// \return extracted commentary body
	const_string read_comment() noexcept;

	/// Skips XML commentary
	void skip_comment() noexcept;

	/// Extracts normalized XML characters, i.e. tag body
	const_string read_chars() noexcept;

	/// Skip characters until next tag declaration e.g. '>  <next-tag>' or 'text <![CDATA[ some data]]>'
	void skip_chars() noexcept;

	/// Extract raw XML characters declared in <!CDATA[]]> section
	/// \return CDATA section content
	const_string read_cdata() noexcept;

private:

	void scan() noexcept;
	void s_instruction_or_prologue() noexcept;
	void s_comment_cdata_or_dtd() noexcept;
	void s_characters_or_eod() noexcept;
	void s_entity() noexcept;

	// assign an error
	inline void assign_error(xml::error ec) noexcept;

	// put a byte into buffer
	// extend when needed or assign error when no memory left
	inline void putch(byte_buffer& buf, char ch) noexcept;

	//char skip_to_symbol(char symbol) noexcept;
	byte_buffer read_entity() noexcept;
	byte_buffer read_until_double_separator(int separator,error ec) noexcept;

	qname extract_qname(const char* from, std::size_t& len) noexcept;
	attribute extract_attribute(const char* from, std::size_t& len) noexcept;
	bool validate_xml_name(const cached_string& str, bool attr) noexcept;

	inline char next() noexcept;

	static __forceinline bool is_eof(char ch) noexcept {
		return !std::char_traits<char>::not_eof(ch);
	}

	static __forceinline bool sb_check(const char* sb) noexcept {
		return nullptr == io_memchr(sb, EOF, MAX_SCAN_BUFF_SIZE);
	}

	static __forceinline void sb_clear(const char* sb) noexcept {
		io_zerro_mem( const_cast<char*>(sb), MAX_SCAN_BUFF_SIZE);
	}

	static __forceinline std::size_t sb_len(const char *sb) noexcept {
		return io_strlen(sb);
	}

	static __forceinline bool sb_empty(const char *sb) noexcept {
		return '\0' == *sb;
	}

	static __forceinline void sb_append(const char* sb,const char c) noexcept {
		const_cast<char*>(sb)[ sb_len(sb) ] = c;
	}

	static __forceinline void sb_append(const char* sb,const char* str) noexcept {
		io_strcat( const_cast<char*>(sb), str);
	}

private:
	s_source src_;
	state state_;
	event_type current_;
	s_string_pool pool_;
	validated_set validated_;
	std::size_t nesting_;
	char scan_buf_[MAX_SCAN_BUFF_SIZE];
};



} // namesapce xml

} //  namesapce io


#endif // __IO_XML_PARSE_HPP_INCLUDED__
