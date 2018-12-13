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
		rgba
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
		pixel_format pfm_;
		image_format format_;
		io::scoped_arr<uint8_t> data_;
	};


	s_image load_png_rgb(io::s_read_channel&& src);
	s_image load_png_rgb(const io::file& file);

	s_image load_png_rgba(io::s_read_channel&& src);
	s_image load_png_rgba(const io::file& file);

} // namespace engine


#endif // __IMAGE_HPP_INCLUDED__
