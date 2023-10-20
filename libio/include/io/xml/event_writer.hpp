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
#ifndef __IO_XML_EVENT_WRITER_HPP_INCLUDED__
#define __IO_XML_EVENT_WRITER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <io/textapi/nio.hpp>

#include "event.hpp"

namespace io {

namespace xml {

class event_writer;
DECLARE_IPTR(event_writer);

struct version {
	uint8_t major;
	uint8_t minor;
};

class IO_PUBLIC_SYMBOL event_writer final: public io::object {
public:


	static s_event_writer open(std::error_code& ec,writer&& to,bool format,const document_event& prologue) noexcept;

	static s_event_writer open(std::error_code& ec,writer&& to, bool format, const version& v,const charset* encoding, bool standalone) noexcept;

	static s_event_writer open(std::error_code& ec,writer&& to) noexcept;

	static s_event_writer open(std::error_code& ec,const s_write_channel& to,bool format,const document_event& prologue) noexcept
	{
		s_event_writer ret;
		s_funnel dst = channel_funnel::create(ec, s_write_channel(to) );
		if(!ec)
			ret = open(ec, writer(dst), format, prologue);
		return ret;
	}

	static s_event_writer open(std::error_code& ec,const s_write_channel& to, bool format, const version& v,const charset* encoding, bool standalone) noexcept
	{
		s_event_writer ret;
		s_funnel dst = channel_funnel::create(ec, s_write_channel(to) );
		if(!ec)
			ret = open(ec, writer(dst), format, v, encoding, standalone);
		return ret;
	}

	static s_event_writer open(std::error_code& ec,const s_write_channel& to) noexcept
	{
		s_event_writer ret;
		s_funnel dst = channel_funnel::create(ec, s_write_channel(to) );
		if(!ec)
			ret = open(ec, writer(dst));
		return ret;
	}

	~event_writer() noexcept override;
	void add(const start_element_event& ev) noexcept;
	void add(const end_element_event& ev) noexcept;
	void add_cdata(const char* str) noexcept;
	void add_chars(const char* str) noexcept;
	void add_coment(const char* str) noexcept;
	bool has_error() const noexcept
	{
		return static_cast<bool>(ec_);
	}
	std::error_code last_error() noexcept
	{
		return ec_;
	}
private:
	explicit event_writer(bool format,writer&& to) noexcept;
	void print(char ch) noexcept;
	void print(const char* str) noexcept;
	void print(const const_string& str) noexcept;
	void print(const qname& name) noexcept;
	void independent() noexcept;
private:
	std::size_t nesting_level_;
	bool format_;
	writer to_;
	std::error_code ec_;
};

} // namespace xml

template <>
class unsafe<xml::event_writer> {
public:
	explicit unsafe(xml::s_event_writer&& ew) noexcept:
		ew_( std::forward<xml::s_event_writer>(ew) )
	{}
	void add(const xml::start_element_event& ev)
	{
		ew_->add(ev);
		check_error_code(ew_->last_error());
	}
	void add(const xml::end_element_event& ev)
	{
		ew_->add(ev);
		check_error_code(ew_->last_error());
	}
	void add_cdata(const char* str)
	{
		ew_->add_cdata(str);
		check_error_code(ew_->last_error());
	}
	void add_chars(const char* str)
	{
		ew_->add_chars(str);
		check_error_code(ew_->last_error());
	}
	void add_coment(const char* str)
	{
		ew_->add_coment(str);
		check_error_code(ew_->last_error());
	}
private:
	xml::s_event_writer ew_;
};

} // namespace io


#endif // __IO_XML_EVENT_WRITER_HPP_INCLUDED__
