#ifndef __IO_XML_EVENT_WRITER_HPP_INCLUDED__
#define __IO_XML_EVENT_WRITER_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "xml_event.hpp"
#include "text.hpp"

namespace io {

namespace xml {

class event_writer;
DECLARE_IPTR(event_writer);

struct version {
	uint8_t major;
	uint16_t minor;
};

class IO_PUBLIC_SYMBOL event_writer final: public io::object {
public:

	static s_event_writer open(std::error_code& ec,const s_write_channel& to, const document_event& prologue) noexcept;

	static s_event_writer open(std::error_code& ec,const s_write_channel& to,  const version& v,const charset& encoding, bool standalone) noexcept;

	static inline s_event_writer open(std::error_code& ec,const s_write_channel& to) noexcept {
		return open(ec, to, {1,0}, code_pages::UTF_8, false );
	}

	~event_writer() noexcept override;
	void add(std::error_code& ec,const start_element_event& ev) noexcept;
	void add(std::error_code& ec,const end_element_event& ev) noexcept;
	void add_cdata(std::error_code& ec,const char* str) noexcept;
	void add_chars(std::error_code& ec,const char* str) noexcept;
	void add_coment(std::error_code& ec,const char* str) noexcept;
private:
	explicit event_writer(writer<char>&& to, io::byte_buffer&& buff) noexcept;
	void print(std::error_code& ec,const char* str, std::size_t len) noexcept;
	void print(std::error_code& ec,const char* str) noexcept;
	void print(std::error_code& ec,const const_string& str) noexcept;
	void print(std::error_code& ec,const qname& name) noexcept;
	void overflow(std::error_code& ec) noexcept;
private:
	writer<char> to_;
	io::byte_buffer buff_;
};

} // namespace xml

} // namespace io


#endif // __IO_XML_EVENT_WRITER_HPP_INCLUDED__
