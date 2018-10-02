#ifndef __IO_XML_VALIDATOR_HPP_INCLUDED__
#define __IO_XML_VALIDATOR_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <vector>

#include "object.hpp"
#include "conststring.hpp"

namespace io {

namespace xs {

constexpr const char* XML_DTD = "http://www.w3.org/TR/REC-xml";
constexpr const char* W3C_XML_SCHEMA_INSTANCE = "http://www.w3.org/2001/XMLSchema-instance";
constexpr const char* W3C_XML_SCHEMA_NS_URI = "http://www.w3.org/2001/XMLSchema";
constexpr const char* W3C_XML_SCHEMA_NS = "http://www.w3.org/2000/xmlns/";

} // namespace xs

} // namespace io

#endif // __IO_XML_VALIDATOR_HPP_INCLUDED__
