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

std::array<const charset*,35> code_pages::ALL = {

	&ASCII,

	&KOI8_R,&KOI8_U,&KOI8_RU,

	&UTF_7,&UTF_8,&UTF_16LE,&UTF_16BE,&UTF_32LE,&UTF_32BE,

	&ISO_8859_1,&ISO_8859_2,&ISO_8859_3,&ISO_8859_4,
	&ISO_8859_5,&ISO_8859_6,&ISO_8859_7,&ISO_8859_8,
	&ISO_8859_9,&ISO_8859_10,&ISO_8859_11,&ISO_8859_12,
	&ISO_8859_13,&ISO_8859_14,&ISO_8859_15,&ISO_8859_16,

	&CP_1250,&CP_1251,&CP_1252,&CP_1253,
	&CP_1254,&CP_1255,&CP_1256,&CP_1257,
	&CP_1258
};

const charset* code_pages::ascii() noexcept
{
	return &ASCII;
}

const charset* code_pages::utf7() noexcept
{
	return &UTF_7;
}

const charset* code_pages::utf8() noexcept
{
	return &UTF_8;
}

const charset* code_pages::utf16le() noexcept
{
	return &UTF_16LE;
}

const charset* code_pages::utf16be() noexcept
{
	return &UTF_16BE;
}

const charset* code_pages::utf32le() noexcept
{
	return &UTF_32LE;
}

const charset* code_pages::utf32be() noexcept
{
	return &UTF_32BE;
}

const charset* code_pages::iso_8859_1() noexcept
{
	return &ISO_8859_1;
}

const charset* code_pages::iso_8859_2() noexcept
{
	return &ISO_8859_2;
}

const charset* code_pages::iso_8859_3() noexcept
{
	return &ISO_8859_3;
}

const charset* code_pages::iso_8859_4() noexcept
{
	return &ISO_8859_4;
}

const charset* code_pages::iso_8859_5() noexcept
{
	return &ISO_8859_5;
}

const charset* code_pages::iso_8859_6() noexcept
{
	return &ISO_8859_6;
}

const charset* code_pages::iso_8859_7() noexcept
{
	return &ISO_8859_7;
}

const charset* code_pages::iso_8859_8() noexcept
{
	return &ISO_8859_8;
}

const charset* code_pages::iso_8859_9() noexcept
{
	return &ISO_8859_9;
}

const charset* code_pages::iso_8859_10() noexcept
{
	return &ISO_8859_10;
}

const charset* code_pages::iso_8859_11() noexcept
{
	return &ISO_8859_11;
}

const charset* code_pages::iso_8859_12() noexcept
{
	return &ISO_8859_12;
}

const charset* code_pages::iso_8859_13() noexcept
{
	return &ISO_8859_13;
}

const charset* code_pages::iso_8859_14() noexcept
{
	return &ISO_8859_14;
}

const charset* code_pages::iso_8859_15() noexcept
{
	return &ISO_8859_15;
}

const charset* code_pages::iso_8859_16() noexcept
{
	return &ISO_8859_16;
}

const charset* code_pages::koi8r () noexcept
{
	return &KOI8_R;
}

const charset* code_pages::koi8u () noexcept
{
	return &KOI8_U;
}

const charset* code_pages::koi8ru() noexcept
{
	return &KOI8_RU;
}

const charset* code_pages::cp1250() noexcept
{
	return &CP_1250;
}

const charset* code_pages::cp1251() noexcept
{
	return &CP_1251;
}

const charset* code_pages::cp1252() noexcept
{
	return &CP_1252;
}

const charset* code_pages::cp1253() noexcept
{
	return &CP_1253;
}

const charset* code_pages::cp1254() noexcept
{
	return &CP_1254;
}

const charset* code_pages::cp1255() noexcept
{
	return &CP_1255;
}

const charset* code_pages::cp1256() noexcept
{
	return &CP_1256;
}

const charset* code_pages::cp1257() noexcept
{
	return &CP_1257;
}

const charset* code_pages::cp1258() noexcept
{
	return &CP_1258;
}

/// Returns a character set for a name
std::pair<bool, const charset*> code_pages::for_name(const char* name) noexcept
{
	static constexpr std::size_t MAX_LEN = 11;
	std::pair<bool, const charset*> ret = std::make_pair( false, nullptr );
	std::size_t len = (nullptr != name && '\0' != *name) ? io_strlen(name) : 0;
	if(len <= MAX_LEN ) {
		auto it = std::find_if(ALL.cbegin(), ALL.cend(), [name,len] (const charset* ch) {
			return 0 == io_strncasecmp(ch->name(), name, len);
		} );
		if(ALL.cend() != it)
			ret = std::make_pair(true, *it);
	}
	return ret;
}

std::pair<bool, const charset*> code_pages::for_code(uint16_t code) noexcept
{
	std::pair<bool, const charset*> ret = std::make_pair( false, nullptr );
	auto it = std::find_if(ALL.cbegin(), ALL.cend(), [code] (const charset* ch) {
		return code == ch->code();
	} );
	if(ALL.cend() != it)
		ret = std::make_pair(true,*it);
	return ret;
}

const charset* code_pages::system_wide() noexcept
{
#if defined(__IO_WINDOWS_BACKEND__)
	return utf16le();
#elif defined(__IO_POSIX_BACKEND__)
#	ifdef IO_IS_LITTLE_ENDIAN
	return utf32le();
#	else
	return utf32be();
#	endif // IO_IS_LITTLE_ENDIAN
#endif // __IO_POSIX_BACKEND__
}

const charset* code_pages::platform_default() noexcept
{
#if defined(__IO_WINDOWS_BACKEND__)
	return utf16le();
#elif defined(__IO_POSIX_BACKED__)
	return utf8();
#else
	return ascii();
#endif
}

const charset* code_pages::platform_current() noexcept
{
#if defined(__IO_WINDOWS_BACKEND__)
	auto it = for_code( ::GetACP() );
	return it.first ? it.second : utf16le();
#elif defined(__IO_POSIX_BACKEND__)
	std::pair<bool, charset> it = for_name(::nl_langinfo(CODESET));
	return it.first ? it.second : utf8();
#else
	return utf8();
#endif
}

} // namesapce io
