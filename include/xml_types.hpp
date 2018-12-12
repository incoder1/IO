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
#ifndef __IO_XML_TYPES_HPP_INCLUDED__
#define __IO_XML_TYPES_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <algorithm>
#include <chrono>
#include <ostream>
#include <iomanip>
#include <list>
#include <vector>
#include <type_traits>
#include <utility>

#include "tuple_meta_reflect.hpp"
#include "xml_lexcast.hpp"

#define IO_XML_HAS_TO_XSD !defined(IO_NO_RTTI) && !defined(NDEBUG)

namespace io {

namespace xml {

#ifdef IO_XML_HAS_TO_XSD

/// Extract a name from typeid, in order it can be used as XSD name
template<class T>
const char* extract_xsd_type_name()
{
	char *tn = const_cast<char*>( typeid(T).name() );
	for( ; io_isdigit(*tn); ++tn);
	return tn;
}

#endif // IO_XML_HAS_TO_XSD

enum class xsd_type {
	xs_string,
	xs_boolean,
	xs_byte,
	xs_ubyte,
	xs_short,
	xs_ushort,
	xs_int,
	xs_uint,
	xs_long,
	xs_ulong,
	xs_float,
	xs_double,
	xs_deximal,
	xs_date,
	xs_time,
	xs_date_time
};

namespace detail {

// formating util
inline constexpr const char* xsd_type_name(xsd_type t)
{
	return
	    (t  == xsd_type::xs_string) ?
	    "string"
	    : (t == xsd_type::xs_long) ?
	    "long"
	    : (t == xsd_type::xs_ulong) ?
	    "unsignedLong"
	    : (t == xsd_type::xs_int) ?
	    "int"
	    : (t == xsd_type::xs_uint) ?
	    "unsignedInt"
	    : (t == xsd_type::xs_short) ?
	    "short"
	    : (t == xsd_type::xs_ushort) ?
	    "unsignedShort"
	    : (t == xsd_type::xs_byte) ?
	    "byte"
	    : (t == xsd_type::xs_ubyte) ?
	    "unsigned"
	    : (t == xsd_type::xs_float) ?
	    "float"
	    : (t == xsd_type::xs_double) ?
	    "double"
	    : (t == xsd_type::xs_deximal) ?
	    "deximal"
	    : (t == xsd_type::xs_boolean) ?
	    "boolean"
	    : (t == xsd_type::xs_date) ?
	    "date"
	    : (t == xsd_type::xs_time) ?
	    "time"
	    : (t == xsd_type::xs_date_time) ?
	    "dateTime"
	    : "unsupported_type";
}

inline void pretty_begin(std::ostream& to,uint8_t shift)
{
	for(int i=0; i < (shift-1); i++)
		to << '\t';
}

inline void pretty_end(std::ostream& to,uint8_t shift)
{
	if(shift > 0)
		to << '\n';
}

static inline void write_begin(std::ostream& to, const char* name, bool attr, uint8_t shift)
{
	if(attr)
		to << ' ' << name << "=\"";
	else {
		pretty_begin(to,shift);
		to << '<' << name << '>';
	}
}
static inline void write_end(std::ostream& to, const char* name, bool attr, uint8_t shift)
{
	if(attr)
		to << "\"";
	else {
		to << "</" << name << '>';
		detail::pretty_end(to, shift);
	}
}

IO_PUSH_IGNORE_UNUSED_PARAM

struct marshalling_functor {
public:
	constexpr marshalling_functor(std::ostream& to, uint8_t shift) noexcept:
		shift_(shift),
		to_(to)
	{}
	template<class T>
	inline constexpr void operator()(const T& t) const
	{
		t.marshal(to_, shift_);
	}
private:
	uint8_t shift_;
	std::ostream& to_;
};

// XSD generation util
#ifdef IO_XML_HAS_TO_XSD

template<class T, class is_comlex, class is_list>
struct xsd_el_gen {
};

template<class T>
struct xsd_el_gen<T, std::true_type, std::false_type> {
private:
	typedef typename T::mapped_type cplx_mpt; // complex mapped type, i.e. class, list or structure name
public:
	static void gen(const T& t, std::ostream& to)
	{
		to << "<xs:element name=\"" << t.name() << "\" type=\"" << extract_xsd_type_name<cplx_mpt>() << "\" />";
	}
};

template<class T>
struct xsd_el_gen<T, std::false_type, std::true_type> {
	static void gen(const T& t, std::ostream& to)
	{
		t.to_xsd(to);
	}
};

// Simple type XSD generator
template<class T>
struct xsd_el_gen<T, std::false_type, std::false_type> {
private:
	static constexpr bool not_namespace_or_schema(const char* what)
	{
		typedef std::char_traits<char> tr;
		return 0 != tr::compare(what,"xmlns:xsi",9) && 0 != tr::compare(what,"xsi:",4);
	}
public:
	static void gen(const T& t, std::ostream& to)
	{
		if( not_namespace_or_schema( t.name() ) )
			t.to_xsd(to);
	}
};

struct xsd_functor {
public:
	constexpr xsd_functor(std::ostream& to) noexcept:
		to_(to)
	{}
	template<class T>
	inline void operator()(const T& t) const
	{
		typedef typename T::is_complex is_complex;
		typedef typename T::is_list is_list;
		xsd_el_gen<T,is_complex,is_list>::gen(t,to_);
	}
private:
	std::ostream& to_;
};

#endif // IO_XML_HAS_TO_XSD

template<class _tuple_t>
struct marshaller {
	static inline void marshal(const _tuple_t& t, std::ostream& to,uint8_t shift)
	{
		meta::for_each( const_cast<_tuple_t&&>(t), marshalling_functor(to,shift) );
	}
};

template<>
struct marshaller< std::tuple<> > {
public:
	static inline void marshal(const std::tuple<>& t, std::ostream& to,uint8_t shift)
	{
	}
};

#ifdef IO_XML_HAS_TO_XSD
template<class _tuple_t>
struct xsd_generator {
	static inline void generate(const _tuple_t& t, std::ostream& to)
	{
		meta::for_each( const_cast<_tuple_t&&>(t), xsd_functor(to) );
	}
};

template<>
struct xsd_generator< std::tuple<> > {
public:
	static inline void generate(const std::tuple<>& t, std::ostream& to)
	{
	}
};

#endif // IO_XML_HAS_TO_XSD

template<typename V, xsd_type _xs_type, bool is_attribute>
class simple_type {
public:
	static constexpr bool is_simple = true;
	static constexpr bool attribute = is_attribute;

	typedef V mapped_type;
	typedef std::false_type is_complex;
	typedef std::false_type is_list;
	constexpr simple_type(const char* name, V v) noexcept:
		name_(name),
		v_(v)
	{}

	inline void marshal(std::ostream& to, uint8_t shift) const
	{
		write_begin(to,name_,is_attribute, shift);
		to << v_;
		write_end(to,name_,is_attribute, shift);
	}

#ifdef IO_XML_HAS_TO_XSD
	static constexpr const char* XS_TYPE = xsd_type_name(_xs_type);

	inline void to_xsd(std::ostream& to) const
	{
		to << (is_attribute ? "<xs:attribute" : "<xs:element" ) << " name=\"" << name_;
		to <<  "\" type=\"xs:" << XS_TYPE << "\" />";
	}
#endif // IO_XML_HAS_TO_XSD

	inline const char* name() const noexcept
	{
		return name_;
	}
	inline mapped_type value() const noexcept
	{
		return v_;
	}
	inline void set_value(mapped_type&& v) noexcept
	{
		v_ = std::move(v);
	}
private:
	const char* name_;
	mapped_type v_;
};

template<bool is_attribute>
class simple_type<uint8_t,xsd_type::xs_ubyte,is_attribute> {
public:
	static constexpr bool is_simple = true;
	static constexpr bool attribute = is_attribute;
	typedef uint8_t mapped_type;
	typedef std::false_type is_complex;
	typedef std::false_type is_list;

	constexpr simple_type(const char* name, uint8_t v) noexcept:
		name_(name),
		v_(v)
	{}

	void marshal(std::ostream& to, uint8_t shift) const noexcept
	{
		detail::write_begin(to,name_,is_attribute,shift);
		to << static_cast<uint16_t>(v_);
		detail::write_end(to,name_,is_attribute,shift);
	}

#ifdef IO_XML_HAS_TO_XSD

	static constexpr const char* XS_TYPE = "unsignedByte";

	inline void to_xsd(std::ostream& to) const
	{
		to << (is_attribute ? "<xs:attribute" : "<xs:element" );
		to << " name=\"" << name_ << "\" type=\"xs:unsignedByte\" />";
	}
#endif // IO_XML_HAS_TO_XSD

	inline const char* name() const noexcept
	{
		return name_;
	}
	inline uint8_t value() const noexcept
	{
		return v_;
	}
	inline void set_value(uint8_t&& v) noexcept
	{
		v_ = v;
	}
private:
	const char* name_;
	uint8_t v_;
};

template<bool is_attribute>
class simple_type<int8_t,xsd_type::xs_byte,is_attribute> {
public:
	static constexpr bool is_simple = true;
	static constexpr bool attribute = is_attribute;

	typedef int8_t mapped_type;
	typedef std::false_type is_complex;
	typedef std::false_type is_list;

	constexpr simple_type(const char* name, int8_t v) noexcept:
		name_(name),
		v_(v)
	{}
	void marshal(std::ostream& to, uint8_t shift) const noexcept
	{
		write_begin(to,name_,is_attribute,shift);
		to << static_cast<int16_t>(v_);
		write_end(to,name_,is_attribute,shift);
	}

#ifdef IO_XML_HAS_TO_XSD
	static constexpr const char* XS_TYPE = "byte";

	inline void to_xsd(std::ostream& to) const
	{
		to << (is_attribute ? "<xs:attribute" : "<xs:element" );
		to << " name=\"" << name_ << "\" type=\"xs:byte\" />";
	}
#endif // IO_XML_HAS_TO_XSD

	inline const char* name() const noexcept
	{
		return name_;
	}
	inline int8_t value() const noexcept
	{
		return v_;
	}
	inline void set_value(int8_t&& v) noexcept
	{
		v_ = v;
	}
private:
	const char* name_;
	uint8_t v_;
};

template<bool is_attribute>
class simple_type<bool,xsd_type::xs_boolean,is_attribute> {
public:
	static constexpr bool is_simple = true;
	static constexpr bool attribute = is_attribute;

	typedef bool mapped_type;
	typedef std::false_type is_complex;
	typedef std::false_type is_list;

	constexpr simple_type(const char* name, bool v) noexcept:
		name_(name),
		v_(v)
	{}
	void marshal(std::ostream& to,uint8_t shift) const noexcept
	{
		write_begin(to, name_, is_attribute, shift);
		to << (v_ ? "true" : "false") ;
		write_end(to, name_, is_attribute, shift);
	}

#ifdef IO_XML_HAS_TO_XSD
	static constexpr const char* XS_TYPE = "boolean";

	inline void to_xsd(std::ostream& to) const
	{
		to << (is_attribute ? "<xs:attribute" : "<xs:element");
		to << " name=\"" << name_  << "\" type=\"xs:boolean\" />";
	}
#endif // IO_XML_HAS_TO_XSD

	inline const char* name() const noexcept
	{
		return name_;
	}
	inline int8_t value() const noexcept
	{
		return v_;
	}
	inline void set_value(bool&& v) noexcept
	{
		v_ = v;
	}
private:
	const char* name_;
	bool v_;
};

template<bool is_attribute>
class simple_type<
	std::chrono::time_point<std::chrono::system_clock>,
	xsd_type::xs_time,
	is_attribute> {
public:
	static constexpr bool is_simple = true;
	static constexpr bool attribute = is_attribute;

	typedef std::chrono::time_point<std::chrono::system_clock> mapped_type;
	typedef std::false_type is_complex;
	typedef std::false_type is_list;

	constexpr simple_type(const char* name, const mapped_type& tm) noexcept:
		name_(name),
		v_( tm )
	{}
	void marshal(std::ostream& to,uint8_t shift) const noexcept
	{
		write_begin(to, name_, is_attribute, shift);
		std::time_t tm = std::chrono::system_clock::to_time_t(v_);
		to << std::put_time( std::gmtime(&tm), "%Y-%m-%dT%H:%M:%SZ");
		write_end(to, name_, is_attribute, shift);
	}

#ifdef IO_XML_HAS_TO_XSD
	static constexpr const char* XS_TYPE = "time";

	inline void to_xsd(std::ostream& to) const
	{
		to << (is_attribute ? "<xs:attribute" : "<xs:element" );
		to << " name=\"" << name_;
		to <<  "\" type=\"xs:time\" />";
	}
#endif // IO_XML_HAS_TO_XSD

	inline const char* name() const noexcept
	{
		return name_;
	}
	inline const mapped_type& value() const noexcept
	{
		return v_;
	}
	inline void set_value(mapped_type&& v) noexcept
	{
		v_ = std::move(v);
	}
private:
	const char* name_;
	mapped_type v_;
};

template<bool is_attribute>
class simple_type<
	std::chrono::time_point<std::chrono::system_clock>,
	xsd_type::xs_date,
	is_attribute> {
public:
	static constexpr bool is_simple = true;
	static constexpr bool attribute = is_attribute;

	typedef std::chrono::time_point<std::chrono::system_clock> mapped_type;
	typedef std::false_type is_complex;
	typedef std::false_type is_list;

	constexpr simple_type(const char* name, const mapped_type& tm) noexcept:
		name_(name),
		v_( tm )
	{}
	void marshal(std::ostream& to,uint8_t shift) const noexcept
	{
		write_begin(to, name_, is_attribute, shift);
		std::time_t tm = std::chrono::system_clock::to_time_t(v_);
		to << std::put_time( std::gmtime(&tm), "%Y-%m-%dZ");
		write_end(to, name_, is_attribute, shift);
	}

#ifdef IO_XML_HAS_TO_XSD
	static constexpr const char* XS_TYPE = "date";

	inline void to_xsd(std::ostream& to) const
	{
		to << (is_attribute ? "<xs:attribute" : "<xs:element" );
		to << " name=\"" << name_;
		to <<  "\" type=\"xs:date\" />";
	}
#endif // IO_XML_HAS_TO_XSD

	inline const char* name() const noexcept
	{
		return name_;
	}
	inline const mapped_type& value() const noexcept
	{
		return v_;
	}
	inline void set_value(mapped_type&& v) noexcept
	{
		v_ = std::move(v);
	}
private:
	const char* name_;
	mapped_type v_;
};


template<class M, class A, class E >
class complex_type_impl {
	complex_type_impl(const complex_type_impl&) = delete;
	complex_type_impl& operator=(const complex_type_impl&) = delete;
public:
	typedef M mapped_type;
	typedef A attribute_types;
	typedef E element_types;
private:

	static constexpr std::size_t ASIZE = std::tuple_size<attribute_types>::value;
	static constexpr std::size_t ESIZE = std::tuple_size<element_types>::value;

	inline void write_begin_el(std::ostream& to,uint8_t shift) const
	{
		pretty_begin(to,shift);
		to << '<' << name_ ;
	}

	inline void write_end_el(std::ostream& to,uint8_t shift) const
	{
		pretty_begin(to,shift);
		to << "</" << name_ << '>';
		pretty_end(to,shift);
	}

	inline friend void intrusive_ptr_add_ref(complex_type_impl* const i) noexcept
	{
		i->ref_count_.fetch_add(1, std::memory_order_relaxed);
	}

	friend void intrusive_ptr_release(complex_type_impl* const i) noexcept
	{
		if(1 == i->ref_count_.fetch_sub(1, std::memory_order_acquire) ) {
			std::atomic_thread_fence( std::memory_order_release);
			delete i;
		}
	}

public:

	~complex_type_impl() noexcept = default;

	complex_type_impl(const char* name,attribute_types&& att,element_types&& el) noexcept:
		name_(name),
		attrs_(  std::forward<attribute_types> (att) ),
		elements_( std::forward<element_types> (el) ),
		ref_count_(0)
	{}

	void marshal(std::ostream& to,const uint8_t shift) const
	{
		write_begin_el(to, shift );
		if(ASIZE > 0)
			marshaller<attribute_types>::marshal( std::move(attrs_), to, shift);
		if( ESIZE > 0 ) {
			to << '>';
			pretty_end(to,shift);
			uint8_t chld_sh = (shift > 0) ? shift+1 : 0;
			marshaller<element_types>::marshal( std::move(elements_), to, chld_sh);
			write_end_el(to, shift);
		} else {
			to << "/>";
			pretty_end(to,shift);
		}
	}

#ifdef IO_XML_HAS_TO_XSD

	// workaround for list
	static constexpr const char* XS_TYPE = "dummy";

	void to_xsd(std::ostream& to) const
	{
		to << "<xs:complexType name=\"" << extract_xsd_type_name<mapped_type>() << "\">";
		if( ESIZE > 0 ) {
			to << "<xs:sequence>";
			xsd_generator<element_types>::generate( elements_, to);
			to << "</xs:sequence>";
		}
		if( ASIZE > 0 )
			xsd_generator<attribute_types>::generate( attrs_, to);
		to << "</xs:complexType>";
	}
#endif // IO_XML_HAS_TO_XSD

	inline const char* name() const
	{
		return name_;
	}

	inline attribute_types attributes() const
	{
		return attrs_;
	}

	inline element_types elements() const
	{
		return elements_;
	}
private:
	const char* name_;
	attribute_types attrs_;
	element_types elements_;
	std::atomic_size_t ref_count_;
};


template<class Container>
class list_type_impl {
	list_type_impl(const list_type_impl&) = delete;
	list_type_impl& operator=(const list_type_impl&) = delete;

public:
	typedef Container container;
	typedef typename Container::value_type element_type;
	typedef typename element_type::mapped_type mapped_type;
public:

	typedef typename container::const_iterator iterator;

	~list_type_impl() noexcept = default;

	list_type_impl(uint32_t min, uint32_t max, const char* wr):
		min_ocurs_(min),
		max_ocurs_(max),
		wrapper_name_(wr),
		has_wrapper_(false),
		cont_(),
		ref_count_(0)
	{
		has_wrapper_ = nullptr != wrapper_name_ && std::char_traits<char>::length(wrapper_name_) > 0;
	}

private:

	template<class MI,class _compl>
	struct mapper {
	};

	template<class MI>
	struct mapper<MI,std::false_type> {
		static inline void map_to(const char* name, const MI& b, const MI& e,container& to)
		{
			MI it = b;
			while(it != e) {
				to.emplace_back( name, false, *it  );
				++it;
			}
		}
	};

	template<class MI>
	struct mapper<MI,std::true_type> {
		static inline void map_to(const char* name,const MI& b,const MI& e,container& to)
		{
			MI it = b;
			while(it != e) {
				to.emplace_back( it->to_xml_type() );
				++it;
			}
		}
	};

	template<class MC, class _is_complex_type>
	struct unmapper {
	};

	// simple type unmapper
	template<class MC>
	struct unmapper<MC, std::false_type > {
		static inline void unmap_to(iterator b, iterator e, MC& to)
		{
			iterator it = b;
			while( it != e) {
				to.emplace_back( it->value() );
				++it;
			}
		}
	};

	// complex type unmapper
	template<class MC>
	struct unmapper<MC, std::true_type> {
		static inline void unmap_to(iterator b, iterator e, MC& to)
		{
			iterator it = b;
			while( it != e) {
				//element_type el = *it;
				to.emplace_back( mapped_type::from_xml_type(*it) );
				++it;
			}
		}
	};

	inline friend void intrusive_ptr_add_ref(list_type_impl* const i) noexcept
	{
		i->ref_count_.fetch_add(1, std::memory_order_relaxed);
	}

	friend void intrusive_ptr_release(list_type_impl* const i) noexcept
	{
		if(1 == i->ref_count_.fetch_sub(1, std::memory_order_acquire) ) {
			std::atomic_thread_fence( std::memory_order_release);
			delete i;
		}
	}
public:

	inline void add_element(element_type&& e)
	{
		cont_.emplace_back( std::forward<element_type>(e) );
	}

	template<class _Iterator>
	void add_elements(const char* name, _Iterator& b, const _Iterator& e)
	{
		typedef typename element_type::is_complex complex_t;
		typedef mapper<_Iterator,complex_t> mpr;
		mpr::map_to(name, b, e, cont_ );
	}

	template<class _STL_container>
	void unmap(_STL_container& to) const
	{
		typedef typename element_type::is_complex complex_t;
		typedef unmapper<_STL_container,complex_t> umpr;
		umpr::unmap_to( cont_.cbegin(), cont_.cend(), to);
	}

	void marshal(std::ostream& to, uint8_t shift) const
	{
		uint8_t embd_shift = shift >= 1 ? shift : 0;
		if(has_wrapper_) {
			pretty_begin(to,shift);
			to << '<' << wrapper_name_ << '>';
			pretty_end(to,shift);
			if(embd_shift > 0 ) ++embd_shift;
		}


		for(iterator it = cont_.cbegin(); it != cont_.cend(); ++it)
			it->marshal(to, embd_shift);

		if(has_wrapper_) {
			pretty_begin(to,shift);
			to << "</" << wrapper_name_ << '>';
			pretty_end(to,shift);
		}
	}

#ifdef IO_XML_HAS_TO_XSD
private:

	static bool holding_complex()
	{
		typedef typename element_type::is_complex is_cmp;
		return std::is_same<is_cmp, std::true_type >::value;
	}

	static const char* holding_type()
	{
		if( holding_complex() )
			return extract_xsd_type_name<mapped_type>();
		return element_type::XS_TYPE;
	}

	inline const char* holding_element() const
	{
		assert( !cont_.empty() );
		return cont_.begin()->name();
	}

	void gen_xsd_type_with_wrapper(std::ostream& to) const
	{
		to << "<xs:element name=\"" << wrapper_name_ << "\">";
		to << "<xs:complexType><xs:sequence>";
		to << "<xs:element name=\"" << holding_element() << "\" ";
		to << "type=\"" << holding_type() << "\" ";
		to << "minOccurs=\"" << min_ocurs_ << "\" ";
		to << "maxOccurs=\"";
		if(max_ocurs_ > 0)
			to << max_ocurs_ << "\" ";
		else
			to<< "unbounded\"";
		to << "/></xs:sequence></xs:complexType></xs:element>";
	}

	void make_xs_ref(std::ostream& to) const
	{
		to << "<xs:element name=\"" << holding_element() << "\" ";
		to << "type=\"" << holding_type() << "\" ";
		to << "minOccurs=\"" << min_ocurs_ << "\" ";
		to << "maxOccurs=\"";
		if(max_ocurs_ > 0)
			to << max_ocurs_ << "\" ";
		else
			to<< "unbounded\"";
		to << "/>";
	}

public:

	void to_xsd(std::ostream& to) const
	{
		if(has_wrapper_)
			gen_xsd_type_with_wrapper(to);
		else
			make_xs_ref(to);
	}
#endif // IO_XML_HAS_TO_XSD

	inline iterator begin() const
	{
		return cont_.cbegin();
	}
	inline iterator end() const
	{
		return cont_.cend();
	}
	inline std::size_t size() const
	{
		return cont_.size();
	}
private:
	uint32_t min_ocurs_;
	uint32_t max_ocurs_;
	const char* wrapper_name_;
	bool has_wrapper_;
	container cont_;
	std::atomic_size_t ref_count_;
};

IO_POP_IGNORE_UNUSED_PARAM

} // namespace detail


/// XML primitive integer attribute mapped to uint8_t
typedef detail::simple_type<uint8_t,xsd_type::xs_ubyte,true>    byte_attribute;
/// XML primitive integer element mapped to uint8_t
typedef detail::simple_type<uint8_t,xsd_type::xs_ubyte,false>   byte_element;

/// XML primitive integer attibute mapped to int8_t
typedef detail::simple_type<int8_t,xsd_type::xs_byte,true>     small_attribute;
/// XML primitive integer element mapped to int8_t
typedef detail::simple_type<int8_t,xsd_type::xs_byte,false>    small_element;

/// XML primitive integer attribute mapped to uint16_t
typedef detail::simple_type<uint16_t,xsd_type::xs_ushort,true>   word_attribute;
/// XML primitive integer element mapped to uint16_t
typedef detail::simple_type<uint16_t,xsd_type::xs_ushort,false>  word_element;

/// XML primitive integer attribute mapped to int16_t
typedef detail::simple_type<int16_t,xsd_type::xs_short,true>   short_attribute;
/// XML primitive integer element mapped to int16_t
typedef detail::simple_type<int16_t,xsd_type::xs_short,false>  short_element;

/// XML primitive integer attribute mapped to uint32_t
typedef detail::simple_type<uint32_t,xsd_type::xs_uint,true>   dword_attribute;
/// XML primitive integer element mapped to uint32_t
typedef detail::simple_type<uint32_t,xsd_type::xs_uint,false>  dword_element;

/// XML primitive integer attribute mapped to int32_t
typedef detail::simple_type<int32_t,xsd_type::xs_int,true>    int_attribute;
/// XML primitive integer element mapped to int32_t
typedef detail::simple_type<int32_t,xsd_type::xs_int,false>   int_element;


/// XML primitive integer attribute mapped to uint64_t
typedef detail::simple_type<uint64_t,xsd_type::xs_ulong,true>   qword_attribute;
/// XML primitive integer element mapped to uint64_t
typedef detail::simple_type<uint64_t,xsd_type::xs_ulong,false>  qword_element;

/// XML primitive integer attribute mapped to int64_t
typedef detail::simple_type<int64_t,xsd_type::xs_long,true>    llong_attribute;
/// XML primitive integer element mapped to int64_t
typedef detail::simple_type<int64_t,xsd_type::xs_long,false>    llong_element;

/// XML primitive decimal attribute mapped to float
typedef detail::simple_type<float,xsd_type::xs_float,true>   float_attribute;
/// XML primitive decimal element mapped to float
typedef detail::simple_type<float,xsd_type::xs_float,false>  float_element;

/// XML primitive decimal attribute mapped to double
typedef detail::simple_type<double,xsd_type::xs_double,true>  double_attribute;
/// XML primitive decimal element mapped to double
typedef detail::simple_type<double,xsd_type::xs_double,false> double_element;

/// XML primitive decimal attribute mapped to long double
typedef detail::simple_type<long double,xsd_type::xs_deximal,true>  long_double_attribute;
/// XML primitive decimal element mapped to long double
typedef detail::simple_type<long double,xsd_type::xs_deximal,false> long_double_element;

/// XML primitive boolean attribute mapped to bool
typedef detail::simple_type<bool,xsd_type::xs_boolean,true>   bool_attribute;
/// XML primitive boolean element mapped to bool
typedef detail::simple_type<bool,xsd_type::xs_boolean,false>  bool_element;

/// XML primitive boolean attribute mapped to const char* string in UTF-8
typedef detail::simple_type<std::string,xsd_type::xs_string,true>   string_attribute;
/// XML primitive boolean element mapped to const char* string in UTF-8
typedef detail::simple_type<std::string,xsd_type::xs_string,false>  string_element;

/// XML primitive time attribute mapped to std::chrono::time_point<std::chrono::system_clock>
typedef detail::simple_type<
		std::chrono::time_point<std::chrono::system_clock>,
		xsd_type::xs_time,true>   time_attribute;
/// XML primitive time element mapped to std::chrono::time_point<std::chrono::system_clock>
typedef detail::simple_type<
		std::chrono::time_point<std::chrono::system_clock>,
		xsd_type::xs_time,false>  time_element;

/// XML primitive date attribute mapped to std::chrono::time_point<std::chrono::system_clock>
typedef detail::simple_type<
		std::chrono::time_point<std::chrono::system_clock>,
		xsd_type::xs_date,true>   date_attribute;
/// XML primitive date element mapped to std::chrono::time_point<std::chrono::system_clock>
typedef detail::simple_type<
	std::chrono::time_point<std::chrono::system_clock>,
	xsd_type::xs_date,false>  date_element;

/// XML/XSD complex type template
/// \param M mapped type - i.e. domain object class or temp stub
/// \param A std::typle stores argument XML/XSD types
/// \param E std::typle stores embeded elements XML/XSD types
template<class M, class A, class E>
class complex_type {
private:
	typedef detail::complex_type_impl<M,A,E> impl_t;
	typedef boost::intrusive_ptr<impl_t> s_impl_t;
public:
	typedef M mapped_type;
	typedef A attribute_types;
	typedef E element_types;

	typedef std::true_type   is_complex;
	typedef std::false_type  is_list;

	/// Constructs new XML/XSD complex type
	/// \param att std::typle stores argument XML/XSD types
	/// \param el std::typle stores embeded elements XML/XSD types
	complex_type(const char* name, attribute_types&& att,element_types&& el) noexcept:
		self_( new impl_t(name, std::forward<A>(att), std::forward<E>(el) ) )
	{}

	/// Marshal this object into output stream recursive
	/// \param to output stream to marshal into
	/// \param shift pretty print shift, 0 - no pretty print i.e. single line
	void marshal(std::ostream& to,const uint8_t shift) const
	{
		self_->marshal(to,shift);
	}

#ifdef IO_XML_HAS_TO_XSD

	static constexpr const char* XS_TYPE = impl_t::XS_TYPE;

	/// Recursive generate XSD from this XML type
	/// \param to output stream to marshal
	void to_xsd(std::ostream& to)
	{
		self_->to_xsd(to);
	}
#endif // IO_XML_HAS_TO_XSD

	/// Returns specified element name
	/// \return element name
	inline const char* name() const
	{
		return self_->name();
	}

	/// Returns attributes XML/XSD types tuple
	/// \return attributes tuple
	inline attribute_types attributes() const
	{
		return self_->attributes();
	}

	/// Returns element XML/XSD types tuple
	/// \return elements type tuple
	inline element_types elements() const
	{
		return self_->elements();
	}

private:
	s_impl_t self_;
};


template<class E>
class list_type {
private:
	typedef detail::list_type_impl< std::list<E> > impl_t;
	typedef boost::intrusive_ptr<impl_t> s_impl_t;
public:
	typedef typename E::mapped_type mapped_type;
	typedef typename impl_t::iterator iterator;

	typedef typename std::vector<mapped_type> mapped_vector_t;
	typedef typename mapped_vector_t::const_iterator mapped_vector_cit;

	typedef typename std::list<mapped_type> mapped_list_t;
	typedef typename mapped_list_t::const_iterator mapped_list_cit;

	typedef std::false_type  is_complex;
	typedef std::true_type   is_list;

	list_type(uint8_t min, uint8_t max, const char* wrapper):
		self_(new impl_t(min,max,wrapper) )
	{}

	list_type(const char* wrapper):
		list_type(1,0,wrapper)
	{}

	list_type():
		list_type(1,0,nullptr)
	{}

	inline void add_element(E&& e)
	{
		self_->add_element( std::forward<E>(e) );
	}

	inline void add_elements(const char* name, mapped_vector_cit b, mapped_vector_cit e)
	{
		self_->add_elements(name,b,e);
	}

	inline void add_elements(const char* name, mapped_list_cit b, mapped_list_cit e)
	{
		self_->add_elements(name,b,e);
	}

	inline void unmap(mapped_vector_t& to) const
	{
		self_->unmap(to);
	}

	inline void unmap(mapped_list_t& to) const
	{
		self_->unmap(to);
	}

	inline iterator begin() const
	{
		return self_->begin();
	}

	inline iterator end() const
	{
		return self_->end();
	}

	inline std::size_t size() const
	{
		return self_->size();
	}

	inline void marshal(std::ostream& to, uint8_t shift) const
	{
		self_->marshal(to,shift);
	}

#ifdef IO_XML_HAS_TO_XSD
	inline void to_xsd(std::ostream& to) const
	{
		self_->to_xsd(to);
	}
#endif // IO_XML_HAS_TO_XSD

private:
	s_impl_t self_;
};

} // namespace xml

} // namespace io

#endif // __IO_XML_TYPES_HPP_INCLUDED__
