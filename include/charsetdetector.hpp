#ifndef __IO_CHARSET_DETECTOR_HPP_INCLUDED__
#define __IO_CHARSET_DETECTOR_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "buffer.hpp"
#include "charsets.hpp"
#include "object.hpp"

namespace io {

namespace detail {

class prober;
DECLARE_IPTR(prober);

class prober: public object
{
    prober(const prober&) = delete;
    prober& operator=(const prober&) = delete;
protected:
	constexpr prober() noexcept:
		object()
	{}
public:
	virtual charset get_charset() const noexcept = 0;
	virtual bool probe(float& confidence, const uint8_t* buff, std::size_t size) const noexcept = 0;
};


} // namesapce detail


class IO_PUBLIC_SYMBOL charset_detector {
	charset_detector(const charset_detector&) = delete;
	charset_detector& operator=(const charset_detector&) = delete;
public:
	charset_detector();
	bool detect(charset& detected, charset& likely, const uint8_t* buff, std::size_t size) noexcept;
private:
	detail::s_prober probers_[1];
};

} // namespace io

#endif // __IO_CHARSET_DETECTOR_HPP_INCLUDED__
