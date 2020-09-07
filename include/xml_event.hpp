/*
 *
 * Copyright (c) 2016-2020
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

#include <algorithm>
#include <functional>
#include <set>

namespace io {

namespace xml {

/// \brief XML event masking type
enum class event_type {
	start_document,
	start_element,
	end_element,
	processing_instruction
};

/// \brief Start document event
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

	/// Returns the version of XML of this XML stream
	/// \return this XML stream version
	inline const_string version() const noexcept {
		return version_;
	}

	/// Returns the encoding style of the XML data
	/// \see io::code_pages::for_name
	/// \return encoding style
	inline const_string encoding() const noexcept {
		return encoding_;
	}

    /// Returns if this XML is standalone
    /// \return whether XML is standalone
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

	inline const_string target() const noexcept {
		return target_;
	}

	inline const_string data() const noexcept {
		return data_;
	}
private:
	const_string target_;
	const_string data_;
};

class event_stream_parser;

/// \brief qualified name
class qname final {
public:

	constexpr qname() noexcept:
		prefix_(),
		local_name_()
	{}

	qname(const cached_string& p,const cached_string& n) noexcept:
		prefix_(p),
		local_name_(n)
	{}

	qname(const qname& other) noexcept:
		qname(other.prefix_, other.local_name_)
	{}

	qname& operator=(const qname& rhs) noexcept
	{
		qname( rhs ).swap( *this );
		return *this;
	}

	qname(qname&& other) noexcept:
		prefix_( std::move( other.prefix_) ),
		local_name_( std::move( other.local_name_) )
	{}

	qname& operator=(qname&& rhs) noexcept
	{
		qname( std::forward<qname>(rhs) ).swap( *this );
		return *this;
	}

	~qname() noexcept = default;

	inline void swap(qname& other) noexcept {
		prefix_.swap( other.prefix_ );
		local_name_.swap( other.local_name_ );
	}

	/// Returns whether this name contains name space prefix
	/// \return whether name space prefix present
	inline bool has_prefix() const noexcept {
		return !prefix_.empty();
	}

	/// Returns name space prefix if present
	/// \return name space prefix of empty string if it is not present
	inline cached_string prefix() const noexcept {
		return prefix_;
	}


	/// Returns tag local name
	/// \return tag local name
	inline cached_string local_name() const noexcept {
		return local_name_;
	}

	inline bool operator==(const qname& rhs) const noexcept {
		return prefix_ == rhs.prefix_ && local_name_ == rhs.local_name_;
	}

	inline bool operator<(const qname& rhs) const {
		if( prefix_ == rhs.prefix_ )
			return local_name_ < rhs.local_name_;
		return prefix_ < rhs.prefix_;
	}

	inline bool equal(const char* prefix, const char* name) const noexcept
	{
        return prefix_.equal(prefix) && local_name_.equal(name);
	}

private:
	cached_string prefix_;
	cached_string local_name_;
};

/// \brief XML tag attribute
class IO_PUBLIC_SYMBOL attribute {
public:

	attribute(const attribute&) noexcept = default;
	attribute& operator=(const attribute&) noexcept = default;
	attribute(attribute&&) noexcept = default;
	attribute& operator=(attribute&&) noexcept = default;
	~attribute() noexcept;

	constexpr attribute() noexcept:
		name_(),
		value_()
	{}

	attribute(qname&& name,const_string&& value) noexcept:
		name_( std::forward<qname>(name) ),
		value_( std::forward<const_string>(value) )
	{}


	/// Returns attribute name
	/// \return attribute name
	inline qname name() const noexcept {
		return name_;
	}

	/// Returns attribute value
	/// \return attribute value, in string representation
	inline const_string value() const noexcept {
		return value_;
	}

	void swap(attribute& rhs) noexcept {
		name_.swap(rhs.name_);
		value_.swap(rhs.value_);
	}

	bool operator==(const attribute& rhs) const noexcept {
		return name_ == rhs.name_;
	}

private:
	qname name_;
	const_string value_;
};


class IO_PUBLIC_SYMBOL start_element_event final {
private:

	struct attr_less
	{
      	typedef attribute first_argument_type;

      	typedef attribute second_argument_type;

      	typedef bool result_type;

     	inline result_type operator()(const first_argument_type& lsh, const second_argument_type& rhs) const noexcept
     	{
     		return lsh.name() < rhs.name();
     	}
	};

	typedef std::set<attribute, attr_less, h_allocator<attribute> > attrs_storage;

public:
	typedef attrs_storage::const_iterator iterator;

	start_element_event(const start_element_event& rhs) = delete;
	start_element_event& operator=(const start_element_event& rhs) = delete;

	start_element_event() noexcept;
	~start_element_event() noexcept = default;

	start_element_event(qname&& name, bool empty_element) noexcept;
	start_element_event(start_element_event&& rhs) noexcept;

	start_element_event& operator=(start_element_event&& rhs) noexcept {
		start_element_event( std::forward<start_element_event>(rhs) ).swap( *this );
		return *this;
	}

	explicit operator bool() const noexcept {
		return !name_.local_name().empty();
	}

	bool add_attribute(attribute&& attr) noexcept;

	inline bool empty_element() const noexcept {
		return empty_element_;
	}

	inline qname name() const noexcept {
		return name_;
	}

	inline bool has_attributes() const noexcept {
		return !attributes_.empty();
	}

	// returns iterator on first attribute
	inline iterator attr_begin() const noexcept {
		return attributes_.cbegin();
	}

	// returns iterator on last attribute
	inline iterator attr_end() const noexcept {
		return attributes_.cend();
	}

	std::pair<const_string, bool> get_attribute(const char* prefix, const char* local_name) const noexcept;

	inline void swap(start_element_event& other) noexcept {
		name_.swap(other.name_);
		attributes_.swap(other.attributes_);
		std::swap(empty_element_, other.empty_element_);
	}

private:
	qname name_;
	attrs_storage attributes_;
	bool empty_element_;
};

class IO_PUBLIC_SYMBOL end_element_event final {
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

	end_element_event& operator=(end_element_event&& rhs) noexcept {
		name_.swap( rhs.name_ );
		return *this;
	}

	explicit operator bool() const noexcept {
		return !name_.local_name().empty();
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
