/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_XML_READER_HPP_INCLUDED__
#define __IO_XML_READER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <sstream>

#include <io/core/error_check.hpp>

#include "event_stream_parser.hpp"

namespace io {

namespace xml {

/// \brief XML Parser cursor API
class IO_PUBLIC_SYMBOL reader {
public:
	/// Construct an XML reader on top of StAX parser
	/// \param parser StAX parser instance
	explicit reader(s_event_stream_parser&& parser) noexcept;

	/// Drop parser to the next tag - e.g. to next start element event
	/// Check for tag self closing by start_element_event#empty_element
	/// \param ec operation error code
	/// \return start element event
	start_element_event next_tag_begin(std::error_code& ec) noexcept;

	/// Drop parser to the next tag end - e.g. to next end element event
	/// WARN. Self closed tags do not handled by this method
	/// \param ec operation error code
	/// \return end element event
	end_element_event next_tag_end(std::error_code& ec) noexcept;

	/// Checks that start element event points to the specific element name
	/// \param sev a start element event
	/// \param nmp a XML name space prefix
	/// \param local_name XML element local name
	/// \return whether start element event points to the specific element name
	bool is_element(const start_element_event& sev, const const_string& nmp, const const_string& local_name) noexcept {
		qname rhs( nmp, local_name );
		return rhs == sev.name();
	}

	/// Checks that start element event points to the specific element name
	/// \param eev an end element event
	/// \param nmp a XML name space prefix
	/// \param local_name XML element local name
	/// \return whether start element event points to the specific element name
	bool is_element(const end_element_event& eev, const const_string& nmp, const const_string& local_name) noexcept {
		qname rhs( nmp, local_name );
		return rhs == eev.name();
	}

	/// Checks that start element event points to the specific element name
	/// \param sev a start element event
	/// \param nmp a XML name space prefix
	/// \param local_name XML element local name
	/// \return whether start element event points to the specific element name
	bool is_element(const start_element_event& sev, const char* nmp, const char* local_name) noexcept {
		return is_element(sev, parser_->precache(nmp), parser_->precache(local_name) );
	}

	/// Checks that start element event points to the specific element name
	/// \param eev an end element event
	/// \param nmp a XML name space prefix
	/// \param local_name XML element local name
	/// \return whether start element event points to the specific element name
	bool is_element(const end_element_event& eev, const char* nmp, const char* local_name) noexcept {
		return is_element(eev, parser_->precache(nmp), parser_->precache(local_name) );
	}

	/// Checks that start element event points to the specific element name
	/// \param sev a start element event
	/// \param local_name XML element local name
	/// \return whether start element event points to the specific element name
	bool is_element(const start_element_event& sev, const char* local_name) noexcept {
		return is_element(sev, "", local_name);
	}

	/// Checks that start element event points to the specific element name
	/// \param eev an end element event
	/// \param local_name XML element local name
	/// \return whether start element event points to the specific element name
	bool is_element(const end_element_event& eev, const char* local_name) noexcept {
		return is_element(eev, "", local_name );
	}

	/// Read current characters e.g tag value
	/// \param ec operation error code
	/// \return tag characters
	const_string next_characters(std::error_code& ec) noexcept;

	/// Obtains next detected XML document parse state
	/// \return document state like tag start, tag end or characters
	state_type next_state() const noexcept {
		return state_;
	}

	/// Get parsers row and column XML document position
	/// \return pair where row is first element and column is last element
	std::pair<std::size_t,std::size_t> position() const noexcept {
		return std::make_pair( parser_->row(), parser_->col() );
	}

	/// Scan XML document and detects the next parse step
	/// \param ec error code indicating a parsing error, if any
	void to_next_state(std::error_code& ec) noexcept;

	/// Checks whether next XML document state is a start element event
	/// \return whether next document state is a start element event
	bool is_tag_begin_next() noexcept;

	/// Checks whether next XML document state is characters
	/// \return whether next document is characters
	bool is_characters_next() noexcept;

	/// Checks whether next XML document state is an end element event
	/// \return whether next document state is an end element event
	bool is_tag_end_next() noexcept;

	/// Drop parser to the next tag and checks whether it has expected tag name with default name space
	/// Check for tag self closing by start_element_event#empty_element
	/// \param ec operation error code
	/// \param nsp tag name name space prefix
	/// \param local_name tag name local name
	/// \return start element event
	start_element_event next_expected_tag_begin(std::error_code& ec, const char* nsp, const char* local_name) noexcept;


	/// Drop parser to the next tag end and checks whether it has expected tag name
	/// WARN. Self closed tags do not handled by this method
	/// \param ec operation error code
	/// \param nsp tag name name space prefix
	/// \param local_name tag name local name
	/// \return end element event
	end_element_event next_expected_tag_end(std::error_code& ec, const char* nsp, const char* local_name) noexcept;


	/// Drop parser to the next tag and checks whether it has expected tag name with default name space
	/// Check for tag self closing by start_element_event#empty_element
	/// \param ec operation error code
	/// \param nsp tag name name space prefix
	/// \param local_name tag name local name
	/// \return start element event
	start_element_event next_expected_tag_begin(std::error_code& ec, const char* local_name) noexcept
	{
		return next_expected_tag_begin(ec, "", local_name);
	}


	/// Drop parser to the next tag end and checks whether it has expected tag name with default name space
	/// WARN. Self closed tags do not handled by this method
	/// \param ec operation error code
	/// \param local_name tag name local name
	/// \return end element event
	end_element_event next_expected_tag_end(std::error_code& ec, const char* local_name) noexcept
	{
		return next_expected_tag_end(ec, "", local_name);
	}

private:
	inline bool parse_error(std::error_code& ec) noexcept {
		if( io_unlikely(ec) ) {
			return true;
		}
		else if( parser_->is_error() ) {
			parser_->get_last_error(ec);
			return true;
		}
		return false;
	}

private:
	s_event_stream_parser parser_;
	state_type state_;
};


} // namespace xml

template<>
class unsafe<xml::reader> {
public:
	unsafe(xml::reader&& rd) noexcept:
		rd_(std::forward<xml::reader>(rd)),
		ec_()
	{}

	unsafe(xml::s_event_stream_parser&& parser) noexcept:
		rd_(std::forward<xml::s_event_stream_parser>(parser)),
		ec_()
	{}

	xml::start_element_event next_tag_begin() {
		xml::start_element_event ret = rd_.next_tag_begin(ec_);
		check_error_code(ec_);
		return ret;
	}

	xml::end_element_event next_tag_end() {
		xml::end_element_event ret = rd_.next_tag_end(ec_);
		check_error_code(ec_);
		return ret;
	}

	const_string next_characters() {
		const_string ret = rd_.next_characters(ec_);
		check_error_code(ec_);
		return ret;
	}

	xml::state_type next_state() const noexcept {
		return rd_.next_state();
	}

	void to_next_state() {
		rd_.to_next_state(ec_);
		check_error_code(ec_);
	}

	bool is_tag_begin_next() noexcept {
		return rd_.is_tag_begin_next();
	}

	bool is_characters_next() noexcept {
		return rd_.is_characters_next();
	}

	bool is_tag_end_next() noexcept {
		return rd_.is_tag_end_next();
	}

	bool is_element(const xml::start_element_event& sev, const const_string& nmp, const const_string& local_name) noexcept {
		return rd_.is_element(sev,nmp,local_name);
	}

	bool is_element(const xml::end_element_event& eev, const const_string& nmp, const const_string& local_name) noexcept {
		return rd_.is_element(eev,nmp,local_name);
	}

	bool is_element(const xml::start_element_event& sev, const char* nmp, const char* local_name) noexcept {
		return rd_.is_element(sev, nmp, local_name);
	}

	bool is_element(const xml::end_element_event& eev, const char* nmp, const char* local_name) noexcept {
		return rd_.is_element(eev, nmp, local_name );
	}

	bool is_element(const xml::start_element_event& sev, const char* local_name) noexcept {
		return rd_.is_element(sev, local_name);
	}

	bool is_element(const xml::end_element_event& eev, const char* local_name) noexcept {
		return rd_.is_element(eev, local_name );
	}

	std::pair<std::size_t,std::size_t> position() const noexcept {
		return rd_.position();
	}

	xml::start_element_event next_expected_tag_begin(const char* nsp, const char* local_name) {
		xml::start_element_event ret = rd_.next_expected_tag_begin(ec_, nsp, local_name);
		check_error("tag start", nsp, local_name);
		return ret;
	}

	xml::start_element_event next_expected_tag_begin(const char* local_name) noexcept
	{
		return next_expected_tag_begin("",local_name);
	}

	xml::end_element_event next_expected_tag_end(const char* nsp, const char* local_name) {
		xml::end_element_event ret = rd_.next_expected_tag_end(ec_, nsp, local_name);
		check_error("tag end", nsp, local_name);
		return ret;
	}

	xml::end_element_event next_expected_tag_end(const char* local_name) noexcept
	{
		return next_expected_tag_end("", local_name);
	}
private:

	void check_error(const char* type, const char* nmp, const char* local_name)
	{
		if(ec_) {
			char emsg[ 512 ] = {'\0'};
			std::pair<std::size_t, std::size_t> pos = rd_.position();
			if(nullptr != nmp && '\0' != *nmp)
				io_snprintf(
						emsg,
						512,
						"XML validation error at [%zu,%zu] %s:%s %s expected in this state",
						pos.first,
						pos.second,
						nmp,
						local_name,
						type);
			else
				io_snprintf(
						emsg,
						512,
						"XML validation error at [%zu,%zu] %s %s expected in this state",
						pos.first,
						pos.second,
						local_name,
						type);
#ifdef IO_NO_EXCEPTIONS
		io::detail::panic( ec_.value() , emsg );
#else
		throw std::runtime_error( emsg );
#endif // IO_NO_EXCEPTIONS
		}
	}

private:
	xml::reader rd_;
	std::error_code ec_;
};

} // namespace io

#endif // __IO_XML_READER_HPP_INCLUDED__
