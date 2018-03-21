#ifndef __IO_XML_VALIDATOR_HPP_INCLUDED__
#define __IO_XML_VALIDATOR_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "object.hpp"

namespace io {

namespace xml {

constexpr const char* XML_DTD = "http://www.w3.org/TR/REC-xml";
constexpr const char* W3C_XML_SCHEMA_INSTANCE = "http://www.w3.org/2001/XMLSchema-instance";
constexpr const char* W3C_XML_SCHEMA_NS_URI = "http://www.w3.org/2001/XMLSchema";
constexpr const char* W3C_XML_SCHEMA_NS = "http://www.w3.org/2000/xmlns/";

class grammar
{
};

class IO_PUBLIC_SYMBOL schema:public object
{
	schema(const schema&) = delete;
	schema& operator=(const schema&) = delete;
public:

};

class IO_PUBLIC_SYMBOL validator:public object
{
};

} // namespace xml

} // namespace io

#endif // __IO_XML_VALIDATOR_HPP_INCLUDED__
