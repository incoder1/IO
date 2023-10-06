/// This example shows IO channels to std lib input/output streams API compatibility layer

#include <iostream>
#include <ios>

#include <core/console.hpp>
#include <core/files.hpp>

#include <textapi/stream.hpp>

/// Opens a file channel with auto-reconverting content
/// to UTF-8 from current wchar_t UNICODE
/// Win32 wchar_t is UTF-16LE, Linux/Unix is UTF-32LE or UTF-32BE
/// depending on CPU architecture.
/// Intel and AMD  = LE
/// PowerPC, SPARC =  BE
/// ARM can be configured to use LE or BE, depending on operating system choose
/// for ARM 3+ and Android it is LE
io::s_write_channel prepare_file() {
	using namespace io;
	// Open a file to write results
	std::error_code ec;
	file f("result.txt");
	s_write_channel fch = f.open_for_write(ec, write_open_mode::overwrite);
	io::check_error_code(ec);
	// write byte order mark
	fch->write(ec, utf8_bom::data(), utf8_bom::len());
	io::check_error_code(ec);
	// open converting write channel, and stream on top of it
	s_code_cnvtr cvt = code_cnvtr::open(ec, code_pages::SYSTEM_WIDE , code_pages::UTF_8,
										cnvrt_control::failure_on_failing_chars);
	io::check_error_code(ec);
	s_write_channel ret= conv_write_channel::open(ec, fch, cvt);
	io::check_error_code(ec);
	return ret;
}

const char* umessage = "Hello!\nПривет!\nПривіт!\nΧαιρετίσματα!\nHelló!\nHallå!\n";
const wchar_t* wmessage = L"Hello!\nПривет!\nПривіт!\nΧαιρετίσματα!\nHelló!\nHallå!\n";

int main(int argc, const char** argv)
{

	// writing wchar_t chars into UTF-8 file
	io::cnl_wostream fout( prepare_file() );
	fout << wmessage
	<< 1234567890ull << L'\n'
	<< 123456.78e+09 << L'\n'
	<< 12356.789e+10L << L'\n'
	<< std::hex << 0xCAFEBABE
	<< std::endl;


	// Now we can use stream manipulators for colored output
	// this cout supports UF-8 UNICODE on Windows and colored output
	// make sure your console uses some FreeType font for better result
	io::console cons;
	io::console_output_stream cout(cons);
	io::console_error_stream cerr(cons);

	// stream manipulators used for colors
	cout << io::cclr::yellow << "This console streams supports UNICODE output, including Windows backend, and colored output" << std::endl;;
	cout << io::cclr::white << "You can found test output in result.txt file, it is converted from system wide wchar_t UNICODE representation to UTF-8" << std::endl;

	//
	cerr << io::cclr::brown << "Test output: "  << std::endl;

	cout <<  io::cclr::light_aqua << umessage;
	cout <<  io::cclr::gray << 1234567890ull << '\n';
	cout <<  io::cclr::light_green << 123456.78e+09 << '\n';
	cout <<  io::cclr::navy_green << 12356.789e+10L << '\n';
	cout << io::cclr::magenta << std::hex << 0xCAFEBABE << std::endl;

	cerr << io::cclr::light_red << "No errors so far"  << std::endl;

	// reset color to original
	cout << io::cclr::reset <<  "Thank you, Goodbye!" << std::endl;

	return 0;
}
