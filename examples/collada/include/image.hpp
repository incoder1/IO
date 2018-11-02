#ifndef __IMAGE_HPP_INCLUDED__
#define __IMAGE_HPP_INCLUDED__

#include <object.hpp>
#include <buffer.hpp>
#include <channels.hpp>

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

	class image;
	DECLARE_IPTR(image);
	class image final: public io::object {
	private:
		image(std::size_t w, std::size_t h, unsigned short stride,io::byte_buffer&& data) noexcept;
	public:
		static s_image load(io::s_read_channel&& src, image_format format);
		const std::size_t width() const noexcept {
			return width_;
		}
		const std::size_t heigh() const noexcept {
			return height_;
		}
		const unsigned short stride() const noexcept {
			return stride_;
		}
		const uint8_t* data() const noexcept {
			return data_.position().get();
		}
	private:
		std::size_t width_;
		std::size_t height_;
		unsigned short stride_;
		io::byte_buffer data_;
	};

} // namespace engine


#endif // __IMAGE_HPP_INCLUDED__
