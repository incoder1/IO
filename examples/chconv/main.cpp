// FIXME: this API should be changed since converting channel abstraction is wrong, reader/writer should be used instead
// If you see this don't use this API, it is internal workaround for give XML parser ability to read any source
/*
 * This example shows how to use IO character set conversion API
 */

 #include <iostream>

#include <io/core/files.hpp>
#include <io/textapi/charset_converter.hpp>

static const std::size_t BUFFER_SIZE = 512;

int main(int argc, const char** argv)
{
	using namespace io;
	std::error_code ec;

	// Open IO file channel with UTF-16LE encoded text
	file sf( "test-utf16le.txt" );
	if(!sf.exist()) {
		std::cerr << "Test file "<< sf.path() << "is not exist"<< std::endl;
		return -1;
	}
	// Create new destination file with UTF-8 encoded text result
	file to("conv-res-utf8.txt");
	if( !to.exist() )
		to.create();

	std::cout << "Converting "<< sf.path() <<" to "<< to.path() << std::endl;

	// Open a character set converter between UTF-16LE and UTF-8 code pages
	auto cvn = charset_converter::open(ec,
							code_pages::utf16le(),
							code_pages::utf8(),
							cnvrt_control::failure_on_failing_chars
						);

	// Open a converting channel, which will recode all source characters to destination
	check_error_code(ec);
    s_read_channel src = conv_read_channel::open(ec,  sf.open_for_read(ec), cvn );
	check_error_code(ec);

	s_write_channel dst = to.open_for_write(ec, write_open_mode::overwrite);
	check_error_code(ec);

	// write a UTF-8 BOM to destination file first
	dst->write(ec, io::utf8_bom::data(), io::utf8_bom::len() );
	check_error_code(ec);

	std::size_t transcoded = transmit(ec, src, dst, BUFFER_SIZE);
	check_error_code(ec);

	std::cout << "Transcoded "<< transcoded << " bytes from "<< sf.name() << " to " << to.name() << std::endl;

    return 0;
}
