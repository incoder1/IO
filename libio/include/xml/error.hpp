/*
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_XML_ERROR_HPP_INCLUDED__
#define __IO_XML_ERROR_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace xml {

/// \brief XML parsing error codes enumeration
enum class error
{
	ok, //!< No errors
	io_error, //!< XML source input/output error (disk, socket etc)
	out_of_memory, //!< Not enough memory to complete operation
	illegal_attribute, //!< Element attribute is illegal
	illegal_prologue, //!< XML prologue is illegal
	illegal_chars, //!< Illegal characters for XML document
	illegal_name, //!< XML element name is illegal i.e. <xmlnode> or <%134node> etc
	illegal_markup, ///!< Illegal XML markup i.e. <node<node arttr="</> etc
	illegal_dtd, //!< Illegal DTD declaration
	illegal_commentary, //!Illegal XML commentary i.e. <!-- Lorem -- ipsum -->
	illegal_cdata_section, //!Illegal <![CDATA[ <text> ]]> section
	root_element_is_unbalanced, //! An element is unbalanced i.e. <tag_0> <tag_1> <tag_0/>
	invalid_state //! Logical error indicating an attempt to parse XML node when it's not allowed
};

/// \brief Error category for XML specific std::error_code
class IO_PUBLIC_SYMBOL error_category final: public std::error_category {
private:

	friend inline std::error_code make_error_code(error errc) noexcept;
	friend inline std::error_condition make_error_condition(error err);

	static inline const error_category* instance()
	{
		static error_category _instance;
		return &_instance;
	}
	const char* cstr_message(int err_code) const noexcept;
public:
	error_category() noexcept:
		std::error_category()
	{}
	virtual ~error_category() = default;
	virtual const char* name() const noexcept override;
	virtual std::error_condition default_error_condition(int ec) const noexcept override;
	virtual bool equivalent (const std::error_code& code, int condition) const noexcept override;

	virtual std::string message(int err_code) const override
	{
		return std::string( cstr_message(err_code) );
	}
};

/// Creates std::error_code for the error enumeration
/// \param errc error enumeration value
inline std::error_code make_error_code(io::xml::error errc) noexcept
{
	return std::error_code(static_cast<int>(errc), *io::xml::error_category::instance() );
}

/// Creates std::error_condition for the error enumeration
/// \param errc error enumeration value
inline std::error_condition make_error_condition(io::xml::error err)
{
	return io::xml::error_category::instance()->default_error_condition( static_cast<int>(err) );
}

} // namespace xml

} // namespace io

namespace std {

template<>
struct is_error_condition_enum<io::xml::error> : public true_type
{};

// implementation to standard name space
using io::xml::make_error_code;
using io::xml::make_error_condition;

} // namespace std

#endif // __IO_XML_ERROR_HPP_INCLUDED__
