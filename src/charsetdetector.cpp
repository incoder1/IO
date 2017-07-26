#include "stdafx.hpp"
#include "charsetdetector.hpp"

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

/*
#define GETFROMPCK(i, c) \
( \
	( \
		(((c).data)[(i)>>(c).idxsft]) \
		>> \
		(((i)&(c).sftmsk)<<(c).bitsft) \
	)&(c).unitmsk \
) */

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

class state_machine: public object {
	state_machine(const state_machine&) = delete;
	state_machine&  operator=(const state_machine&) = delete;
private:
	friend class nobadalloc<state_machine>;
	explicit constexpr state_machine(const model_t* model) noexcept:
		object(),
		model_(model),
		state_(state_t::start),
		char_len_(0),
		byte_pos_(0)
	{}
public:

	static s_state_machine create(std::error_code& ec, const model_t* model) noexcept
	{
		return nobadalloc<state_machine>::construct( ec, model );
	}

	state_t next_state(uint8_t c) noexcept;

	inline uint8_t current_char_len() const noexcept
	{
		return char_len_;
	}

private:
	const model_t* model_;
	state_t state_;
	uint8_t  char_len_;
	std::size_t byte_pos_;
};

state_t state_machine::next_state(uint8_t c) noexcept
{
	// for each byte we get its class,
	// if it is first byte, we also get byte length
	uint32_t byte_class = get_class( c, model_);
	if(state_ == state_t::start) {
		byte_pos_ = 0;
		char_len_ = model_->char_len_table[ byte_class ];
	}
	//from byte's class and stateTable, we get its next state
	uint32_t ind = static_cast<uint32_t>(state_) * (model_->class_factor) + byte_class;
	state_ = static_cast<state_t>( get_from_pck( ind, model_->state_table ) );
	++byte_pos_;
	return  state_;
}
/*
#define PCK16BITS(a,b)            ((PRUint32)(((b) << 16) | (a)))

#define PCK8BITS(a,b,c,d)         PCK16BITS( ((PRUint32)(((b) << 8) | (a))),  \
                                             ((PRUint32)(((d) << 8) | (c))))

#define PCK4BITS(a,b,c,d,e,f,g,h) PCK8BITS(  ((PRUint32)(((b) << 4) | (a))), \
                                             ((PRUint32)(((d) << 4) | (c))), \
                                             ((PRUint32)(((f) << 4) | (e))), \
											 ((PRUint32)(((h) << 4) | (g))) )
*/

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

static uint32_t UTF8_class [ 32 ] = {
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

} // namespace coding

namespace detail {

class utf8_prober final: public prober {
private:

	static constexpr float ONE_CHAR_PROB = 0.5F;

	friend class nobadalloc<utf8_prober>;
	explicit utf8_prober(coding::s_state_machine&& sm) noexcept:
		prober(),
		sm_(sm)
	{}

	float calc_confidance(uint8_t multibyte_chars) const noexcept
	{
		float unlike = 0.99F;
		if (multibyte_chars < 6) {
			for (uint8_t i = 0; i < multibyte_chars; i++)
				unlike *= ONE_CHAR_PROB;
			return (1.0F - unlike);
		}
		return 0.99F;
	}

public:
	static s_prober create(std::error_code& ec) noexcept
	{
		coding::s_state_machine sm( coding::state_machine::create(ec, &coding::UTF8_MODEL ) );
		utf8_prober *ret = nobadalloc<utf8_prober>::construct(ec, std::move(sm) );
		return s_prober( ret );
	}
	virtual charset get_charset() const noexcept override
	{
        return code_pages::UTF_8;
	}
	virtual bool probe(float& confidence,const uint8_t* buff, std::size_t size) const noexcept override;
private:
	coding::s_state_machine sm_;
};

bool utf8_prober::probe(float& confidence,const uint8_t* buff, std::size_t size) const noexcept
{
	coding::state_t coding_state;
	uint8_t multibyte_chars = 0;
	for (std::size_t i = 0; i < size; i++) {
		coding_state = sm_->next_state( buff[i] );
		switch(coding_state) {
		case coding::state_t::start:
			if( sm_->current_char_len() > 2 )
				++multibyte_chars;
			break;
		case coding::state_t::found:
			return true;
		case coding::state_t::error:
		default:
			break;
		}
	}
	confidence = calc_confidance(multibyte_chars);
	return false;
}

} // detail



} // namespace io
