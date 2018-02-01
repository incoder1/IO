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
#include "unicode_bom.hpp"

#include <array>
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

/// Contains status of character set detection
class charset_detect_status {
public:
	constexpr charset_detect_status() noexcept:
		charset_(),
		confidence_(0.0F)
	{}
	charset_detect_status(const charset& ch, float confidence) noexcept:
		charset_( ch ),
		confidence_(confidence)
	{}
	/// Returns true if and only if detector 100% sure about detection accuracy
	/// \return whether detector 100% sure about accuracy
	operator bool() const noexcept
	{
		return 1.0F == confidence_;
	}
	/// Returns detected character set (code page)
	/// \return detected character set (code page)
	inline charset character_set() const noexcept {
		return charset_;
	}
	/// Returns detection accuracy confidence, to get percentage value mull on 100.0F
	/// \return detection accuracy confidence
	inline float confidence() const noexcept {
		return confidence_;
	}
private:
	charset charset_;
	float confidence_;
};

/// Universal character set detector
/// Based on Mozilla Universal character detector source code
/// but with C++ 11 implementation, and improvements
class IO_PUBLIC_SYMBOL charset_detector:public object {
	charset_detector(const charset_detector&) = delete;
	charset_detector& operator=(const charset_detector&) = delete;
private:
	friend class nobadalloc<charset_detector>;
	typedef std::array<detail::s_prober, 2> v_pobers;
	explicit charset_detector(v_pobers&& probers) noexcept;
public:
	/// Creates new intrusive pointer on charset_detector object
	/// \param ec in case of error code, more likely will be out of memory error
	/// \return intrusive pointer on charset_detector object, or on null_ptr object in case of error
	/// \throw never throws, including no throwing on bad_alloc
	static s_charset_detector create(std::error_code& ec) noexcept;
	/// Detect or guess character set (code page) from a portion of bytes
	/// \param ec will be set in case of error (more likely in case of our of memory only)
	/// \param buff a portion of bytes to detect a character set
	/// \param size a count of bytes will be used to detect character set
	/// \return detection result \see charset_detect_status
	charset_detect_status detect(std::error_code& ec, const uint8_t* buff, std::size_t size) const noexcept;
private:
	v_pobers probers_;
};

} // namespace io

#endif // __IO_CHARSET_DETECTOR_HPP_INCLUDED__
