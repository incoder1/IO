#include "stdafx.hpp"
#include "io/textapi/detail/coding_state_machine.hpp"

namespace io {

namespace detail {

// coding_state_machine
coding_state_machine::coding_state_machine(const model_t* model) noexcept:
    model_( model ),
    current_state_(state_t::start),
    current_char_len_(0),
    current_byte_pos_(0)
{}

coding_state_machine::state_t coding_state_machine::next_state(const char c) noexcept
{
	uint32_t byte_class = model_->byte_class( static_cast<uint8_t>(c) );
	if(state_t::start == current_state_ ) {
		current_byte_pos_ = 0;
		current_char_len_ = model_->char_len_table[byte_class];
		assert(byte_class < model_->char_len_table_length);
	}
	current_state_ = static_cast<state_t> ( model_->state_to_byte_class( static_cast<uint32_t>(current_state_), byte_class) );
	++current_byte_pos_;
	return current_state_;
}

void coding_state_machine::reset() noexcept
{
    current_state_ = state_t::start;
}

} // namespace detail

} // namespace io

