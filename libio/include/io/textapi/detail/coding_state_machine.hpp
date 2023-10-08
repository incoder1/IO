#ifndef __IO_TEXT_API_CODING_STATE_MACHINE_HPP_INCLUDED__
#define __IO_TEXT_API_CODING_STATE_MACHINE_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <io/textapi/charsets.hpp>

#include "pkg_int.hpp"

namespace io {

namespace detail {

// coding state mashine
class coding_state_machine {
public:

    enum class state_t {
        start = 0,
        error = 1,
        found = 2
    };

    struct model_t {
        pkg_int class_table;
        uint32_t class_factor;
        pkg_int state_table;
        const uint32_t* char_len_table;
#ifndef NDEBUG
		std::size_t char_len_table_length;
#endif // NO_DEBUG
        const std::size_t charset_code;
        inline uint32_t byte_class(uint8_t c) const noexcept
        {
        	return class_table.get( c );
        }
        inline uint32_t state_to_byte_class(uint32_t previews_state, uint32_t byte_class) const noexcept
        {
			uint32_t factor = (previews_state * class_factor) + byte_class;
			return state_table.get( factor );
        }
    };

    explicit coding_state_machine(const model_t* mdl) noexcept;

    state_t next_state(const char c) noexcept;

    void reset() noexcept;

    uint32_t current_char_len() const noexcept
    {
        return current_char_len_;
    }

    const std::size_t charset_code() const noexcept
    {
        return model_->charset_code;
    }

private:
    const model_t* model_;
    state_t current_state_;
    uint32_t current_char_len_;
    uint32_t current_byte_pos_;
};

} // namespace detail

} // namespace io

#endif // __IO_TEXT_API_CODING_STATE_MACHINE_HPP_INCLUDED__
