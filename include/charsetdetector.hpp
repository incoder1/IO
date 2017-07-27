#ifndef __IO_CHARSET_DETECTOR_HPP_INCLUDED__
#define __IO_CHARSET_DETECTOR_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "buffer.hpp"
#include "charsets.hpp"
#include "charsetcvt.hpp"
#include "object.hpp"

#include <vector>

namespace io {

namespace detail {

class prober;
DECLARE_IPTR(prober);

class prober: public virtual object {
	prober(const prober&) = delete;
	prober& operator=(const prober&) = delete;
protected:
	prober() noexcept;
	//This filter applies to all scripts which do not use English characters
	byte_buffer filter_without_english_letters(std::error_code& ec, const uint8_t* buff, std::size_t size) const noexcept;
	//This filter applies to all scripts which contain both English characters and upper ASCII characters
	byte_buffer filter_with_english_letters(std::error_code& ec, const uint8_t* buff, std::size_t size) const noexcept;
public:
	virtual charset get_charset() const noexcept = 0;
	virtual bool probe(std::error_code& ec,float& confidence, const uint8_t* buff, std::size_t size) const noexcept = 0;
};


} // namesapce detail

class charset_detector;
DECLARE_IPTR(charset_detector);

class charset_detect_status {
public:
	constexpr charset_detect_status(const charset& ch, float confidence) noexcept:
		charset_(ch),
		confidence_(confidence)
	{}
	constexpr charset_detect_status() noexcept:
		charset_detect_status(charset(), 0.0F)
	{}
	operator bool() const noexcept
	{
		return 1.0F == confidence_;
	}
	inline charset character_set() const noexcept {
		return charset_;
	}
	inline float confidence() const noexcept {
		return confidence_;
	}
private:
	charset charset_;
	float confidence_;
};

class IO_PUBLIC_SYMBOL charset_detector:public object {
	charset_detector(const charset_detector&) = delete;
	charset_detector& operator=(const charset_detector&) = delete;
private:
	friend class nobadalloc<charset_detector>;
	typedef std::vector<detail::s_prober, h_allocator<detail::s_prober, memory_traits > > v_pobers;
	explicit charset_detector(v_pobers&& probers) noexcept;
public:
	static s_charset_detector create(std::error_code& ec) noexcept;
	charset_detect_status detect(std::error_code& ec, const uint8_t* buff, std::size_t size) const noexcept;
private:
	v_pobers probers_;
};

} // namespace io

#endif // __IO_CHARSET_DETECTOR_HPP_INCLUDED__
