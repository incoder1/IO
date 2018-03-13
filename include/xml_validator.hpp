#ifndef __IO_XML_VALIDATOR_HPP_INCLUDED__
#define __IO_XML_VALIDATOR_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "object.hpp"

namespace io {

namespace xml {

namespace xs {

	class model
	{

	};

} // namespace xs

constexpr const char* W3C_XML_SCHEMA_NS_URI = "http://www.w3.org/2001/XMLSchema";
constexpr const char* XML_DTD = "http://www.w3.org/TR/REC-xml";

class IO_PUBLIC_SYMBOL schema:public object
{
	schema(const schema&) = delete;
	schema& operator=(const schema&) = delete;
public:

	//static const char* W3C_XML_SCHEMA_INSTANCE_NS_URI;
};

class IO_PUBLIC_SYMBOL validator
{
};

} // namespace xml

} // namespace io

#endif // __IO_XML_VALIDATOR_HPP_INCLUDED__
