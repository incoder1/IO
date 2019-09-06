#if _WIN32_WINNT < 0x0600
#	undef  _WIN32_WINNT
#	define WINVER 0x0600
#	define _WIN32_WINNT 0x0600
#endif // _WIN32_WINNT

#include <text.hpp>
#include <files.hpp>

#include <iostream>
#include <vector>

int main()
{
	using namespace io;
	std::error_code ec;

	file sf( "test-utf16le.txt" );
	if(!sf.exist()) {
		std::cerr << " test file " << sf.path()
		<< "is not exist or can not be found"
		<< std::endl;
		return -1;
	}
	file to("conv-res-utf8.txt");
	if( !to.exist() )
		to.create();

	std::cout << "Converting " << sf.path()
			  << " to "  << to.path()
			  << std::endl;

	s_code_cnvtr cvn = code_cnvtr::open(ec,
							code_pages::UTF_16LE,
							code_pages::for_name("UTF-8").second,
							cnvrt_control::failure_on_failing_chars
						);
	check_error_code(ec);
    s_read_channel src = conv_read_channel::open(ec,  sf.open_for_read(ec), cvn );
	check_error_code(ec);

	s_write_channel dst = to.open_for_write(ec, write_open_mode::overwrite);
	check_error_code(ec);
	// write a UTF-8 BOM to destination file first
	dst->write(ec, io::utf8_bom::data(), io::utf8_bom::len() );
	check_error_code(ec);

	std::size_t transcoded = transmit(ec, src, dst, 512);
	check_error_code(ec);

	std::cout<< "Transcoded " << transcoded <<
		" bytes from "<< sf.name()
		<< " to " << to.name()
		<< std::endl;

    return 0;
}
