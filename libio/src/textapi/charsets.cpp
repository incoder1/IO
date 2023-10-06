/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "io/textapi/charset_converter.hpp"

#ifdef __IO_POSIX_BACKEND__
#	include <langinfo.h>
#endif // __

namespace io {


// code_pages

#define DECLARE_CHARSET(ID,__code,__name,__is_unicode,__maxchar) \
const charset code_pages::ID( (__code), (__name), (__is_unicode), (__maxchar) );

// UNICODE character sets
DECLARE_CHARSET(UTF_8,65001,"UTF-8",4,true)
DECLARE_CHARSET(UTF_16LE,1200,"UTF-16LE",2,true)
DECLARE_CHARSET(UTF_16BE,1201,"UTF-16BE",2,true)
DECLARE_CHARSET(UTF_32LE,12000,"UTF-32LE",4,true)
DECLARE_CHARSET(UTF_32BE,12001,"UTF-32BE",4,true)
DECLARE_CHARSET(UTF_7,65000, "UTF-7", 2, true)

// one byte code pages
DECLARE_CHARSET(ASCII,20127,"ASCII",1,false)
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
DECLARE_CHARSET(KOI8_RU,20866,"KOI8-RU",1,false) // Unix Belorussian
// Windows national code pages for the alphabet based languages
DECLARE_CHARSET(CP_1250,1250,"CP1250",1,false) // ANSI Central European; Central European (Windows)
DECLARE_CHARSET(CP_1251,1251,"CP1251",1,false) // ANSI Cyrillic; Cyrillic (Windows) (Bulgarian, Belorussian, Russian, Serbian or Ukrainian)
DECLARE_CHARSET(CP_1252,1252,"CP1252",1,false) // ANSI Latin 1; Western European (Windows)
DECLARE_CHARSET(CP_1253,1253,"CP1253",1,false) // ANSI Greek; Greek (Windows)
DECLARE_CHARSET(CP_1254,1254,"CP1254",1,false) // ANSI Turkish; Turkish (Windows)
DECLARE_CHARSET(CP_1255,1255,"CP1255",1,false) // ANSI Hebrew; Hebrew (Windows)
DECLARE_CHARSET(CP_1256,1256,"CP1256",1,false) // ANSI Arabic; Arabic (Windows)
DECLARE_CHARSET(CP_1257,1257,"CP1257",1,false) // ANSI Baltic; Baltic (Windows)
DECLARE_CHARSET(CP_1258,1258,"CP1258",1,false) // ANSI/OEM Vietnamese; Vietnamese (Windows)

#undef DECLARE_CHARSET

#ifdef __IO_POSIX_BACKEND__
#	ifdef IO_IS_LITTLE_ENDIAN
		const charset code_pages::SYSTEM_WIDE = UTF_32LE;
#	else
		const charset code_pages::SYSTEM_WIDE = UTF_32BE;
#	endif // IO_IS_LITTLE_ENDIAN
#endif // __IO_POSIX_BACKEND__


#ifdef __IO_WINDOWS_BACKEND__
const charset code_pages::SYSTEM_WIDE = UTF_16LE;
#endif // __IO_WINDOWS_BACKEND__

static constexpr std::size_t MAX_SUPPORTED = 35;

const charset* ALL_SUPPORTED[MAX_SUPPORTED] =
{
	// UNICODE
	&code_pages::UTF_8,
	&code_pages::UTF_16LE,
	&code_pages::UTF_16BE,
	&code_pages::UTF_32LE,
	&code_pages::UTF_32BE,
	&code_pages::UTF_7,
	// one byte code pages
	&code_pages::ASCII,
	&code_pages::KOI8_R,
	&code_pages::KOI8_U,
	&code_pages::KOI8_RU,
	// iso
	&code_pages::ISO_8859_1,
	&code_pages::ISO_8859_2,
	&code_pages::ISO_8859_3,
	&code_pages::ISO_8859_4,
	&code_pages::ISO_8859_5,
	&code_pages::ISO_8859_6,
	&code_pages::ISO_8859_7,
	&code_pages::ISO_8859_8,
	&code_pages::ISO_8859_9,
	&code_pages::ISO_8859_10,
	&code_pages::ISO_8859_11,
	&code_pages::ISO_8859_12,
	&code_pages::ISO_8859_13,
	&code_pages::ISO_8859_14,
	&code_pages::ISO_8859_15,
	&code_pages::ISO_8859_16,
	// windows
	&code_pages::CP_1250,
	&code_pages::CP_1251,
	&code_pages::CP_1252,
	&code_pages::CP_1253,
	&code_pages::CP_1254,
	&code_pages::CP_1255,
	&code_pages::CP_1256,
	&code_pages::CP_1257,
	&code_pages::CP_1258
};

/// Returns a character set for a name
std::pair<bool, charset> code_pages::for_name(const char* name) noexcept
{
	static constexpr std::size_t MAX_LEN = 11;
	std::pair<bool, charset> ret = std::make_pair( false, charset() );
	std::size_t len = (nullptr != name && '\0' != *name) ? io_strlen(name) : 0;
	if(len <= MAX_LEN ) {
		for(std::size_t i=0; i < MAX_SUPPORTED; i++) {
			if( 0 == io_strncasecmp(ALL_SUPPORTED[i]->name(), name, len) ) {
				ret = std::make_pair(true, *ALL_SUPPORTED[i] );
				break;
			}
		}
	}
	return ret;
}

const charset& code_pages::platform_default() noexcept
{
#if defined(__IO_WINDOWS_BACKEND__)
	return UTF_16LE;
#elif defined(__IO_POSIX_BACKED__)
	return UTF_8;
#else
	return ASCII;
#endif
}

charset code_pages::platform_current() noexcept
{
#if defined(__IO_WINDOWS_BACKEND__)
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
		case 1200:
		default:
			return UTF_16LE;
	}
#elif defined(__IO_POSIX_BACKEND__)
	std::pair<bool, charset> sres = for_name(::nl_langinfo(CODESET));
	return sres.first ? sres.second : UTF_8;
#else
	return UTF_8;
#endif
}

} // namesapce io
