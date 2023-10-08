#ifndef __IO_TEXT_API_DETAIL_LATIN1_PROBER_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_LATIN1_PROBER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <array>

#include "prober.hpp"

namespace io {

namespace detail {

class latin1_prober final: public prober {
private:
    static constexpr std::size_t FREQ_CATS_COUNT = 4;
    static constexpr std::size_t CLASS_NUM = 8;
    // undefine
    static constexpr uint8_t UDF = 0;
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
    static const std::array<uint8_t,256> CHAR_TO_CLASS;
    static const std::array<uint8_t,64> CLASS_MODEL;
    latin1_prober() noexcept;
public:
    static s_prober create(std::error_code& ec) noexcept;
    virtual uint16_t get_charset_code() const noexcept override;
    virtual prober::state_t handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept override;
    virtual prober::state_t state() noexcept override;
    virtual void reset() noexcept override;
    virtual float confidence() noexcept override;
private:
    prober::state_t state_;
    std::size_t last_char_class_;
    std::array<std::size_t,FREQ_CATS_COUNT> freq_counter_;
};

} // namespace detail

} // namespace io

#endif // __IO_TEXT_API_DETAIL_LATIN1_PROBER_HPP_INCLUDED__
