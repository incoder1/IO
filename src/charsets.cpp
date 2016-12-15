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
#include "stdafx.hpp"
#include "charsets.hpp"

#define DECLARE_CHARSET(ID,code,name,unicode,maxchar) constexpr charset code_pages::ID = charset(code,name,unicode,maxchar);

namespace io {

// unicode character sets
DECLARE_CHARSET(UTF_8,65001,"UTF-8",4,true)
DECLARE_CHARSET(UTF_16LE,1200,"UTF-16LE",2,true)
DECLARE_CHARSET(UTF_16BE,1201,"UTF-16BE",2,true)
DECLARE_CHARSET(UTF_32LE,12000,"UTF-32LE",4,true)
DECLARE_CHARSET(UTF_32BE,12001,"UTF-32BE",4,true)
DECLARE_CHARSET(UTF_7,65000, "UTF-7", 2, true)

// one byte code pages
DECLARE_CHARSET(ISO_8859_1,28591,"ISO-8859-1",1,false) // Latin 1
DECLARE_CHARSET(ISO_8859_2,28592,"ISO-8859-2",1,false)
DECLARE_CHARSET(ISO_8859_3,28593,"ISO-8859-3",1,false)
DECLARE_CHARSET(ISO_8859_4,28594,"ISO-8859-4",1,false)
DECLARE_CHARSET(ISO_8859_5,28595,"ISO-8859-5",1,false)
DECLARE_CHARSET(ISO_8859_6,28596,"ISO-8859-6",1,false)
DECLARE_CHARSET(ISO_8859_7,28597,"ISO-8859-7",1,false)
DECLARE_CHARSET(ISO_8859_8,28598,"ISO-8859-8",1,false)
DECLARE_CHARSET(ISO_8859_9,28599,"ISO-8859-9",1,false)
DECLARE_CHARSET(ISO_8859_10,28600,"ISO-8859-10",1,false)
DECLARE_CHARSET(ISO_8859_11,28601,"ISO-8859-11",1,false)
DECLARE_CHARSET(ISO_8859_12,28602,"ISO-8859-12",1,false)
DECLARE_CHARSET(ISO_8859_13,28603,"ISO-8859-13",1,false)
DECLARE_CHARSET(ISO_8859_14,28604,"ISO-8859-14",1,false)
DECLARE_CHARSET(ISO_8859_15,28605,"ISO-8859-15",1,false)
DECLARE_CHARSET(ISO_8859_16,28606,"ISO-8859-16",1,false)
/* Cyrillic Unix */
DECLARE_CHARSET(KOI8_R,20866,"KOI8-R",1,false) // Unix Russian
DECLARE_CHARSET(KOI8_U,21866,"KOI8-U",1,false) // Unix Ukrainian
DECLARE_CHARSET(KOI8_RU,20866,"KOI8-RU",1,false) // Unix Belarusian
// Windows national code pages for the alphabet based languages
DECLARE_CHARSET(CP_1250,1250,"CP1250",1,false) // Windows version of ISO-8859-1 + additional character
DECLARE_CHARSET(CP_1251,1251,"CP1251",1,false) // Windows version of ISO-8859-5 + additional character
DECLARE_CHARSET(CP_1252,1252,"CP1252",1,false)
DECLARE_CHARSET(CP_1253,1253,"CP1253",1,false)
DECLARE_CHARSET(CP_1254,1254,"CP1254",1,false)
DECLARE_CHARSET(CP_1255,1255,"CP1255",1,false)
DECLARE_CHARSET(CP_1256,1256,"CP1256",1,false)
DECLARE_CHARSET(CP_1257,1257,"CP1257",1,false)
DECLARE_CHARSET(CP_1258,1258,"CP1258",1,false)


bool charset::operator==(const charset& rhs) const noexcept
{
	return code_ == rhs.code_;
}

bool charset::operator!=(const charset& rhs) const noexcept
{
	return code_ != rhs.code_;
}


const charset& code_pages::platform_default() noexcept {
	#ifndef __IO_WINDOWS_BACKEND__
		return UTF_16LE;
	#elif __IO_LINUX_BACKEND__
		return UTF_8;
	#else
		return UTF_8;
	#endif // __IO_WINDOWS_BACKEND__
}

const charset& code_pages::platform_current() noexcept {
	#ifdef __IO_WINDOWS_BACKEND__
		switch(::GetACP()) {
			case 1250:
				return CP_1250;
			case 1251:
				return CP_1251;
			case 1252:
				return CP_1252;
			case 1253:
				return CP_1253;
			case 1254:
				return CP_1254;
			case 1255:
				return CP_1255;
			case 1256:
				return CP_1256;
			case 1257:
				return CP_1257;
			case 1258:
				return CP_1258;
			case 12000:
				return UTF_32LE;
			case 65001:
				return UTF_8;
			default:
				return UTF_16LE;
		}
	#else
		return UTF_8;
	#endif // __IO_WINDOWS_BACKEND__
}

} // namesapce io

#undef DECLARE_CHARSET
