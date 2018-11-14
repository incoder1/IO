#ifndef __IMAGE_HPP_INCLUDED__
#define __IMAGE_HPP_INCLUDED__

#include <object.hpp>
#include <buffer.hpp>
#include <channels.hpp>
#include <files.hpp>

namespace engine {

	enum class image_format {
		BMP,
		DDS,
		EXR,
		GIF,
		HDR,
		JPEG,
        PNG,
        TGA,
        TIFF
	};

	enum class pixel_format {
		rgb,
		rgba
	};

	class image;
	DECLARE_IPTR(image);
	class image final: public io::object {
	private:
		image(std::size_t w, std::size_t h,pixel_format pfm, io::byte_buffer&& data) noexcept;
	public:

		static s_image load_rgb(io::s_read_channel&& src, image_format format);
		static s_image load_rgb(const io::file& file, image_format format);

		static s_image load_rgba(io::s_read_channel&& src, image_format format);
		static s_image load_rgba(const io::file& file, image_format format);


		const std::size_t width() const noexcept {
			return width_;
		}
		const std::size_t height() const noexcept {
			return height_;
		}

		const pixel_format pix_format() const noexcept {
			return pfm_;
		}

		const uint8_t* data() const noexcept {
			return data_.position().get();
		}
	private:
		std::size_t width_;
		std::size_t height_;
		pixel_format pfm_;
		io::byte_buffer data_;
	};

} // namespace engine


#endif // __IMAGE_HPP_INCLUDED__
