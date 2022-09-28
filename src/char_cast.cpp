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

#include <float.h>

#include "char_cast.hpp"
#include "string_algs.hpp"

#ifdef __IO_WINDOWS_BACKEND__

// TODO: review implementation found at
// https://github.com/msys2/MINGW-packages/blob/master/mingw-w64-libkml/strptime.c

typedef unsigned char u_char;
typedef unsigned int uint;

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

#ifndef __GNUC__
static int strncasecmp(const char* lhs, const char* rhs, std::size_t size) noexcept
{
	io::scoped_arr<char> l_lc( lhs, size );
	io::scoped_arr<char> r_lc( rhs, size );
	io::downcase_latin1(l_lc.begin());
	io::downcase_latin1(l_lc.begin());
	return io_memcmp(l_lc.begin(), r_lc.begin(), size);
}
#endif // __GNUC__

char *strptime(const char *buf, const char *fmt, struct tm *tm) noexcept
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
			bp = io::skip_spaces(bp);
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
			if (io_strncmp((const char *)bp, gmt, 3) == 0
					||  strncasecmp((const char *)bp, utc, 3) == 0) {
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
				ep = find_string(bp, &i, (const char * const *)_tzname, NULL, 2);
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
			bp = io::skip_spaces(bp);

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
			bp = io::skip_spaces(bp);
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
		return to+3;
	}
	else if(!value && buf_size >= 2) {
		io_memmove(to, "no", 2);
		return to+2;
	}
	return nullptr;
}


// Brached LUT constants
static char DIGITS[201] =
	"0001020304050607080910111213141516171819"
	"2021222324252627282930313233343536373839"
	"4041424344454647484950515253545556575859"
	"6061626364656667686970717273747576777879"
	"8081828384858687888990919293949596979899";


static uint32_t BRANCH_1  = 10U;
static uint32_t BRANCH_2  = 100U;
static uint32_t BRANCH_3  = 1000U;
static uint32_t BRANCH_4  = 10000U;
static uint32_t BRANCH_5  = 100000U;
static uint32_t BRANCH_6  = 1000000U;
static uint32_t BRANCH_7  = 10000000U;
static uint32_t BRANCH_8  = 100000000U;
static uintmax_t BRANCH_9  = 1000000000UL;
static uintmax_t BRANCH_10 = 10000000000UL;
static uintmax_t BRANCH_11 = 100000000000UL;
static uintmax_t BRANCH_12 = 1000000000000UL;
static uintmax_t BRANCH_13 = 10000000000000UL;
static uintmax_t BRANCH_14 = 100000000000000UL;
static uintmax_t BRANCH_15 = 1000000000000000UL;
static uintmax_t BRANCH_16 = 10000000000000000UL;

static char* copy_one(char* s, const uint32_t i) noexcept
{
	*s = DIGITS[ i ];
	return --s;
}

static char* copy_two(char* s, const uint32_t i) noexcept
{
	return copy_one( copy_one(s, (i + 1) ), i );
}

static char* copy_four(char* s, const uint32_t i, const uint32_t j) noexcept
{
	char* ret = copy_two(s, j);
	ret = copy_two(ret, i);
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) char* uintmax_to_chars_reverse(char* const last, uintmax_t value) noexcept
#else
char* IO_PUBLIC_SYMBOL uintmax_to_chars_reverse(char* const last, uintmax_t value) noexcept
#endif // _MSC_VER
{
	char* ret = last;
	if (value < BRANCH_8) {
		uint32_t v = static_cast<uint32_t>(value);
		if(v < BRANCH_4) {
			const uint32_t d1 = static_cast<uint32_t>(v / BRANCH_2) << 1;
			const uint32_t d2 = static_cast<uint32_t>(v % BRANCH_2) << 1;
			ret = copy_one(ret, d2 + 1);
			if (v >= BRANCH_1)
				ret = copy_one(ret, d2);
			if (v >= BRANCH_2)
				ret = copy_one(ret, d1 + 1);
			if (v >= BRANCH_3)
				ret = copy_one(ret, d1);
			++ret;
		}
		else {
			const uint32_t b = static_cast<uint32_t>(v / BRANCH_4);
			const uint32_t c = static_cast<uint32_t>(v % BRANCH_4);

			const uint32_t d1 = static_cast<uint32_t>(b / BRANCH_2) << 1;
			const uint32_t d2 = static_cast<uint32_t>(b % BRANCH_2) << 1;
			const uint32_t d3 = static_cast<uint32_t>(c / BRANCH_2) << 1;
			const uint32_t d4 = static_cast<uint32_t>(c % BRANCH_2) << 1;

			ret = copy_two(ret, d4);
			ret = copy_two(ret, d3);
			ret = copy_one(ret, d2 + 1);

			if (value >= BRANCH_5)
				ret = copy_one(ret, d2);
			if (value >= BRANCH_6)
				ret = copy_one(ret, d1 + 1);
			if (value >= BRANCH_7)
				ret = copy_one(ret, d1);
			++ret;
		}
	}
	else if(value < BRANCH_16) {
		const uint32_t v0 = static_cast<uint32_t>(value / BRANCH_8);
		const uint32_t b0 = (v0 / BRANCH_4);
		const uint32_t c0 = (v0 % BRANCH_4);

		const uint32_t v1 = static_cast<uint32_t>(value % BRANCH_8);
		const uint32_t b1 = (v1 / BRANCH_4);
		const uint32_t c1 = (v1 % BRANCH_4);

		const uint32_t d1 = (b0 / BRANCH_2) << 1;
		const uint32_t d2 = (b0 % BRANCH_2) << 1;
		const uint32_t d3 = (c0 / BRANCH_2) << 1;
		const uint32_t d4 = (c0 % BRANCH_2) << 1;
		const uint32_t d5 = (b1 / BRANCH_2) << 1;
		const uint32_t d6 = (b1 % BRANCH_2) << 1;
		const uint32_t d7 = (c1 / BRANCH_2) << 1;
		const uint32_t d8 = (c1 % BRANCH_2) << 1;

		ret = copy_two(ret, d8);
		ret = copy_two(ret, d7);
		ret = copy_two(ret, d6);
		ret = copy_two(ret, d5);
		if (value >= BRANCH_8)
			ret = copy_one(ret, d4 + 1);
		if (value >= BRANCH_9)
			ret = copy_one(ret, d4);
		if (value >= BRANCH_10)
			ret = copy_one(ret, d3 + 1);
		if (value >= BRANCH_11)
			ret = copy_one(ret, d3);
		if (value >= BRANCH_12)
			ret = copy_one(ret, d2 + 1);
		if (value >= BRANCH_13)
			ret = copy_one(ret, d2);
		if (value >= BRANCH_14)
			ret = copy_one(ret, d1 + 1);
		if (value >= BRANCH_15)
			ret = copy_one(ret, d1);
		++ret;
	}
	else {

		const uint32_t a = static_cast<uint32_t>(value / BRANCH_16); // 1 to 1844
		value %= BRANCH_16;

		const uint32_t v0 = static_cast<uint32_t>(value / BRANCH_8);
		const uint32_t b0 = v0 / BRANCH_4;
		const uint32_t c0 = v0 % BRANCH_4;

		const uint32_t v1 = static_cast<uint32_t>(value % BRANCH_8);
		const uint32_t b1 = v1 / BRANCH_4;
		const uint32_t c1 = v1 % BRANCH_4;

		const uint32_t d1 = (b0 / BRANCH_2) << 1;
		const uint32_t d2 = (b0 % BRANCH_2) << 1;
		const uint32_t d3 = (c0 / BRANCH_2) << 1;
		const uint32_t d4 = (c0 % BRANCH_2) << 1;
		const uint32_t d5 = (b1 / BRANCH_2) << 1;
		const uint32_t d6 = (b1 % BRANCH_2) << 1;
		const uint32_t d7 = (c1 / BRANCH_2) << 1;
		const uint32_t d8 = (c1 % BRANCH_2) << 1;

		ret = copy_two(ret, d8);
		ret = copy_two(ret, d7);
		ret = copy_two(ret, d6);
		ret = copy_two(ret, d5);
		ret = copy_two(ret, d4);
		ret = copy_two(ret, d3);
		ret = copy_two(ret, d2);
		ret = copy_two(ret, d1);

		if (a < BRANCH_1) {
			*ret = '0' + static_cast<char>(a);
			--ret;
		}
		else if(a < BRANCH_2) {
			const uint32_t i = static_cast<uint32_t>(a << 1);
			ret = copy_two(ret, i);
		}
		else if (a < BRANCH_3) {
			const uint32_t i = static_cast<uint32_t>(a % BRANCH_2) << 1;
			ret = copy_two(ret, i);
			*ret = '0' + static_cast<char>(a / BRANCH_2);
			--ret;
		}
		else {
			const uint32_t i = static_cast<uint32_t>(a / BRANCH_2) << 1;
			const uint32_t j = static_cast<uint32_t>(a % BRANCH_2) << 1;
			ret = copy_four(ret, i, j);
		}
		++ret;
	}
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) from_chars_result unsigned_from_chars(const char* first, const char* last, uintmax_t& value) noexcept
#else
from_chars_result IO_PUBLIC_SYMBOL unsigned_from_chars(const char* first, const char* last, uintmax_t& value) noexcept
#endif
{
	from_chars_result ret = {nullptr, {} };
	if( first >= last ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		char *endp;
#	ifdef IO_CPU_BITS_64
		value = std::strtoull(first, &endp, 10);
#	else
		value = std::strtoul(first, &endp, 10);
#	endif // IO_CPU_BITS_64

		if ( ERANGE == errno ) {
			ret.ptr = nullptr;
			ret.ec = std::errc::result_out_of_range;
		}
		else {
			ret.ptr = endp;
		}
	}
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) from_chars_result signed_from_chars(const char* first, const char* last, intmax_t& value) noexcept
#else
from_chars_result IO_PUBLIC_SYMBOL signed_from_chars(const char* first, const char* last, intmax_t& value) noexcept
#endif
{
	from_chars_result ret = {nullptr, {}};
	if( first >= last ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		char *endp;
#	ifdef IO_CPU_BITS_64
		value = std::strtoll(first, &endp, 10);
#	else
		value = std::strtol(first, &endp, 10);
#	endif // IO_CPU_BITS_64
		ret.ptr = endp;
	}
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) to_chars_result float_to_chars(char* const first, char* const last, float value, unsigned int max_digits) noexcept
#else
to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, float value, unsigned int max_digits) noexcept
#endif
{
	to_chars_result ret = {nullptr, std::errc()};
	std::size_t buff_len = memory_traits::distance(first,last);
	if( (first >= last) || (buff_len < (FLT_DIG + max_digits)) ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		io_snprintf(first, buff_len, "%.*f", static_cast<int>(max_digits), value);
		std::size_t offset = io_strlen(first);
		ret.ptr = first + offset;
	}
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) to_chars_result float_to_chars(char* const first, char* const last, double value, unsigned int max_digits) noexcept
#else
to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, double value, unsigned int max_digits) noexcept
#endif
{
	to_chars_result ret = {nullptr, std::errc()};
	std::size_t buff_len = memory_traits::distance(first,last);
	if( (first >= last) || (buff_len < (DBL_DIG + max_digits)) ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		io_snprintf(first, buff_len, "%.*f", static_cast<int>(max_digits), value);
		std::size_t offset = io_strlen(first);
		ret.ptr = first + offset;
	}
	return ret;
}

#ifdef _MSC_VER
_declspec(dllexport) to_chars_result float_to_chars(char* const first, char* const last, const long double& value, unsigned int max_digits) noexcept
#else
to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, const long double& value, unsigned int max_digits) noexcept
#endif
{
	to_chars_result ret = {nullptr, std::errc()};
	std::size_t buff_len = memory_traits::distance(first,last);
	if( (first >= last) || (buff_len < (LDBL_DIG + max_digits) ) ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		io_snprintf(first, buff_len, "%.*Lf", static_cast<int>(max_digits), value);
		std::size_t offset = io_strlen(first);
		ret.ptr = first + offset;
	}
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) from_chars_result float_from_chars(const char* first, const char* last, float& value) noexcept
#else
from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, float& value) noexcept
#endif
{
	from_chars_result ret = {nullptr, std::errc()};
	if( io_unlikely(first >= last) ) {
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

#ifdef _MSC_VER
__declspec(dllexport) from_chars_result float_from_chars(const char* first, const char* last, double& value) noexcept
#else
from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, double& value) noexcept
#endif
{
	from_chars_result ret = {nullptr, std::errc()};
	if( io_unlikely(first >= last) ) {
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

#ifdef _MSC_VER
__declspec(dllexport) from_chars_result float_from_chars(const char* first, const char* last, long double& value) noexcept
#else
from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, long double& value) noexcept
#endif
{
	from_chars_result ret = {nullptr, std::errc()};
	if( io_unlikely(first >= last) ) {
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

} // namespace detail

#ifdef _MSC_VER
__declspec(dllexport) to_chars_result to_chars(char* first, char* last, bool value, str_bool_format fmt) noexcept
#else
to_chars_result IO_PUBLIC_SYMBOL to_chars(char* first, char* last, bool value, str_bool_format fmt) noexcept
#endif
{
	to_chars_result ret = {nullptr, std::errc()};
	if( io_unlikely(first >= last) ) {
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

static bool cmp_no(const char* s) noexcept
{
	return start_with(s,"no",2);
}

static bool cmp_yes(const char* s) noexcept
{
	return start_with(s,"yes",3);
}

static bool cmp_true(const char* s) noexcept
{
	return start_with(s, "true", 4);
}

static bool cmp_false(const char* s) noexcept
{
	return start_with(s, "false", 5);
}

#ifdef _MSC_VER
__declspec(dllexport) from_chars_result from_chars(const char* first, const char* last, bool& value) noexcept
#else
from_chars_result IO_PUBLIC_SYMBOL from_chars(const char* first,const char* last, bool& value) noexcept
#endif
{
	from_chars_result ret = {nullptr, {}};
	const char* s = skip_spaces_ranged(first, last);
	if( (s+1) >= last ) {
		ret.ec = std::errc::invalid_argument;
	}
	else {
		std::size_t buf_size = memory_traits::distance(s,last);
		char tmp[8] = { '\0' };
		const std::size_t max_len = buf_size > 5 ? 5 : buf_size;
		io_memmove(tmp, s, max_len);
		downcase_latin1(tmp);
		if( cmp_no(tmp) ) {
			value = false;
			ret.ptr = s + 2;
		}
		else if(cmp_yes(tmp)) {
			value = true;
			ret.ptr = s + 3;
		}
		else if(cmp_true(tmp)) {
			value = true;
			ret.ptr = s + 4;
		}
		else if(cmp_false(tmp)) {
			value = false;
			ret.ptr = s + 5;
		}
		else {
			ret.ec = std::errc::result_out_of_range;
		}
	}
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) io::const_string to_string(std::error_code& ec, const bool value, str_bool_format fmt) noexcept
#else
io::const_string IO_PUBLIC_SYMBOL to_string(std::error_code& ec, const bool value, str_bool_format fmt) noexcept
#endif
{
	char tmp[6] = {'\0'};
	auto ret = to_chars(tmp, tmp+sizeof(tmp), value, fmt);
	if( 0 != static_cast<unsigned int>(ret.ec) ) {
 		ec = std::make_error_code( ret.ec );
		return io::const_string();
	}
	return io::const_string(tmp);
}


#ifdef _MSC_VER
__declspec(dllexport) to_chars_result to_chars(char* const first, char* const last, const char* format, const std::time_t& value) noexcept
#else
to_chars_result IO_PUBLIC_SYMBOL to_chars(char* const first, char* const last, const char* format, const std::time_t& value) noexcept
#endif
{
	to_chars_result ret = {nullptr, std::errc()};
	if( io_unlikely( (first+io_strlen(format)) >= last ) ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		const std::size_t buff_size = (2 * io_strlen(format) ) + 1;
		io::scoped_arr<char> buff(buff_size);
		const std::size_t len = std::strftime(buff.begin(), buff_size, format, std::localtime(std::addressof(value)) );
		if( (first+len) > last  ) {
			ret.ec = std::errc::no_buffer_space;
		}
		else {
			io_memmove(first, buff.begin(), len);
			ret.ptr = first + len;
		}
	}
	return ret;
}

#ifdef _MSC_VER
__declspec(dllexport) to_chars_result from_chars(const char* first, const char* last, const char* format, std::time_t& value) noexcept
#else
to_chars_result IO_PUBLIC_SYMBOL from_chars(const char* first,const char* last,const char* format, std::time_t& value) noexcept
#endif
{
	to_chars_result ret = {nullptr, std::errc()};
	if( io_unlikely( (first+io_strlen(format)) >= last ) ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else {
		tm t;
		ret.ptr = ::strptime( first, format, &t );
		value = std::mktime(&t);
	}
	return ret;
}

} // namespace io

