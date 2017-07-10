#include <console.hpp>
#include <files.hpp>
#include <iostream>

#include <text.hpp>
#include <stream.hpp>

void check_error(const std::error_code& ec) {
	if(ec) {
		io::channel_ostream<wchar_t> ucerr( io::console::err() );
		ucerr<< io::transcode_to_ucs( ec.message().data() ) << std::endl;
		std::exit( -1 );
    }
}

io::s_write_channel prepare_file() {
	using namespace io;
	// Open a file to write results
	std::error_code ec;
	file f = file::get(ec, "result.txt");
	check_error(ec);
	s_write_channel fch = f.open_for_write(ec, io::write_open_mode::overwrite);
	check_error(ec);
	// write byte order mark
	fch->write(ec, utf8_bom::data(), utf8_bom::len());
	check_error(ec);
	// open converting write channel, and stream on top of it
	s_code_cnvtr cvt = code_cnvtr::open(ec, IO_SYS_UNICODE, code_pages::UTF_8);
	check_error(ec);
	s_write_channel ret=  new_text_conv(ec, fch, cvt);
	check_error(ec);
	return ret;
}

int main()
{
	const wchar_t* message = L"Hello!\nПривет!\nПривіт!\nΧαιρετίσματα!\nHelló!\nHallå!\nこんにちは!\n您好!\n";
	io::channel_ostream<wchar_t> fout( prepare_file() );
	fout << message << std::endl;
	//fout.flush();

	io::console::reset_colors( io::text_color::yellow, io::text_color::light_green,  io::text_color::light_red );

	io::channel_ostream<wchar_t> ucout( io::console::out() );
	ucout << message << 1234567890ull << L'\n' << 123456.78e+09 << std::endl;

	// io::channel_istream<wchar_t> ucin( io::console::in() );
	//ucout <<  L"Type something to echo:" ;
	//ucout.flush();
	//wchar_t typed[64];
	//io_memset(typed, 0, 64*sizeof(wchar_t) );
	//ucin >> typed;
	//std::error_code ec;
	//ucout << L"You've typed: ";
	//ucout.flush();
	//io::channel_ostream<wchar_t> ucerr( io::console::err() );
	//ucerr << typed << std::endl;
    return 0;
}
