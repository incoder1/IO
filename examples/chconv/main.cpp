#include <text.hpp>
#include <files.hpp>

#include <iostream>

static void check_system_error(std::error_code& ec) {
	static std::error_condition ok = std::system_category().default_error_condition(0);
	if(ok != ec) {
        std::cerr<< ec.message() << std::endl;
        std::exit( ec.value() );
	}
}

int main()
{
	using namespace io;
	std::error_code ec;

	std::wcout << transcode_to_ucs("Converting test-utf16le.txt to conv-res-utf8.txt") << std::endl;

	file sf = file::get(ec,"test-utf16le.txt");
	check_system_error(ec);
	file to = file::get(ec,"conv-res-utf8.txt");
	check_system_error(ec);

    s_read_channel src = conv_read_channel::open(ec,  sf.open_for_read(ec), code_pages::UTF_16LE, code_pages::UTF_8 );
	check_system_error(ec);

	s_write_channel dst = to.open_for_write(ec, write_open_mode::overwrite);
	check_system_error(ec);
	// write a UTF-8 BOM to destination file first
	dst->write(ec, io::utf8_bom::data(), io::utf8_bom::len() );
	check_system_error(ec);

	std::size_t transfered = transfer(ec, src, dst, 1024);
	check_system_error(ec);
	std::wcout<< transfered << L" bytes converted from "<< sf.name() << L" to " << to.name() << std::endl;

    return 0;
}
