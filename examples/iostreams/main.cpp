
#ifdef _WIN32
#define _WIN32_WINNT 0x0600
#endif // _WIN32

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

/// Opens a file channel with auto-reconverting content
/// to UTF-8 from current wchar_t unicode
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
	s_write_channel ret= conv_write_channel::open(ec, fch, cvt);
	check_error(ec);
	return ret;
}

const char* umessage = "Hello!\nПривет!\nПривіт!\nΧαιρετίσματα!\nHelló!\nHallå!\nこんにちは!\n您好!\n";
const wchar_t* wmessage = L"Hello!\nПривет!\nПривіт!\nΧαιρετίσματα!\nHelló!\nHallå!\nこんにちは!\n您好!\n";

int main()
{
	io::channel_ostream<wchar_t> fout( prepare_file() );
	fout << wmessage << 1234567890ull << L'\n' << 123456.78e+09 << L'\n' << 12356.789e+10L << std::endl;
	io::console::reset_colors( io::text_color::yellow, io::text_color::light_green,  io::text_color::light_red );

	std::ostream& cout = io::console::out_stream();
	std::ostream& cerr = io::console::error_stream();
	cout << "You can found the same message in result.txt file " << std::endl;
	cout << umessage << 1234567890ull << '\n' << 123456.78e+09 << '\n' << 12356.789e+10L << std::endl;
	cerr << "No errors so far" << std::endl;

	std::wostream& wcout = io::console::out_wstream();
	std::wostream& wcerr = io::console::error_wstream();
	io::console::reset_out_color(io::text_color::white);
	wcout << L"Wide version: " << std::endl;
	io::console::reset_out_color(io::text_color::light_blue);
	wcout << wmessage << 1234567890ull << L'\n' << 123456.78e+09 << L'\n' << 12356.789e+10L << std::endl;
	io::console::reset_err_color(io::text_color::yellow);
	wcerr << L"No errors so far" << std::endl;

    return 0;
}
