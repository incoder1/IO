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
#include "stdafx.hpp"
#include "image.hpp"

#include <atomic>
#include <wincodec.h>
#include <shlwapi.h>

inline void intrusive_ptr_add_ref(::IUnknown* const obj) noexcept
{
	obj->AddRef();
}

inline void intrusive_ptr_release(::IUnknown* const obj) noexcept
{
	obj->Release();
}


namespace engine {

// COM
class COM {
	COM(const COM&) = delete;
	COM& operator=(COM&) = delete;
public:
	COM() noexcept:
		initialized_(false)
	{
		initialized_ = SUCCEEDED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE));
	}
	~COM() noexcept {
		if(initialized_)
			::CoUninitialize();
	}
private:
	bool initialized_;
};


// read_stream
class read_stream final: public ::IStream {
	read_stream(const read_stream&) = delete;
	read_stream& operator=(const read_stream&) = delete;
public:
	read_stream(io::s_read_channel&& ch);
	virtual ~read_stream() noexcept;

	virtual ::HRESULT __stdcall QueryInterface(const ::IID & iid, void ** ppv) noexcept override;
	virtual ::ULONG __stdcall AddRef() noexcept override;
	virtual ::ULONG __stdcall Release() noexcept override;

	virtual ::HRESULT __stdcall Read(void * pv, ULONG cb, ULONG * pcbRead) noexcept override;
	virtual ::HRESULT __stdcall Write(const void * pv, ULONG cb, ULONG * pcbWritten) noexcept override;

	virtual ::HRESULT __stdcall Seek(::LARGE_INTEGER dlibMove, ::DWORD dwOrigin,
									 ::ULARGE_INTEGER * plibNewPosition) noexcept override;
	virtual ::HRESULT __stdcall SetSize(::ULARGE_INTEGER libNewSize) noexcept override;
	virtual ::HRESULT __stdcall CopyTo(::IStream * pstm, ::ULARGE_INTEGER cb,
									   ::ULARGE_INTEGER * pcbRead,
									   ::ULARGE_INTEGER * pcbWritten) noexcept override;
	virtual ::HRESULT __stdcall Commit(::DWORD grfCommitFlags) noexcept override;
	virtual ::HRESULT __stdcall Revert() noexcept override;
	virtual ::HRESULT __stdcall LockRegion(::ULARGE_INTEGER libOffset,
										   ::ULARGE_INTEGER cb,
										   ::DWORD dwLockType) noexcept override;
	virtual ::HRESULT __stdcall UnlockRegion(::ULARGE_INTEGER libOffset,
			::ULARGE_INTEGER cb,
			::DWORD dwLockType) noexcept override;
	virtual ::HRESULT __stdcall Stat(::STATSTG * pstatstg,::DWORD grfStatFlag);
	virtual ::HRESULT __stdcall Clone(::IStream ** ppstm) noexcept override;

private:
	const io::s_read_channel ch_;
	::IStream *mem_proxy_;
	std::atomic_ulong ref_count_;
};

read_stream::read_stream(io::s_read_channel&& ch):
	::IStream(),
	ch_( std::forward<io::s_read_channel>(ch) ),
	mem_proxy_(::SHCreateMemStream(nullptr,0)),
	ref_count_(1)
{
	std::size_t read;
	::ULONG written;
	std::error_code ec;
	io::scoped_arr<uint8_t> arr( io::memory_traits::page_size() );
	read = ch_->read( ec, arr.get(), arr.len() );
	while(!ec && read > 0 ) {
		for(std::size_t left = read; left > 0; left -= written) {
			uint8_t *px = arr.get() + (read - left);
			mem_proxy_->Write(px, left, &written);
		}
		read = ch_->read( ec, arr.get(), arr.len() );
	}
	if(ec)
		throw std::system_error(ec);
	::LARGE_INTEGER pos;
	pos.QuadPart = 0UL;
	::ULARGE_INTEGER new_pos;
	mem_proxy_->Seek(pos,STREAM_SEEK_SET, &new_pos );
}

read_stream::~read_stream() noexcept
{
	mem_proxy_->Release();
}

::HRESULT __stdcall read_stream::QueryInterface(const ::IID & iid, void ** ppv) noexcept
{
	if ( nullptr == ppv)
		return E_INVALIDARG;

	if (    iid == __uuidof(IUnknown) ||
		    iid == __uuidof(IStream) ||
		    iid == __uuidof(ISequentialStream)
		)
	{
		*ppv = static_cast<IStream*>(this);
		this->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

::ULONG __stdcall read_stream::AddRef() noexcept
{
	return ref_count_.fetch_add(1, std::memory_order_relaxed) + 1;
}

::ULONG __stdcall read_stream::Release() noexcept
{
	::ULONG ret = ref_count_.fetch_sub(1, std::memory_order_acq_rel);
	if(0 == --ret)
		delete this;
	return ret;
}


::HRESULT __stdcall read_stream::Read(void * pv, ::ULONG cb, ::ULONG * pcbRead) noexcept
{
	return mem_proxy_->Read(pv,cb,pcbRead);
}

::HRESULT __stdcall read_stream::Write(const void * pv, ::ULONG cb, ::ULONG * pcbWritten) noexcept
{
	return E_NOTIMPL;
}

::HRESULT __stdcall read_stream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
									  ULARGE_INTEGER * plibNewPosition) noexcept
{
	return mem_proxy_->Seek(dlibMove,dwOrigin,plibNewPosition);
}

::HRESULT __stdcall read_stream::SetSize(::ULARGE_INTEGER libNewSize) noexcept
{
	return mem_proxy_->SetSize(libNewSize);
}

::HRESULT __stdcall read_stream::CopyTo(::IStream * pstm, ::ULARGE_INTEGER cb,
										::ULARGE_INTEGER * pcbRead,
										::ULARGE_INTEGER * pcbWritten) noexcept
{
	return mem_proxy_->CopyTo(pstm,cb,pcbRead,pcbWritten);
}

::HRESULT __stdcall read_stream::Commit(::DWORD grfCommitFlags) noexcept
{
	return mem_proxy_->Commit(grfCommitFlags);
}

::HRESULT __stdcall read_stream::Revert() noexcept
{
	return mem_proxy_->Revert();
}

::HRESULT __stdcall read_stream::LockRegion(::ULARGE_INTEGER libOffset,
		::ULARGE_INTEGER cb,
		::DWORD dwLockType) noexcept
{
	return mem_proxy_->LockRegion(libOffset, cb, dwLockType);
}

::HRESULT __stdcall read_stream::UnlockRegion(::ULARGE_INTEGER libOffset,
		ULARGE_INTEGER cb,
		DWORD dwLockType) noexcept
{
	return mem_proxy_->UnlockRegion(libOffset,cb,dwLockType);
}

::HRESULT __stdcall read_stream::Stat(::STATSTG * pstatstg, ::DWORD grfStatFlag)
{
	return  mem_proxy_->Stat(pstatstg,grfStatFlag);
}

::HRESULT __stdcall read_stream::Clone(::IStream ** ppstm) noexcept
{
	return  mem_proxy_->Clone(ppstm);
}

// s_image

DECLARE_IPTR(IWICImagingFactory);
DECLARE_IPTR(IWICBitmapDecoder);
DECLARE_IPTR(IWICFormatConverter);
DECLARE_IPTR(IWICBitmap);
DECLARE_IPTR(IWICBitmapLock);
DECLARE_IPTR(IStream);

static void validate_succeeded(::HRESULT errc, const std::string& msg)
{
	if( !SUCCEEDED(errc) )
		throw std::runtime_error(msg);
}

static s_IWICImagingFactory create_image_factory()
{
	::IWICImagingFactory *ret;
	::HRESULT errc = ::CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&ret));
	validate_succeeded(errc,"No Direct2D image decoding supported");
	return s_IWICImagingFactory(ret, false);
}

static s_IWICBitmapDecoder create_bitmap_decoder(const s_IWICImagingFactory& imgFactory,const s_IStream& src)
{
	::IWICBitmapDecoder *ret;
	::HRESULT errc = imgFactory->CreateDecoderFromStream(src.get(),nullptr,WICDecodeMetadataCacheOnDemand,&ret);
	validate_succeeded(errc,"Can't create decoder form provided stream data");
	return s_IWICBitmapDecoder(ret);
}

static s_IWICFormatConverter create_format_converter(const s_IWICImagingFactory& imgFactory,const s_IWICBitmapDecoder& decoder,const pixel_format pfm)
{
	::IWICFormatConverter *pConverter;
	::HRESULT errc = imgFactory->CreateFormatConverter(&pConverter);
	validate_succeeded(errc,"Decoding of this format not supported");
	s_IWICFormatConverter ret(pConverter);
	::IWICBitmapFrameDecode* frame;
	errc = decoder->GetFrame(0, &frame);
	validate_succeeded(errc,"Can't obtain frame");
	::GUID dstFormat;
	switch(pfm) {
	case pixel_format::rgb:
		dstFormat = ::GUID_WICPixelFormat24bppRGB;
		break;
	case pixel_format::rgba:
		dstFormat = ::GUID_WICPixelFormat32bppRGBA;
		break;
	case pixel_format::bgra:
		dstFormat = ::GUID_WICPixelFormat32bppBGRA;
		break;
	}
	errc = ret->Initialize(
			 frame,
			 dstFormat,
			 WICBitmapDitherTypeNone,
			 nullptr,// Specify a particular palette
			 0.0F,// Alpha threshold
			 ::WICBitmapPaletteTypeCustom// Palette translation type
		 );
	validate_succeeded(errc,"Image decoding failed");
	return ret;
}

static s_IWICBitmap create_bitmap(const s_IWICImagingFactory& imgFactory, s_IWICFormatConverter& converter)
{
	IWICBitmap *ret;
	::HRESULT hr = imgFactory->CreateBitmapFromSource(converter.get(),WICBitmapNoCache,&ret);
	if(!SUCCEEDED(hr))
		throw std::bad_array_new_length();
	return s_IWICBitmap(ret);
}

static s_IWICBitmapLock bitmap_lock(const s_IWICBitmap& bitmap, ::WICRect& rc_lock)
{
	IWICBitmapLock *ret;
	HRESULT hr = bitmap->Lock( std::addressof(rc_lock), WICBitmapLockWrite, &ret);
	validate_succeeded(hr,"Can't obtain data from bitmap");
	return s_IWICBitmapLock(ret);
}

static io::scoped_arr<uint8_t> flip_vertically(const ::BYTE* origin,const std::size_t size,const std::size_t line_stride)
{
	io::scoped_arr<uint8_t> ret(size);
	uint8_t* dst = const_cast<uint8_t*>( ret.get() );
	const uint8_t* src = origin;
	const std::size_t lines_count = size / line_stride;
	#ifdef _OPENMP
	#pragma omp parallel for simd
	#endif // _OPENMP
	for(std::size_t i = 0; i < lines_count; i++) {
		uint8_t *d = dst + (line_stride * (lines_count-i-1) );
		const uint8_t *s = src + (i*line_stride);
		std::memcpy( d, s, line_stride);
	}
	return ret;
}


static io::scoped_arr<uint8_t> decode_image(const s_IStream& stream, unsigned int& w, unsigned int& h, const pixel_format pfm, bool vertical_flip) {

	// CoInitializeEx should be called for each thread
	static thread_local COM __ms_com_guard;

	s_IWICImagingFactory imgFactory = create_image_factory();
	s_IWICBitmapDecoder decoder = create_bitmap_decoder(imgFactory, stream);
	s_IWICFormatConverter converter = create_format_converter(imgFactory, decoder, pfm);
	s_IWICBitmap bit_map = create_bitmap(imgFactory,converter);

	bit_map->GetSize( std::addressof(w), std::addressof(h) );
	::WICRect rc_lock = { 0, 0,  static_cast<::INT>(w), static_cast<::INT>(h) };
	s_IWICBitmapLock lock = bitmap_lock(bit_map,rc_lock);

	::UINT cbBufferSize = 0, cbStride = 0;
	::BYTE *pv = nullptr;
	::HRESULT errc = lock->GetStride(&cbStride);
	errc = lock->GetDataPointer(&cbBufferSize, &pv);
	validate_succeeded(errc,"Can't obtain data from bitmap");
	return  vertical_flip
		? flip_vertically(pv, cbBufferSize, cbStride)
		: io::scoped_arr<uint8_t>(pv, cbBufferSize);
}


s_image load_png_rgb(io::s_read_channel&& src, bool vertical_flip)
{
	s_IStream stream( new read_stream( std::forward<io::s_read_channel>(src)), false);
	unsigned int w,h;
	io::scoped_arr<uint8_t> image_data = decode_image( stream, w, h, pixel_format::rgb, vertical_flip );
	return  s_image(new image( w, h, image_format::bitmap, pixel_format::rgb, std::move(image_data) ) );
}

s_image load_png_rgb(const io::file& file,bool vertical_flip)
{
	IStream* src;
	::HRESULT errc = ::SHCreateStreamOnFileEx(file.wpath().data(), STGM_READ, FILE_ATTRIBUTE_READONLY, FALSE, nullptr, &src);
	validate_succeeded(errc, std::string("Can not open image file: ").append(file.path()) );
	unsigned int w,h;
	io::scoped_arr<uint8_t> image_data = decode_image( s_IStream(src,false), w, h, pixel_format::rgb, vertical_flip );
	return s_image(new image( w, h, image_format::bitmap, pixel_format::rgb, std::move(image_data) ) );
}

s_image load_png_rgba(io::s_read_channel&& src, bool vertical_flip)
{
	 s_IStream stream( new read_stream( std::forward<io::s_read_channel>(src)), false);
	unsigned int w,h;
	io::scoped_arr<uint8_t> image_data = decode_image( stream, w, h, pixel_format::rgb, vertical_flip );
	return  s_image(new image( w, h, image_format::bitmap, pixel_format::rgba, std::move(image_data) ) );
}

s_image load_png_rgba(const io::file& file,bool vertical_flip)
{
	IStream* src;
	::HRESULT errc = ::SHCreateStreamOnFileEx(file.wname().data(), STGM_READ, FILE_ATTRIBUTE_READONLY, FALSE, nullptr, &src);
	validate_succeeded(errc, std::string("Can not open image file: ").append(file.path()));
	unsigned int w,h;
	io::scoped_arr<uint8_t> image_data = decode_image( s_IStream(src,false), w, h, pixel_format::rgba, vertical_flip );
	return s_image(new image( w, h, image_format::bitmap, pixel_format::rgba, std::move(image_data) ) );
}


} // namespace engine
