/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IMAGE_HPP_INCLUDED__
#define __IMAGE_HPP_INCLUDED__

#include <object.hpp>
#include <channels.hpp>
#include <files.hpp>
#include <scoped_array.hpp>

namespace engine {

	enum class image_format {
		dds,
		bitmap
	};

	enum class pixel_format {
		rgb,
		rgba,
		bgra
	};

	class image;
	DECLARE_IPTR(image);
	class image final: public io::object {
	public:
		image(std::size_t w, std::size_t h,image_format fmt,pixel_format pfm, io::scoped_arr<uint8_t>&& data) noexcept;

		const std::size_t width() const noexcept {
			return width_;
		}
		const std::size_t height() const noexcept {
			return height_;
		}

		pixel_format pix_format() const noexcept {
			return pfm_;
		}

		image_format format() const noexcept {
			return format_;
		}

		const uint8_t* data() const noexcept {
			return data_.get();
		}
	private:
		std::size_t width_;
		std::size_t height_;
		image_format format_;
		pixel_format pfm_;
		io::scoped_arr<uint8_t> data_;
	};


	s_image load_png_rgb(io::s_read_channel&& src, bool vertical_flip);
	s_image load_png_rgb(const io::file& file, bool vertical_flip);

	s_image load_png_rgba(io::s_read_channel&& src, bool vertical_flip);
	s_image load_png_rgba(const io::file& file, bool vertical_flip);


} // namespace engine


#endif // __IMAGE_HPP_INCLUDED__
