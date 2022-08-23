/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"

#include "char_cast.hpp"


#if defined(__WIN32) && ( defined(__MINGW32__) || defined(__MINGW64__) )

// TODO: review implementation found at
// https://github.com/msys2/MINGW-packages/blob/master/mingw-w64-libkml/strptime.c

typedef unsigned char u_char;
typedef unsigned int uint;
typedef unsigned __int64 uint64_t;

#define	_ctloc(x)		(_CurrentTimeLocale->x)

/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E			0x01
#define ALT_O			0x02
#define	LEGAL_ALT(x)		{ if (alt_format & ~(x)) return NULL; }

#ifndef TIME_MAX
#define TIME_MAX	INT64_MAX
#endif


static int TM_YEAR_BASE = 1900;
static char gmt[] = { "GMT" };
static char utc[] = { "UTC" };

/* RFC-822/RFC-2822 */
static const char * const nast[5] = {
	"EST",
	"CST",
	"MST",
	"PST",
	"\0\0\0"
};

static const char * const nadt[5] = {
	"EDT",
	"CDT",
	"MDT",
	"PDT",
	"\0\0\0"
};

static const char * const am_pm[2] = {
	"am",
	"pm"
};

static const char * const day[7] = {
	"sunday",
	"monday",
	"tuesday",
	"wednesday",
	"thursday",
	"friday",
	"saturday"
};

static const char * const abday[7] = {
	"sun",
	"mon",
	"tue",
	"wed",
	"thu",
	"fri",
	"sat"
};

static const char * const mon[12] = {
	"january",
	"february",
	"march",
	"april",
	"may",
	"june",
	"july",
	"august",
	"september",
	"october",
	"november",
	"december"
};

static const char * const abmon[12] = {
	"jan",
	"feb",
	"mar",
	"apr",
	"may",
	"jun",
	"jul",
	"aug",
	"sep",
	"oct",
	"nov",
	"dec"
};


static const u_char * conv_num(const unsigned char *buf, int *dest, unsigned int llim, unsigned int ulim)
{
	unsigned int result = 0;
	unsigned char ch;

	/* The limit also determines the number of valid digits. */
	unsigned int rulim = ulim;

	ch = *buf;
	if (ch < '0' || ch > '9')
		return nullptr;

	do {
		result *= 10;
		result += ch - '0';
		rulim /= 10;
		ch = *++buf;
	}
	while ((result * 10 <= ulim) && rulim && ch >= '0' && ch <= '9');

	if (result < llim || result > ulim)
		return nullptr;

	*dest = result;
	return buf;
}

static const u_char *find_string(const u_char *bp, int *tgt, const char * const *n1, const char * const *n2, int c)
{
	/* check full name - then abbreviated ones */
	for (; n1 != nullptr; n1 = n2, n2 = nullptr) {
		for (int i = 0; i < c; i++, n1++) {
			size_t len = io_strlen(*n1);
			if (0 == io_memcmp(*n1, bp, len) ) {
				*tgt = i;
				return bp + len;
			}
		}
	}
	/* Nothing matched */
	return nullptr;
}

char *strptime(const char *buf, const char *fmt, struct tm *tm)
{
	unsigned char c;
	const unsigned char *bp, *ep;
	int alt_format, i, split_year = 0, neg = 0, offs;
	const char *new_fmt;

	bp = (const u_char *)buf;

	while (bp != NULL && (c = *fmt++) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;
		i = 0;

		/* Eat up white-space. */
		if (io_isspace(c)) {
			while (io_isspace(*bp))
				bp++;
			continue;
		}

		if (c != '%')
			goto literal;

again:
		switch (c = *fmt++) {
		case '%':	/* "%%" is converted to "%". */
literal:
			if (c != *bp++)
				return NULL;
			LEGAL_ALT(0);
			continue;

		/*
		* "Alternative" modifiers. Just set the appropriate flag
		* and start over again.
		*/
		case 'E':	/* "%E?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_E;
			goto again;

		case 'O':	/* "%O?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_O;
			goto again;

		/*
		* "Complex" conversion rules, implemented through recursion.
		 we do not need 'c'

		case 'c': Date and time, using the locale's format.
			new_fmt = _ctloc(d_t_fmt);
			goto recurse;
		*/

		case 'D':	/* The date as "%m/%d/%y". */
			new_fmt = "%m/%d/%y";
			LEGAL_ALT(0);
			goto recurse;

		case 'F':	/* The date as "%Y-%m-%d". */
			new_fmt = "%Y-%m-%d";
			LEGAL_ALT(0);
			goto recurse;

		case 'R':	/* The time as "%H:%M". */
			new_fmt = "%H:%M";
			LEGAL_ALT(0);
			goto recurse;

		case 'r':	/* The time in 12-hour clock representation. */
			new_fmt = "%I:%M:S %p";//_ctloc(t_fmt_ampm);
			LEGAL_ALT(0);
			goto recurse;

		case 'T':	/* The time as "%H:%M:%S". */
			new_fmt = "%H:%M:%S";
			LEGAL_ALT(0);
			goto recurse;

			/* we don't use 'X'
			case 'X': The time, using the locale's format.
				new_fmt =_ctloc(t_fmt);
				goto recurse;
			*/

			/* we do not need 'x'
			case 'x': The date, using the locale's format.
			new_fmt =_ctloc(d_fmt);*/
recurse:
			bp = (const u_char *)strptime((const char *)bp,
										  new_fmt, tm);
			LEGAL_ALT(ALT_E);
			continue;

		/*
		* "Elementary" conversion rules.
		*/
		case 'A':	/* The day of week, using the locale's form. */
		case 'a':
			bp = find_string(bp, &tm->tm_wday, day, abday, 7);
			LEGAL_ALT(0);
			continue;

		case 'B':	/* The month, using the locale's form. */
		case 'b':
		case 'h':
			bp = find_string(bp, &tm->tm_mon, mon, abmon, 12);
			LEGAL_ALT(0);
			continue;

		case 'C':	/* The century number. */
			i = 20;
			bp = conv_num(bp, &i, 0, 99);

			i = i * 100 - TM_YEAR_BASE;
			if (split_year)
				i += tm->tm_year % 100;
			split_year = 1;
			tm->tm_year = i;
			LEGAL_ALT(ALT_E);
			continue;

		case 'd':	/* The day of month. */
		case 'e':
			bp = conv_num(bp, &tm->tm_mday, 1, 31);
			LEGAL_ALT(ALT_O);
			continue;

		case 'k':	/* The hour (24-hour clock representation). */
			LEGAL_ALT(0);
		/* FALLTHROUGH */
		case 'H':
			bp = conv_num(bp, &tm->tm_hour, 0, 23);
			LEGAL_ALT(ALT_O);
			continue;

		case 'l':	/* The hour (12-hour clock representation). */
			LEGAL_ALT(0);
		/* FALLTHROUGH */
		case 'I':
			bp = conv_num(bp, &tm->tm_hour, 1, 12);
			if (tm->tm_hour == 12)
				tm->tm_hour = 0;
			LEGAL_ALT(ALT_O);
			continue;

		case 'j':	/* The day of year. */
			i = 1;
			bp = conv_num(bp, &i, 1, 366);
			tm->tm_yday = i - 1;
			LEGAL_ALT(0);
			continue;

		case 'M':	/* The minute. */
			bp = conv_num(bp, &tm->tm_min, 0, 59);
			LEGAL_ALT(ALT_O);
			continue;

		case 'm':	/* The month. */
			i = 1;
			bp = conv_num(bp, &i, 1, 12);
			tm->tm_mon = i - 1;
			LEGAL_ALT(ALT_O);
			continue;

		case 'p':	/* The locale's equivalent of AM/PM. */
			bp = find_string(bp, &i, am_pm, NULL, 2);
			if (tm->tm_hour > 11)
				return NULL;
			tm->tm_hour += i * 12;
			LEGAL_ALT(0);
			continue;

		case 'S':	/* The seconds. */
			bp = conv_num(bp, &tm->tm_sec, 0, 61);
			LEGAL_ALT(ALT_O);
			continue;

		case 's': {	/* seconds since the epoch */
			time_t sse = 0;
			uint64_t rulim = TIME_MAX;

			if (*bp < '0' || *bp > '9') {
				bp = NULL;
				continue;
			}

			do {
				sse *= 10;
				sse += *bp++ - '0';
				rulim /= 10;
			}
			while ((sse * 10 <= TIME_MAX) && rulim && *bp >= '0' && *bp <= '9');

			if (sse < 0 || (uint64_t)sse > TIME_MAX) {
				bp = NULL;
				continue;
			}

			tm = localtime(&sse);
			if(tm == NULL)
				bp = NULL;
		}
		continue;

		case 'U':	/* The week of year, beginning on sunday. */
		case 'W':	/* The week of year, beginning on monday. */
			/*
			 * XXX This is bogus, as we can not assume any valid
			 * information present in the tm structure at this
			 * point to calculate a real value, so just check the
			 * range for now.
			 */
			bp = conv_num(bp, &i, 0, 53);
			LEGAL_ALT(ALT_O);
			continue;

		case 'w':	/* The day of week, beginning on sunday. */
			bp = conv_num(bp, &tm->tm_wday, 0, 6);
			LEGAL_ALT(ALT_O);
			continue;

		case 'u':	/* The day of week, monday = 1. */
			bp = conv_num(bp, &i, 1, 7);
			tm->tm_wday = i % 7;
			LEGAL_ALT(ALT_O);
			continue;

		case 'g':	/* The year corresponding to the ISO week
			 * number but without the century.
			 */
			bp = conv_num(bp, &i, 0, 99);
			continue;

		case 'G':	/* The year corresponding to the ISO week
			 * number with century.
			 */
			do {
				bp++;
			}
			while (isdigit(*bp));
			continue;

		case 'V':	/* The ISO 8601:1988 week number as decimal */
			bp = conv_num(bp, &i, 0, 53);
			continue;

		case 'Y':	/* The year. */
			i = TM_YEAR_BASE;	/* just for data sanity... */
			bp = conv_num(bp, &i, 0, 9999);
			tm->tm_year = i - TM_YEAR_BASE;
			LEGAL_ALT(ALT_E);
			continue;

		case 'y':	/* The year within 100 years of the epoch. */
			/* LEGAL_ALT(ALT_E | ALT_O); */
			bp = conv_num(bp, &i, 0, 99);

			if (split_year)
				/* preserve century */
				i += (tm->tm_year / 100) * 100;
			else {
				split_year = 1;
				if (i <= 68)
					i = i + 2000 - TM_YEAR_BASE;
				else
					i = i + 1900 - TM_YEAR_BASE;
			}
			tm->tm_year = i;
			continue;

		case 'Z':
			_tzset();
			if (strncasecmp((const char *)bp, gmt, 3) == 0
					|| strncasecmp((const char *)bp, utc, 3) == 0) {
				tm->tm_isdst = 0;
#ifdef TM_GMTOFF
				tm->TM_GMTOFF = 0;
#endif
#ifdef TM_ZONE
				tm->TM_ZONE = gmt;
#endif
				bp += 3;
			}
			else {
				ep = find_string(bp, &i, (const char * const *)tzname, NULL, 2);
				if (ep != NULL) {
					tm->tm_isdst = i;
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -(timezone);
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = tzname[i];
#endif
				}
				bp = ep;
			}
			continue;

		case 'z':
			/*
			* We recognize all ISO 8601 formats:
			* Z	= Zulu time/UTC
			* [+-]hhmm
			* [+-]hh:mm
			* [+-]hh
			* We recognize all RFC-822/RFC-2822 formats:
			* UT|GMT
			*			 North American : UTC offsets
			* E[DS]T = Eastern : -4 | -5
			* C[DS]T = Central : -5 | -6
			* M[DS]T = Mountain: -6 | -7
			* P[DS]T = Pacific : -7 | -8
			*			 Military
			* [A-IL-M] = -1 ... -9 (J not used)
			* [N-Y]  = +1 ... +12
			*/
			while (io_isspace(*bp))
				++bp;

			switch (*bp++) {
			case 'G':
				if (*bp++ != 'M')
					return NULL;
			/*FALLTHROUGH*/
			case 'U':
				if (*bp++ != 'T')
					return NULL;
			/*FALLTHROUGH*/
			case 'Z':
				tm->tm_isdst = 0;
#ifdef TM_GMTOFF
				tm->TM_GMTOFF = 0;
#endif
#ifdef TM_ZONE
				tm->TM_ZONE = utc;
#endif
				continue;
			case '+':
				neg = 0;
				break;
			case '-':
				neg = 1;
				break;
			default:
				--bp;
				ep = find_string(bp, &i, nast, NULL, 4);
				if (ep != NULL) {
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -5 - i;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = __UNCONST(nast[i]);
#endif
					bp = ep;
					continue;
				}
				ep = find_string(bp, &i, nadt, NULL, 4);
				if (ep != NULL) {
					tm->tm_isdst = 1;
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -4 - i;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = __UNCONST(nadt[i]);
#endif
					bp = ep;
					continue;
				}

				if ((*bp >= 'A' && *bp <= 'I') ||
						(*bp >= 'L' && *bp <= 'Y')) {
#ifdef TM_GMTOFF
					/* Argh! No 'J'! */
					if (*bp >= 'A' && *bp <= 'I')
						tm->TM_GMTOFF = ('A' - 1) - (int)*bp;
					else if (*bp >= 'L' && *bp <= 'M')
						tm->TM_GMTOFF = 'A' - (int)*bp;
					else if (*bp >= 'N' && *bp <= 'Y')
						tm->TM_GMTOFF = (int)*bp - 'M';
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = NULL; /* XXX */
#endif
					bp++;
					continue;
				}
				return NULL;
			}
			offs = 0;
			for (i = 0; i < 4; ) {
				if (isdigit(*bp)) {
					offs = offs * 10 + (*bp++ - '0');
					i++;
					continue;
				}
				if (i == 2 && *bp == ':') {
					bp++;
					continue;
				}
				break;
			}
			switch (i) {
			case 2:
				offs *= 100;
				break;
			case 4:
				i = offs % 100;
				if (i >= 60)
					return NULL;
				/* Convert minutes into decimal */
				offs = (offs / 100) * 100 + (i * 50) / 30;
				break;
			default:
				return NULL;
			}
			if (neg)
				offs = -offs;
			tm->tm_isdst = 0;	/* XXX */
#ifdef TM_GMTOFF
			tm->TM_GMTOFF = offs;
#endif
#ifdef TM_ZONE
			tm->TM_ZONE = NULL;	/* XXX */
#endif
			continue;

		/*
		* Miscellaneous conversions.
		*/
		case 'n':	/* Any kind of white-space. */
		case 't':
			while (io_isspace(*bp))
				bp++;
			LEGAL_ALT(0);
			continue;


		default:	/* Unknown/unsupported conversion. */
			return NULL;
		}
	}

	return (char *)(bp);
}

#endif // __WIN32

namespace io {

namespace detail {

static char* fmt_true_false(char* to, std::size_t buf_size, bool value) noexcept
{
	if(value && buf_size >= 4) {
		io_memmove(to, "true", 4);
		return to+4;
	}
	else if(!value && buf_size >= 5) {
		io_memmove(to, "false", 5);
		return to+5;
	}
	return nullptr;
}

static char* fmt_yes_no(char* to, std::size_t buf_size, bool value) noexcept
{
	if(value && buf_size >= 3) {
		io_memmove(to, "yes", 3);
		return to+4;
	}
	else if(!value && buf_size >= 2) {
		io_memmove(to, "no", 2);
		return to+5;
	}
	return nullptr;
}

char* IO_PUBLIC_SYMBOL uintmax_to_chars_reverse(char* const last, uintmax_t value) noexcept
{
	static constexpr char _digits[201] =
		"0001020304050607080910111213141516171819"
		"2021222324252627282930313233343536373839"
		"4041424344454647484950515253545556575859"
		"6061626364656667686970717273747576777879"
		"8081828384858687888990919293949596979899";
	char* ret = last;
	while (value >= 100) {
		std::size_t idx = (value % 100) << 1;
		value /= 100;
		*ret = _digits[idx + 1];
		--ret;
		*ret = _digits[idx];
		--ret;
	}
	if (value >= 10) {
		std::size_t idx = value << 1;
		*ret = _digits[idx + 1];
		--ret;
		*ret = _digits[idx];
	}
	else {
		*ret = '0' + value;
	}
	return ret;
}

from_chars_result IO_PUBLIC_SYMBOL unsigned_from_chars(const char* first, const char* last, uintmax_t& value) noexcept
{
	from_chars_result ret = {nullptr, std::errc()};
	if(0 == memory_traits::distance(first,last) ) {
		ret.ec = std::errc::invalid_argument;
	}
	else {
		char *endp;
#	ifdef IO_CPU_BITS_64
		value = std::strtoull(first, &endp, 10);
#	else
		value = std::strtoul(first, &endp, 10);
#	endif // IO_CPU_BITS_64
		typedef std::numeric_limits<uintmax_t> limits;
		if ( (limits::max() == value) && (ERANGE == errno) ) {
			ret.ptr = nullptr;
			ret.ec = std::errc::result_out_of_range;
		}
		else {
			ret.ptr = endp;
		}
	}
	return ret;
}

from_chars_result IO_PUBLIC_SYMBOL signed_from_chars(const char* first, const char* last, intmax_t& value) noexcept
{
	from_chars_result ret = {nullptr, std::errc()};
	if(0 == memory_traits::distance(first,last) ) {
		ret.ec = std::errc::invalid_argument;
	}
	else {
		char *endp;
#	ifdef IO_CPU_BITS_64
		value = std::strtoll(first, &endp, 10);
#	else
		value = std::strtol(first, &endp, 10);
#	endif // IO_CPU_BITS_64
		typedef std::numeric_limits<intmax_t> limits;
		if ( ( (limits::max() == value) || (limits::min() == value) )  && (ERANGE == errno) ) {
			ret.ptr = nullptr;
			ret.ec = std::errc::result_out_of_range;
		}
		else {
			ret.ptr = endp;
		}
	}
	return ret;
}


template<typename T>
static constexpr unsigned int float_max_digits() noexcept
{
	typedef std::numeric_limits<T> limits_type;
	return 3 + limits_type::digits - limits_type::min_exponent;
}

static constexpr std::size_t FLOAT_MAX_DIGITS = float_max_digits<float>() + 1;
static constexpr std::size_t DOUBLE_MAX_DIGITS = float_max_digits<double>() + 1;
static constexpr std::size_t LONG_DOUBLE_MAX_DIGITS = float_max_digits<long double>() + 1;

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, float value) noexcept
{

	to_chars_result ret = {nullptr, std::errc()};
	std::size_t buf_size = memory_traits::distance(first,last);
	if( 0 == buf_size ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		char buff[ FLOAT_MAX_DIGITS ] = {'\0'};
#ifdef __GNUG__
		__builtin_snprintf(buff, FLOAT_MAX_DIGITS, "%e", value);
#else
		std::snprintf(buff, FLOAT_MAX_DIGITS, "%e", value);
#endif // __GNUG__
		std::size_t len = io_strlen(buff);
		if( len > buf_size ) {
			ret.ec = std::errc::no_buffer_space;
		}
		else {
			io_memmove(first, buff, len);
			ret.ptr = first + len;
		}
	}
	return ret;
}

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, double value) noexcept
{
	to_chars_result ret = {nullptr, std::errc()};
	std::size_t buf_size = memory_traits::distance(first,last);
	if( 0 == buf_size ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		char buff[ DOUBLE_MAX_DIGITS ] = {'\0'};
#ifdef __GNUG__
		__builtin_snprintf(buff, DOUBLE_MAX_DIGITS, "%e", value);
#else
		std::snprintf(buff, DOUBLE_MAX_DIGITS, "%e", value);
#endif // __GNUG__
		std::size_t len = io_strlen(buff);
		if( len > buf_size ) {
			ret.ec = std::errc::no_buffer_space;
		}
		else {
			io_memmove(first, buff, len);
			ret.ptr = first + len;
		}
	}
	return ret;
}

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, const long double& value) noexcept
{
	to_chars_result ret = {nullptr, std::errc()};
	std::size_t buf_size = memory_traits::distance(first,last);
	if( 0 == buf_size ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		char buff[ LONG_DOUBLE_MAX_DIGITS ] = {'\0'};
		std::snprintf(buff, LONG_DOUBLE_MAX_DIGITS, "%Le", value);
		std::size_t len = io_strlen(buff);
		if( len > buf_size ) {
			ret.ec = std::errc::no_buffer_space;
		}
		else {
			io_memmove(first, buff, len);
			ret.ptr = first + len;
		}
	}
	return ret;
}

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, float& value) noexcept
{
	from_chars_result ret = {nullptr, std::errc()};
	if(0 == memory_traits::distance(first,last) ) {
		ret.ec = std::errc::invalid_argument;
	}
	else {
		char *endp;
		value = std::strtof(first,&endp);
		if ( nullptr == endp ) {
			ret.ptr = nullptr;
			ret.ec = std::errc::argument_out_of_domain;
		}
		else {
			ret.ptr = endp;
		}
	}
	return ret;
}

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, double& value) noexcept
{
	from_chars_result ret = {nullptr, std::errc()};
	if(0 == memory_traits::distance(first,last) ) {
		ret.ec = std::errc::invalid_argument;
	}
	else {
		char *endp;
		value = std::strtod(first,&endp);
		if ( nullptr == endp ) {
			ret.ptr = nullptr;
			ret.ec = std::errc::argument_out_of_domain;
		}
		else {
			ret.ptr = endp;
		}
	}
	return ret;
}

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, long double& value) noexcept
{
	from_chars_result ret = {nullptr, std::errc()};
	if(0 == memory_traits::distance(first,last) ) {
		ret.ec = std::errc::invalid_argument;
	}
	else {
		char *endp;
		value = std::strtold(first,&endp);
		if ( nullptr == endp ) {
			ret.ptr = nullptr;
			ret.ec = std::errc::argument_out_of_domain;
		}
		else {
			ret.ptr = endp;
		}
	}
	return ret;
}

to_chars_result IO_PUBLIC_SYMBOL time_to_chars(char* const first, char* const last, const char* format, const std::time_t& value) noexcept
{
	to_chars_result ret = {nullptr, std::errc()};
	std::size_t aprox_buff_size = 2 * io_strlen(format);
	if( (first+aprox_buff_size) > last ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		const std::size_t buff_size = aprox_buff_size + 1;
		char* buff = static_cast<char*> ( io_alloca(  buff_size ) );
		io_zerro_mem(buff, buff_size );
		const std::size_t len = std::strftime(buff, buff_size, format, std::localtime(std::addressof(value)) );
		if( (first+len) > last  ) {
			ret.ec = std::errc::no_buffer_space;
		}
		else {
			io_memmove(first, buff, len);
			ret.ptr = first + len;
		}
	}
	return ret;
}

to_chars_result IO_PUBLIC_SYMBOL time_from_chars(const char* first,const char* last,const char* format, std::time_t& value) noexcept
{
	to_chars_result ret = {nullptr, std::errc()};
	if( (first+io_strlen(format)) >= last ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		tm t;
		ret.ptr = ::strptime( first, format, &t );
		value = std::mktime(&t);
	}
	return ret;
}

} // namespace detail


to_chars_result IO_PUBLIC_SYMBOL to_chars(char* first, char* last, bool value, str_bool_format fmt) noexcept
{
	to_chars_result ret = {nullptr, std::errc()};
	if( first >= last ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		std::size_t buf_size = memory_traits::distance(first,last);
		switch(fmt) {
		case str_bool_format::true_false:
			ret.ptr = detail::fmt_true_false(first,buf_size,value);
			break;
		case str_bool_format::yes_no:
			ret.ptr = detail::fmt_yes_no(first,buf_size,value);
			break;
		}
		if(nullptr == ret.ptr)
			ret.ec = std::errc::no_buffer_space;
	}
	return ret;
}

from_chars_result IO_PUBLIC_SYMBOL from_chars(const char* first,const char* last, bool& value) noexcept
{
	from_chars_result ret = {nullptr, std::errc()};
	if( (first+1) >= last ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		std::size_t buf_size = memory_traits::distance(first,last);
		char tmp[8] = { '\0' };
		const char* s = first;
		while( io_isspace(*s) && (s < last+1) ) {
			++s;
			if(last == s) {
				ret.ec = std::errc::illegal_byte_sequence;
				return ret;
			}
		}
		std::size_t max_len = buf_size > 5 ? 5 : buf_size;
		for(std::size_t i=0; i < max_len; i++) {
			tmp[i] = io_tolower(*s);
			++s;
		}
		if( 0 == io_memcmp(tmp,"no",2) ) {
			value = false;
			ret.ptr = first + 2;
		}
		else if(0 == io_memcmp(tmp,"yes",3)) {
			value = true;
			ret.ptr = first + 3;
		}
		else if(0 == io_memcmp(tmp,"true",4)) {
			value = true;
			ret.ptr = first + 4;
		}
		else if(0 == io_memcmp(tmp,"false",5) ) {
			value = false;
			ret.ptr = first + 5;
		}
		else {
			ret.ec = std::errc::result_out_of_range;
		}
	}
	return ret;
}

} // namespace io
