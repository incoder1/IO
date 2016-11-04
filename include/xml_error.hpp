/*
 * Copyright (c) 2016
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

enum class error
{
	ok,
	io_error,
	out_of_memory,
	illegal_prologue,
	illegal_chars,
	illegal_name,
	illegal_markup,
	illegal_dtd,
	Illegal_commentary,
	illegal_cdata,
	root_element_is_unbalanced,
	invalid_state,
	parse_error
};

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
	constexpr error_category() noexcept:
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

inline std::error_code make_error_code(io::xml::error errc) noexcept
{
	return std::error_code(static_cast<int>(errc), *io::xml::error_category::instance() );
}

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

inline error_code make_error_code(io::xml::error errc) noexcept
{
	return io::xml::make_error_code(errc);
}

inline std::error_condition make_error_condition(io::xml::error err) {
	return io::xml::make_error_condition(err);
}

} // namespace std

#endif // __IO_XML_ERROR_HPP_INCLUDED__
