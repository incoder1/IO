#include "stdafx.hpp"
#include "posix/console.hpp"

namespace io {

namespace posix {

//text_color

#define DECLARE_COLOR( _C_NAME, __attr) \
	static const uint8_t* (_C_NAME) = reinterpret_cast<const uint8_t*>( (__attr) )

DECLARE_COLOR(NAVY_BLUE,"\033[22;34m");
DECLARE_COLOR(NAVY_GREEN,"\033[22;32m");
DECLARE_COLOR(NAVY_AQUA, "\033[22;36m");
DECLARE_COLOR(NAVY_RED,"\033[22;31m");
DECLARE_COLOR(MAGENTA,"\033[22;35m");
DECLARE_COLOR(BROWN,"\033[22;33m");
DECLARE_COLOR(WHITE,"\033[22;37m");
DECLARE_COLOR(GRAY,"\033[01;30m");
DECLARE_COLOR(LIGHT_BLUE,"\033[01;34m");
DECLARE_COLOR(LIGHT_GREEN,"\033[01;32m");
DECLARE_COLOR(LIGHT_AQUA,"\033[01;36m");
DECLARE_COLOR(LIGHT_RED,"\033[01;31m");
DECLARE_COLOR(PURPLE,"\033[01;35m");
DECLARE_COLOR(YELLOW,"\033[01;33m");
DECLARE_COLOR(BRIGHT_WHITE,"\033[01;37m");
DECLARE_COLOR(RESET,"\033[0m");
#undef DECLARE_COLOR

static const std::size_t RESET_LEN = io_strlen( reinterpret_cast<const char*>(RESET) );

static const  uint8_t* COLOR_TABLE[15] = {
	NAVY_BLUE,
	NAVY_GREEN,
	NAVY_AQUA,
	NAVY_RED,
	MAGENTA,
	BROWN,
	WHITE,
	GRAY,
	LIGHT_BLUE,
	LIGHT_GREEN,
	LIGHT_AQUA,
	LIGHT_RED,
	PURPLE,
	YELLOW,
	BRIGHT_WHITE
};

console_channel::console_channel(const fd_t stream) noexcept:
	read_write_channel(),
	stream_(stream),
	color_(RESET),
	color_len_( RESET_LEN )
{}

std::size_t console_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	std::size_t ret;
	if(RESET != color_) {
		if( !::write(STDOUT_FILENO, color_, color_len_) ) {
			ec.assign(errno, std::system_category() );
			return 0;
		}
	}
	::ssize_t result = ::read(stream_, static_cast<void*>(buff), bytes);
	if(result < 0)
		ec.assign(errno, std::system_category() );
	ret = static_cast<size_t>(result);
	if(RESET != color_) {
		if(! ::write(STDOUT_FILENO, RESET, RESET_LEN ) ) {
			ec.assign(errno, std::system_category() );
			return 0;
		}
	}
	return ret;
}

std::size_t console_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	if(RESET != color_) {
		if( !::write(stream_, color_, color_len_) ) {
			ec.assign(errno, std::system_category() );
			return 0;
		}
	}
	::ssize_t result = ::write(stream_, buff, size);
	if(result < 0) {
		ec.assign(errno, std::system_category() );
		return 0;
	}
	if(RESET != color_) {
		if(! ::write(stream_, RESET, RESET_LEN ) ) {
			ec.assign(errno, std::system_category() );
			return 0;
		}
	}
	return static_cast<std::size_t>(result);
}

void console_channel::change_color(text_color cl) noexcept
{
	color_ = COLOR_TABLE[ static_cast<std::size_t>(cl) ];
	color_len_ = io_strlen( reinterpret_cast<const char*>( color_ ) );
}


} // namespace posix


// console
std::atomic<console*> console::_instance(nullptr);
critical_section console::_cs;

void console::release_console() noexcept
{
	console* inst = _instance.load( std::memory_order_seq_cst );
	inst->~console();
	memory_traits::free( inst );
}

const console* console::get()
{
	console *tmp = _instance.load( std::memory_order_relaxed );
	if( nullptr == tmp ) {
		tmp = _instance.load( std::memory_order_acquire );
		lock_guard lock(_cs);
		if( nullptr == tmp ) {
			void *raw = memory_traits::malloc( sizeof(console) );
			if(nullptr != raw) {
				tmp = new (raw) console();
				_instance.store( tmp, std::memory_order_release );
				std::atexit( &console::release_console );
			}
			std::atomic_thread_fence( std::memory_order_release );
		}
	}
	return tmp;
}

console::console():
	in_( new posix::console_channel(STDIN_FILENO) ),
	out_( new posix::console_channel(STDOUT_FILENO) ),
	err_( new posix::console_channel(STDERR_FILENO) )
{}

void console::reset_colors(const text_color in,const text_color out,const text_color err) noexcept
{
	console *con = const_cast<console*>( get() );
	lock_guard lock(_cs);
	posix::s_console_channel cch = con->in_;
	cch->change_color(in);
	cch = con->out_;
	cch->change_color(out);
	cch = con->err_;
	cch->change_color(err);
}

void console::reset_in_color(const text_color clr) noexcept
{
	console *con = const_cast<console*>( get() );
	lock_guard lock(_cs);
	con->in_->change_color(clr);
}

void console::reset_out_color(const text_color clr) noexcept
{
	console *con = const_cast<console*>( get() );
	lock_guard lock(_cs);
	con->out_->change_color(clr);
}

void console::reset_err_color(const text_color clr) noexcept
{
	console *con = const_cast<console*>( get() );
	lock_guard lock(_cs);
	con->err_->change_color(clr);
}

std::ostream& console::out_stream()
{
	static io::channel_ostream<char> _cout( get()->out_ );
	return _cout;
}

std::ostream& console::error_stream()
{
	static io::channel_ostream<char> _cout( get()->err_ );
	return _cout;
}



static s_write_channel conv_channel(const s_write_channel& ch)
{
#ifdef IO_IS_LITTLE_ENDIAN
	static const charset SYSTEM_WIDE = code_pages::UTF_32LE;
#else
	static const charset SYSTEM_WIDE = code_pages::UTF_32BE;
#endif // IO_IS_LITTLE_ENDIAN
	std::error_code ec;
	s_code_cnvtr conv = code_cnvtr::open(ec,
										SYSTEM_WIDE,
										code_pages::UTF_8,
										cnvrt_control::discard_on_failing_chars);
	io::check_error_code( ec );
	s_write_channel result = conv_write_channel::open(ec, ch, conv);
	io::check_error_code( ec );
	return result;
}

std::wostream& console::out_wstream()
{
	static io::channel_ostream<wchar_t> _wcout( conv_channel( get()->out_ ) );
	return _wcout;
}

std::wostream& console::error_wstream()
{
	static io::channel_ostream<wchar_t> _wcerr( conv_channel( get()->err_ ) );
	return _wcerr;
}

} // namespace io
