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

#include <io/core/string_algs.hpp>

#include "io/textapi/charset_detector.hpp"

/*
 This is C++ 11 minimal port of Mozilla universal character set detector
 FIXME: Those state machines is over-complicated, an performance un-effective
  Needed to be replaced with something else like https://github.com/google/compact_enc_det
*/
namespace io {

namespace coding {

enum class idx_sft : uint8_t {
	bits_4  = 3,
	bits_8  = 2,
	bits_16 = 1
};

enum class sft_msk: uint8_t {
	bits_4  = 7,
	bits_8  = 3,
	bits_16 = 1
};

enum class bit_sft: uint8_t {
	bits_4  = 2,
	bits_8  = 3,
	bits_16 = 4
};

enum class unit_msk: uint32_t {
	bits_4  = 0x0000000FL,
	bits_8  = 0x000000FFL,
	bits_16 = 0x0000FFFFL
};

struct pkg_int {
	idx_sft  idxsft;
	sft_msk  sftmsk;
	bit_sft  bitsft;
	unit_msk unitmsk;
	uint32_t  *data;
};

struct model_t {
	pkg_int class_table;
	uint32_t class_factor;
	pkg_int state_table;
	const uint32_t *char_len_table;
};

enum class state_t: uint32_t {
	start = 0,
	error = 1,
	found = 2
};

static inline uint32_t get_from_pck(const std::size_t i, const pkg_int& ct)
{
	uint32_t di = ct.data[ i >> static_cast<std::size_t>(ct.idxsft) ];
	uint32_t dv = (i & static_cast<uint32_t>(ct.sftmsk)) << static_cast<uint32_t>(ct.bitsft);
	return  (di >> dv) & static_cast<uint32_t>(ct.unitmsk);
}

static inline uint32_t get_class(uint8_t c, const model_t* model)
{
	return get_from_pck( static_cast<std::size_t>(c), model->class_table);
}

class state_machine;
DECLARE_IPTR( state_machine );

class state_machine  {
	state_machine(const state_machine&) = delete;
	state_machine&  operator=(const state_machine&) = delete;
public:

	explicit constexpr state_machine(const model_t* model) noexcept:
		model_(model),
		state_(state_t::start),
		char_len_(0),
		byte_pos_(0)
	{}

	state_t next_state(uint8_t c) noexcept {
		// for each byte we get its class,
		// if it is first byte, we also get byte length
		uint32_t byte_class = get_class( c, model_);
		if(state_ == state_t::start) {
			byte_pos_ = 0;
			char_len_ = static_cast<uint8_t>( model_->char_len_table[ byte_class ] );
		}
		//from byte's class and stateTable, we get its next state
		uint32_t ind = static_cast<uint32_t>(state_) * (model_->class_factor) + byte_class;
		state_ = static_cast<state_t>( get_from_pck( ind, model_->state_table ) );
		++byte_pos_;
		return  state_;
	}

	inline uint8_t current_char_len() const noexcept {
		return char_len_;
	}

private:
	const model_t* model_;
	state_t state_;
	uint8_t  char_len_;
	std::size_t byte_pos_;
};

static constexpr uint32_t PCK16BITS(uint32_t a, uint32_t b)
{
	return static_cast<uint32_t> ( (b << 16) | a );
}

static constexpr uint32_t PCK8BITS(uint32_t a,uint32_t b,uint32_t c, uint32_t d)
{
	return PCK16BITS(  ( ( b << 8) | a), ( ( d << 8) | c)  );
}

static constexpr uint32_t PCK4BITS(uint32_t a, uint32_t b, uint32_t c,uint32_t d,
								   uint32_t e, uint32_t f, uint32_t g, uint32_t h)
{
	return PCK8BITS( ( (b << 4) | a),
					 ( (d << 4) | c),
					 ( (f << 4) | e),
					 ( (h << 4) | g)
				   );
}


namespace latin1 {

// undefined
static constexpr uint32_t UDF = 0;
//other
static constexpr uint8_t OTH = 1;
// ascii capital letter
static constexpr uint8_t ASC = 2;
// ascii small letter
static constexpr uint8_t ASS = 3;
// accent capital vowel
static constexpr uint8_t ACV = 4;
// accent capital other
static constexpr uint8_t ACO = 5;
// accent small vowel
static constexpr uint8_t ASV = 6;
// accent small other
static constexpr uint8_t ASO = 7;
// total classes
static constexpr uint8_t CLASS_NUM = 8;

static uint8_t CHAR_TO_CLASS[] = {
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 00 - 07
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 08 - 0F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 10 - 17
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 18 - 1F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 20 - 27
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 28 - 2F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 30 - 37
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 38 - 3F
	OTH, ASC, ASC, ASC, ASC, ASC, ASC, ASC,   // 40 - 47
	ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,   // 48 - 4F
	ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,   // 50 - 57
	ASC, ASC, ASC, OTH, OTH, OTH, OTH, OTH,   // 58 - 5F
	OTH, ASS, ASS, ASS, ASS, ASS, ASS, ASS,   // 60 - 67
	ASS, ASS, ASS, ASS, ASS, ASS, ASS, ASS,   // 68 - 6F
	ASS, ASS, ASS, ASS, ASS, ASS, ASS, ASS,   // 70 - 77
	ASS, ASS, ASS, OTH, OTH, OTH, OTH, OTH,   // 78 - 7F
	OTH, UDF, OTH, ASO, OTH, OTH, OTH, OTH,   // 80 - 87
	OTH, OTH, ACO, OTH, ACO, UDF, ACO, UDF,   // 88 - 8F
	UDF, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // 90 - 97
	OTH, OTH, ASO, OTH, ASO, UDF, ASO, ACO,   // 98 - 9F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // A0 - A7
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // A8 - AF
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // B0 - B7
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,   // B8 - BF
	ACV, ACV, ACV, ACV, ACV, ACV, ACO, ACO,   // C0 - C7
	ACV, ACV, ACV, ACV, ACV, ACV, ACV, ACV,   // C8 - CF
	ACO, ACO, ACV, ACV, ACV, ACV, ACV, OTH,   // D0 - D7
	ACV, ACV, ACV, ACV, ACV, ACO, ACO, ACO,   // D8 - DF
	ASV, ASV, ASV, ASV, ASV, ASV, ASO, ASO,   // E0 - E7
	ASV, ASV, ASV, ASV, ASV, ASV, ASV, ASV,   // E8 - EF
	ASO, ASO, ASV, ASV, ASV, ASV, ASV, OTH,   // F0 - F7
	ASV, ASV, ASV, ASV, ASV, ASO, ASO, ASO,   // F8 - FF
};

static uint8_t CLASS_MODEL[] = {
	/*      UDF OTH ASC ASS ACV ACO ASV ASO  */
	/*UDF*/  0,  0,  0,  0,  0,  0,  0,  0,
	/*OTH*/  0,  3,  3,  3,  3,  3,  3,  3,
	/*ASC*/  0,  3,  3,  3,  3,  3,  3,  3,
	/*ASS*/  0,  3,  3,  3,  1,  1,  3,  3,
	/*ACV*/  0,  3,  3,  3,  1,  2,  1,  2,
	/*ACO*/  0,  3,  3,  3,  3,  3,  3,  3,
	/*ASV*/  0,  3,  1,  3,  1,  1,  1,  3,
	/*ASO*/  0,  3,  1,  3,  1,  1,  3,  3,
};

} // namespace latin1

namespace single_byte {

/** Codepoints **/

/* Illegal codepoints.*/
static constexpr uint8_t ILL = 255;
/* Control character. */
static constexpr uint8_t CTR = 254;
/* Symbols and punctuation that does not belong to words. */
static constexpr uint8_t SYM = 253;
/* Return/Line feeds. */
static constexpr uint8_t RET = 252;
/* Numbers 0-9. */
static constexpr uint8_t NUM = 251;

struct sequence_model {
	// [256] table use to find a char's order
	uint8_t *char_to_order_map;
	// [SAMPLE_SIZE][SAMPLE_SIZE]; table to find a 2-char sequence's frequency
	char *precedence_matrix;
	// The count of frequent characters.
	int freq_char_count;
	// = freqSeqs / totalSeqs
	float  typical_positive_ratio;
	// says if this script contains English characters (not implemented)
	bool keep_english_letter;
	// charset
	charset character_set;
};

namespace cyrylic {

//KOI8-R language model
//Character Mapping Table:
static const unsigned char KOI8R_CHAR_TO_ORDER_MAP[] = {
	CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,RET,CTR,CTR,RET,CTR,CTR,  //00
	CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,  //10
	SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,  //20
	NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,SYM,SYM,SYM,SYM,SYM,SYM,  //30
	SYM,142,143,144,145,146,147,148,149,150,151,152, 74,153, 75,154,  //40
	155,156,157,158,159,160,161,162,163,164,165,SYM,SYM,SYM,SYM,SYM,  //50
	SYM, 71,172, 66,173, 65,174, 76,175, 64,176,177, 77, 72,178, 69,  //60
	67,179, 78, 73,180,181, 79,182,183,184,185,SYM,SYM,SYM,SYM,SYM,  //70
	191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,  //80
	207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,  //90
	223,224,225, 68,226,227,228,229,230,231,232,233,234,235,236,237,  //a0
	238,239,240,241,242,243,244,245,246,247,248,249,250,251,NUM,SYM,  //b0
	27,  3, 21, 28, 13,  2, 39, 19, 26,  4, 23, 11,  8, 12,  5,  1,  //c0
	15, 16,  9,  7,  6, 14, 24, 10, 17, 18, 20, 25, 30, 29, 22, 54,  //d0
	59, 37, 44, 58, 41, 48, 53, 46, 55, 42, 60, 36, 49, 38, 31, 34,  //e0
	35, 43, 45, 32, 40, 52, 56, 33, 61, 62, 51, 57, 47, 63, 50, 70,  //f0
};

} // namesapace cyrylic

} // namespace single_byte

namespace unicode {

static uint32_t UTF8_class[] = {
	//PCK4BITS(0,1,1,1,1,1,1,1),  // 00 - 07
	PCK4BITS(1,1,1,1,1,1,1,1),  // 00 - 07  //allow 0x00 as a legal value
	PCK4BITS(1,1,1,1,1,1,0,0),  // 08 - 0f
	PCK4BITS(1,1,1,1,1,1,1,1),  // 10 - 17
	PCK4BITS(1,1,1,0,1,1,1,1),  // 18 - 1f
	PCK4BITS(1,1,1,1,1,1,1,1),  // 20 - 27
	PCK4BITS(1,1,1,1,1,1,1,1),  // 28 - 2f
	PCK4BITS(1,1,1,1,1,1,1,1),  // 30 - 37
	PCK4BITS(1,1,1,1,1,1,1,1),  // 38 - 3f
	PCK4BITS(1,1,1,1,1,1,1,1),  // 40 - 47
	PCK4BITS(1,1,1,1,1,1,1,1),  // 48 - 4f
	PCK4BITS(1,1,1,1,1,1,1,1),  // 50 - 57
	PCK4BITS(1,1,1,1,1,1,1,1),  // 58 - 5f
	PCK4BITS(1,1,1,1,1,1,1,1),  // 60 - 67
	PCK4BITS(1,1,1,1,1,1,1,1),  // 68 - 6f
	PCK4BITS(1,1,1,1,1,1,1,1),  // 70 - 77
	PCK4BITS(1,1,1,1,1,1,1,1),  // 78 - 7f
	PCK4BITS(2,2,2,2,3,3,3,3),  // 80 - 87
	PCK4BITS(4,4,4,4,4,4,4,4),  // 88 - 8f
	PCK4BITS(4,4,4,4,4,4,4,4),  // 90 - 97
	PCK4BITS(4,4,4,4,4,4,4,4),  // 98 - 9f
	PCK4BITS(5,5,5,5,5,5,5,5),  // a0 - a7
	PCK4BITS(5,5,5,5,5,5,5,5),  // a8 - af
	PCK4BITS(5,5,5,5,5,5,5,5),  // b0 - b7
	PCK4BITS(5,5,5,5,5,5,5,5),  // b8 - bf
	PCK4BITS(0,0,6,6,6,6,6,6),  // c0 - c7
	PCK4BITS(6,6,6,6,6,6,6,6),  // c8 - cf
	PCK4BITS(6,6,6,6,6,6,6,6),  // d0 - d7
	PCK4BITS(6,6,6,6,6,6,6,6),  // d8 - df
	PCK4BITS(7,8,8,8,8,8,8,8),  // e0 - e7
	PCK4BITS(8,8,8,8,8,9,8,8),  // e8 - ef
	PCK4BITS(10,11,11,11,11,11,11,11),  // f0 - f7
	PCK4BITS(12,13,13,13,14,15,0,0)   // f8 - ff
};

static uint32_t UTF8_states [26] = {
	PCK4BITS(1,0,1,1,1,1,12,10),//00-07
	PCK4BITS(9,11,8,7,6,5,4,3),//08-0f
	PCK4BITS(1,1,1,1,1,1,1,1),//10-17
	PCK4BITS(1,1,1,1,1,1,1,1),//18-1f
	PCK4BITS(2,2,2,2,2,2,2,2),//20-27
	PCK4BITS(2,2,2,2,2,2,2,2),//28-2f
	PCK4BITS(1,1,5,5,5,5,1,1),//30-37
	PCK4BITS(1,1,1,1,1,1,1,1),//38-3f
	PCK4BITS(1,1,1,5,5,5,1,1),//40-47
	PCK4BITS(1,1,1,1,1,1,1,1),//48-4f
	PCK4BITS(1,1,7,7,7,7,1,1),//50-57
	PCK4BITS(1,1,1,1,1,1,1,1),//58-5f
	PCK4BITS(1,1,1,1,7,7,1,1),//60-67
	PCK4BITS(1,1,1,1,1,1,1,1),//68-6f
	PCK4BITS(1,1,9,9,9,9,1,1),//70-77
	PCK4BITS(1,1,1,1,1,1,1,1),//78-7f
	PCK4BITS(1,1,1,1,1,9,1,1),//80-87
	PCK4BITS(1,1,1,1,1,1,1,1),//88-8f
	PCK4BITS(1,1,12,12,12,12,1,1),//90-97
	PCK4BITS(1,1,1,1,1,1,1,1),//98-9f
	PCK4BITS(1,1,1,1,1,12,1,1),//a0-a7
	PCK4BITS(1,1,1,1,1,1,1,1),//a8-af
	PCK4BITS(1,1,12,12,12,1,1,1),//b0-b7
	PCK4BITS(1,1,1,1,1,1,1,1),//b8-bf
	PCK4BITS(1,1,0,0,0,0,1,1),//c0-c7
	PCK4BITS(1,1,1,1,1,1,1,1) //c8-cf
};

static const uint32_t UTF8_CHAR_LEN_TABLE[16] = {
	0, 1, 0, 0,
	0, 0, 2, 3,
	3, 3, 4, 4,
	5, 5, 6, 6
};

static model_t UTF8_MODEL = {
	{idx_sft::bits_4, sft_msk::bits_4, bit_sft::bits_4, unit_msk::bits_4, UTF8_class},
	16,
	{idx_sft::bits_4, sft_msk::bits_4, bit_sft::bits_4, unit_msk::bits_4, UTF8_states},
	UTF8_CHAR_LEN_TABLE
};

} // namesapce unicode

} // namespace coding

namespace detail {

// prober
prober::prober() noexcept
{}

byte_buffer prober::filter_without_english_letters(std::error_code& ec, const uint8_t* buff, std::size_t size) const noexcept
{
	byte_buffer ret = byte_buffer::allocate(ec, size);
	if (ec)
		return ret;
	const uint8_t *end = buff + size;
	const uint8_t *prev_ptr = buff, *cur_ptr = buff;
	bool meet_MSB = false;
	for (; cur_ptr < end; cur_ptr++) {
		if (*cur_ptr & 0x80) {
			meet_MSB = false;
			continue;
		}
		if ( is_latin1( static_cast<char>(*cur_ptr) ) ) {
			//current char is a symbol, most likely a punctuation. we treat it as segment delimiter
			if (meet_MSB && (cur_ptr > prev_ptr) ) {
				// this segment contains more than single symbol, and it has upper ASCII, we need to keep it
				ret.put(cur_ptr, cur_ptr);
				ret.put(' ');
				meet_MSB = false;
			}
			else
				//ignore current segment. (either because it is just a symbol or just an English word)
				prev_ptr = cur_ptr + 1;
		}
	}
	if (meet_MSB && (cur_ptr > prev_ptr) )
		ret.put(cur_ptr, cur_ptr);
	ret.flip();
	return ret;
}

//do filtering to reduce load to probers
byte_buffer prober::filter_with_english_letters(std::error_code& ec, const uint8_t* buff, std::size_t size) const noexcept
{
	byte_buffer ret = byte_buffer::allocate(ec, size);
	if (ec)
		return ret;
	bool inside_tag = false;
	const uint8_t *end = buff + size;
	const uint8_t *next = buff;
	const uint8_t *prev = buff;
	while ( next < end ) {
	    if( cheq('<', *next ) ) {
	    	inside_tag = true;
	    } else if( cheq('>',*next) ) {
	    	inside_tag = false;
	    }
		if( ! (*next & 0x80) && is_latin1( static_cast<char>(*next) ) ) {
			// Current segment contains more than just a symbol
			// and it is not inside a tag, keep it.
			if (next > prev && !inside_tag) {
				ret.put( prev, next);
				prev = next;
			} else {
				prev = next + 1;
			}
		}
		++next;
	}
	// If the current segment contains more than just a symbol
	// and it is not inside a tag then keep it.
	if (!inside_tag )
		ret.put(prev, next);
	ret.flip();
	return ret;
}

// latin1_prober
class latin1_prober final:public prober {
private:
	static constexpr std::size_t FREQ_CAT_NUM =  4;
	friend class nobadalloc<latin1_prober>;
	latin1_prober() noexcept:
		prober()
	{}
	float calc_confidence(const uint32_t *freq_counter) const noexcept {
		float ret = 0.0f;
		std::size_t total = 0;
		for (std::size_t i = 0; i < FREQ_CAT_NUM; i++)
			total += freq_counter[ i ];
		if(total > 0) {
			ret = freq_counter[3] * 1.0f / total;
			ret -= freq_counter[1] * 20.0f / total;
		}
		return ( ret < 0.0f) ? 0.0f : (ret * 0.50f);
	}
public:
	static s_prober create(std::error_code& ec) noexcept {
		return s_prober( nobadalloc<latin1_prober>::construct(ec) );
	}
	virtual charset get_charset() const noexcept override {
#ifdef __IO_WINDOWS_BACKEND__
		return code_pages::CP_1252;
#else
		return code_pages::ISO_8859_1;
#endif // __IO_WINDOWS_BACKEND__
	}
	virtual bool probe(std::error_code& ec,float& confidence,const uint8_t* buff, std::size_t size) const noexcept override {
		using namespace coding::latin1;
		typedef byte_buffer::iterator buff_it_t;

		uint8_t last_char_class = OTH;
		uint32_t freq_counter[FREQ_CAT_NUM];
		io_zerro_mem(freq_counter, FREQ_CAT_NUM * sizeof(uint32_t) );
		byte_buffer new_buff( filter_with_english_letters(ec, buff, size) );
		if(ec) {
			confidence = 0.0f;
			return false;
		}
		uint8_t char_class;
		uint8_t freq;
		for(buff_it_t it = new_buff.position(); it < new_buff.last(); ++it) {
			char_class = CHAR_TO_CLASS[ static_cast<std::size_t>( *it )  ];
			freq = CLASS_MODEL[last_char_class * CLASS_NUM + char_class];
			if (freq == 0) {
				confidence = 0.0f;
				return false;
			}
			++freq_counter[freq];
			last_char_class = char_class;
		}
		confidence = calc_confidence(freq_counter);
		return confidence >= 1.0f;
	}
};

/*
class single_byte_char_set_prober:public virtual prober {
public:
	typedef coding::single_byte::sequence_model sequence_model_t;
private:
	static constexpr uint16_t SB_ENOUGH_REL_THRESHOLD = 1024;
	static constexpr float POSITIVE_SHORTCUT_THRESHOLD  = 0.95F;
	static constexpr float NEGATIVE_SHORTCUT_THRESHOLD  = 0.05F;
	static constexpr uint8_t SYMBOL_CAT_ORDER = 250;
	static constexpr std::size_t NUMBER_OF_SEQ_CAT = 4;
	static constexpr std::size_t POSITIVE_CAT = (NUMBER_OF_SEQ_CAT-1);
	static constexpr std::size_t NEGATIVE_CAT = 0;
public:
	single_byte_char_set_prober(
	    const sequence_model_t *model,
	    bool reversed,
	    const s_prober& name_prober) noexcept:
		prober(),
		reversed_(reversed),
		last_order_(255),
		total_seqs_(0),
		total_char_(0),
		ctrl_char_(0),
		freq_char_(0),
		model_(model)
	{
		io_zerro_mem(seq_counters_, NUMBER_OF_SEQ_CAT * sizeof(uint32_t) );
	}
	single_byte_char_set_prober(const coding::single_byte::sequence_model *model):
		single_byte_char_set_prober(model, false, s_prober() )
	{}
	virtual bool probe(std::error_code& ec,float& confidence,const uint8_t* buff, std::size_t size) const noexcept override
	{
		typedef byte_buffer::iterator buff_it_t;
		byte_buffer script = byte_buffer::wrap(buff, size);
		if(script.empty()) {
		}
		return false;
	}
protected:
	void reset()
	{
		last_order_ = 255;
		io_zerro_mem(seq_counters_, NUMBER_OF_SEQ_CAT * sizeof(uint32_t) );
		total_seqs_ = 0;
		total_char_ = 0;
		ctrl_char_  = 0;
		freq_char_ = 0;
	}
	float calc_confidence() const noexcept
	{
		float ret = 0.01F;
		if (total_seqs_ > 0) {
			ret = 1.0F * seq_counters_[POSITIVE_CAT] / total_seqs_ / model_->typical_positive_ratio;
			ret *= seq_counters_[POSITIVE_CAT] / total_char_;
			ret *= (total_char_-ctrl_char_) / total_char_;
			ret *= freq_char_ / total_char_;
			if (ret >= 1.00F)
				ret = 0.99F;
		}
		return ret;
	}
private:
	bool reversed_;
	//char order of last character
	uint8_t last_order_;
	uint32_t total_seqs_;
	uint32_t total_char_;
	uint32_t ctrl_char_;
	//characters that fall in our sampling range
	uint32_t freq_char_;
	s_prober name_prober_;
	uint32_t seq_counters_[NUMBER_OF_SEQ_CAT];
	const sequence_model_t *model_;
};
*/

// utf8_prober
class utf8_prober final: public prober {
private:

	static constexpr float ONE_CHAR_PROB = 0.5F;

	friend class nobadalloc<utf8_prober>;

	explicit utf8_prober() noexcept:
		prober()
	{}

	float calc_confidance(uint8_t multibyte_chars) const noexcept {
		float unlike = 0.99F;
		if (multibyte_chars < 6) {
			for (uint8_t i = 0; i < multibyte_chars; i++)
				unlike *= ONE_CHAR_PROB;
			return (1.0F - unlike);
		}
		return 0.99F;
	}

public:
	static s_prober create(std::error_code& ec) noexcept {
		return s_prober( nobadalloc<utf8_prober>::construct(ec) );
	}
	virtual charset get_charset() const noexcept override {
		return code_pages::UTF_8;
	}
	virtual bool probe(std::error_code& ec,float& confidence,const uint8_t* buff, std::size_t size) const noexcept override {
		coding::state_t coding_state;
		coding::state_machine sm( &coding::unicode::UTF8_MODEL );
		uint8_t multibyte_chars = 0;
		for (std::size_t i = 0; i < size; i++) {
			coding_state = sm.next_state( buff[i] );
			switch(coding_state) {
			case coding::state_t::start:
				if( sm.current_char_len() > 2 )
					++multibyte_chars;
				break;
			case coding::state_t::found:
				confidence = 1.0F;
				return true;
			case coding::state_t::error:
			default:
				ec = std::make_error_code( std::errc::illegal_byte_sequence );
				break;
			}
		}
		confidence = calc_confidance(multibyte_chars);
		return false;
	}
};


#ifdef IO_NO_EXCEPTIONS

class bad_alloc_handler {
public:
	static void handle() {
		_bad_alloc.store(true);
	}
	static std::atomic_bool _bad_alloc;
};

std::atomic_bool bad_alloc_handler::_bad_alloc(false);

#endif // IO_NO_EXCEPTIONS

} // detail

//charset_detector
s_charset_detector charset_detector::create(std::error_code& ec) noexcept
{
	detail::s_prober latin1_prob = detail::latin1_prober::create(ec);
	detail::s_prober utf8_prob = detail::utf8_prober::create(ec);
	if(ec)
		return s_charset_detector();
	v_pobers probers( { latin1_prob, utf8_prob} );
	return !ec ?  s_charset_detector(
			   nobadalloc<charset_detector>::construct(ec, std::move(probers) )
		   )
		   : s_charset_detector();
}

charset_detector::charset_detector(v_pobers&& probers) noexcept:
	object(),
	probers_( std::forward<v_pobers>(probers) )
{}

charset_detect_status charset_detector::detect(std::error_code &ec,const uint8_t* buff, std::size_t size) const noexcept
{
	// try unicode byte order mark first
	unicode_cp unicp = detect_by_bom(buff);
	switch(unicp) {
	case unicode_cp::not_detected:
		break;
	case unicode_cp::utf8:
		return charset_detect_status(code_pages::UTF_8, 1.0f);
	case unicode_cp::utf_16be:
		return charset_detect_status(code_pages::UTF_16BE, 1.0f);
	case unicode_cp::utf_16le:
		return charset_detect_status(code_pages::UTF_16LE, 1.0f);
	case unicode_cp::utf_32be:
		return charset_detect_status(code_pages::UTF_32BE, 1.0f);
	case unicode_cp::utf_32le:
		return charset_detect_status(code_pages::UTF_32BE, 1.0f);
	}
	// no unicode byte order mark found, try to detect/guess by evristic
	// algorythms
	float *confidences = static_cast<float*>( io_alloca( probers_.size() ) );
	float confidence;
	std::size_t i = 0;
	for(auto it = probers_.cbegin(); it != probers_.cend(); ++it) {
		if( (*it)->probe(ec, confidence, buff, size) )
			return charset_detect_status((*it)->get_charset(), 1.0F);
		if(ec)
			return charset_detect_status();
		confidences[i++] = confidence;
	}
	std::size_t maxc_ind = 0;
	float m = 0.0F;
	for( i = 0; i < probers_.size(); i++) {
		if( confidences[i] > m) {
			m = confidences[i];
			maxc_ind = i;
		}
	}
	return charset_detect_status(probers_[maxc_ind]->get_charset(), m);
}

} // namespace io
