/*
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_XML_EVENT_HPP_INCLUDED__
#define __IO_XML_EVENT_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "conststring.hpp"
#include "stringpool.hpp"

#include <vector>

namespace io {

namespace xml {

enum class event_type {
	start_document,
	start_element,
	end_element,
	processing_instruction
};

class IO_PUBLIC_SYMBOL document_event final {
public:
	constexpr document_event() noexcept:
		version_(),
		encoding_(),
		standalone_(false)
	{}
	document_event(const_string&& version,const_string&& enc, bool standalone) noexcept:
		version_( std::forward<const_string>(version) ),
		encoding_( std::forward<const_string>(enc) ),
		standalone_(standalone)
	{}
	inline const_string version() const noexcept {
		return version_;
	}
	inline const_string encoding() const noexcept {
		return encoding_;
	}
	inline bool standalone() const noexcept {
		return standalone_;
	}
private:
	const_string version_;
	const_string encoding_;
	bool standalone_;
};

class IO_PUBLIC_SYMBOL instruction_event final {
public:
	constexpr instruction_event() noexcept:
		target_(),
		data_()
	{}

	instruction_event(const_string&& target,const_string&& data) noexcept:
		target_( std::move(target) ),
		data_( std::move(data) )
	{}

	inline const_string target() const {
		return target_;
	}

	inline const_string data() const {
		return data_;
	}
private:
	const_string target_;
	const_string data_;
};

class event_stream_parser;

/// \brief XML tag attribute
class IO_PUBLIC_SYMBOL attribute
{
public:

	attribute(const attribute&) noexcept = default;
	attribute& operator=(const attribute&) noexcept = default;
	attribute(attribute&&) noexcept = default;
	attribute& operator=(attribute&&) noexcept = default;

	constexpr attribute() noexcept:
		name_(),
		value_()
	{}

	attribute(cached_string&& name,const_string&& value) noexcept;

	~attribute() noexcept
	{}

    /// Returns attribute name
    /// \return attribute name
	inline cached_string name() const {
		return name_;
	}

    /// Returns attribute value
    /// \return attribute value, in string representation
	inline const_string value() const {
		return value_;
	}

	void swap(attribute& rhs) noexcept {
		name_.swap(rhs.name_);
		value_.swap(rhs.value_);
	}

	bool operator==(const attribute& rhs) const noexcept
	{
        return name_ == rhs.name_ && value_ == rhs.value_;
	}

private:
	cached_string name_;
	const_string value_;
};

/// \brief qualified tag name
class qname final
{
public:
	qname(const qname&) noexcept = default;
	qname& operator=(const qname&) noexcept = default;
	qname(qname&&) noexcept = default;
	qname& operator=(qname&&) noexcept = default;

	constexpr qname() noexcept:
		prefix_(),
		local_name_()
	{}

	qname(cached_string&& p,cached_string&& n) noexcept:
		prefix_(std::forward<cached_string>(p)),
		local_name_(std::forward<cached_string>(n))
	{}

	~qname() noexcept = default;

	void swap(qname& other) noexcept {
		prefix_.swap(other.prefix_);
		local_name_.swap(other.local_name_);
	}

	/// Returns whether this name contains name space prefix
	/// \return whether name space prefix present
	inline bool has_prefix() const noexcept {
		return !prefix_.empty();
	}

    /// Returns name space prefix if present
	/// \return name space prefix of empty string if it is not present
	inline cached_string prefix() const  {
		return prefix_;
	}


    /// Returns tag local name
	/// \return tag local name
	inline cached_string local_name() const {
		return local_name_;
	}

private:
	cached_string prefix_;
	cached_string local_name_;
};


class IO_PUBLIC_SYMBOL start_element_event final
{
private:
	typedef std::vector<attribute, h_allocator<attribute> > attrs_storage;
public:
	typedef attrs_storage::const_iterator iterator;

	start_element_event(const start_element_event& rhs) = delete;
	start_element_event& operator=(const start_element_event& rhs) = delete;

	start_element_event() noexcept:
		name_(),
		attributes_(),
		empty_element_(false)
	{}

	~start_element_event() noexcept = default;

	start_element_event(qname&& name, bool empty_element) noexcept;

	start_element_event(start_element_event&& rhs) noexcept:
		name_( std::move(rhs.name_) ),
		attributes_( std::move(rhs.attributes_) ),
		empty_element_(rhs.empty_element_)
	{}

	start_element_event& operator=(start_element_event&& rhs) noexcept {
		name_ = rhs.name_;
		attributes_ = std::move(rhs.attributes_);
		empty_element_ = rhs.empty_element_;
		return *this;
	}

	bool add_attribute(attribute&& attr) noexcept
	{
#ifdef IO_NO_EXCEPTIONS
#   if __cplusplus > 201402L
       return attr == attributes_.emplace_back( std::forward<attribute>( attr ) );
#   else
		std::size_t ps = attributes_.size();
		attributes_.emplace_back( std::forward<attribute>( attr ) );
		return (ps+1) == attributes_.size();
#   endif // __cplusplus 14+
#else
    try {
        attributes_.emplace_back( std::forward<attribute>( attr ) );
        return true;
    } catch(...) {
        return false;
    }
#endif // IO_NO_EXCEPTIONS
	}

	inline bool empty_element() const noexcept {
		return empty_element_;
	}

	inline qname name() const noexcept {
		return name_;
	}

	inline bool has_attributes() const noexcept {
		return !attributes_.empty();
	}

	inline iterator attr_begin() const noexcept {
		return attributes_.cbegin();
	}

	inline iterator attr_end() const noexcept {
		return attributes_.cend();
	}

private:
	qname name_;
	attrs_storage attributes_;
	bool empty_element_;
};

class IO_PUBLIC_SYMBOL end_element_event final
{
public:
	end_element_event(const end_element_event&) = delete;
	end_element_event& operator=(const end_element_event&) = delete;

	constexpr end_element_event() noexcept:
		name_()
	{}

	~end_element_event() noexcept = default;

	end_element_event(qname&& cp) noexcept:
		name_( std::forward<qname>(cp) )
	{}

	end_element_event(end_element_event&& mv) noexcept:
		name_( std::move(mv.name_) )
	{}

	inline end_element_event& operator=(end_element_event&& rhs) noexcept
	{
		name_.swap( rhs.name_ );
		return *this;
	}

	inline qname name() const noexcept {
		return name_;
	}

private:
	qname name_;
};

} // namespace xml

} // namespace io


#endif // __IO_XML_EVENT_HPP_INCLUDED__
