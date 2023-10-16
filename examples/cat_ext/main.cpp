/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

/*
 * This example demonstrate IO library text API possibilities
 * It works as POSIX utility cat https://pubs.opengroup.org/onlinepubs/9699919799.2018edition/utilities/cat.html
 * and in the same time auto-detect input file character set and convert it to system UNCODE before printing to console
 */

#include <vector>

#include <io/core/console.hpp>
#include <io/core/files.hpp>

#include <io/textapi/charset_detector.hpp>
#include <io/textapi/charset_converter.hpp>

void print_error(io::console_error_stream& err,const char* reason, const char* f, const char* what);
void file_to_console(std::error_code& ec,io::file&& f, io::console_out_writer& out);

int main(int argc, const char** argv)
{
	int ret = ERROR_SUCCESS;

	io::console cons;
	io::console_error_stream cerr(cons);

	io::console_out_writer out(cons);

	if(argc < 2) {
		cerr <<  io::cclr::light_red << "No files to display." << io::cclr::reset << std::endl << std::endl;
		cerr << "\tCommand usage : ";
		cerr <<  io::cclr::navy_green << "cat_ext " << io::cclr::reset;
		cerr << '[' << io::cclr::magenta << "file..." << io::cclr::reset << ']' << std::endl;
		ret = EINVAL;
	}

	std::error_code ec;
	for(std::size_t i =1; i < static_cast<std::size_t>(argc); i++) {
		io::file src_file(argv[i]);
		if( !src_file.exist() ) {
			ret = ENOENT;
			print_error(cerr,"Requested",argv[i],"not found");
			break;
		}
		file_to_console(ec,std::move(src_file), out);
		if(ec) {
			ret = ec.value();
			print_error(cerr,"Issue with" ,argv[i], ec.message().data() );
			break;
		}
	}

	return ret;
}


void print_error(io::console_error_stream& err,const char* reason, const char* f, const char* what)
{
	err <<  io::cclr::light_red << "Error" << io::cclr::reset << ": ";
	err << reason << " file ";
	err << io::cclr::yellow << f << io::cclr::reset;
	err << ' ' << what << io::cclr::reset << std::endl;
}

#ifdef __IO_WINDOWS_BACKEND__
static const char* WENDL = "\r\n";
#else
static const char* WENDL = "\n";
#endif // __IO_WINDOWS_BACKEND__

void file_to_console(std::error_code& ec,io::file&& f, io::console_out_writer& out)
{
	io::s_read_channel rch = f.open_for_read(ec);
	if(!ec) {
		io::s_charset_detector chdet = io::charset_detector::create(ec);
		if(!ec) {
			// detect character set
			uint8_t buff[1024] = {0};
			std::size_t read = rch->read(ec, buff, 1024);
			if ( !ec && read > 0 ) {
				auto detect_status = chdet->detect( ec, buff, read );
				if(!ec && !detect_status) {
					ec = std::make_error_code(std::errc::illegal_byte_sequence);
				}
				else {
					out.write("Detected : ");
					out.write(detect_status.character_set()->name());
					out.write(WENDL);
					if( io::code_pages::utf8()->code() != detect_status.character_set()->code() ) {

					}
					else {
						const char* it = reinterpret_cast<const char*>(buff);
						if( io::utf8_bom::is(it) ) {
							it += io::utf8_bom::len();
							read -= io::utf8_bom::len();
						}
						do {
							out.write(it, read);
							out.flush();
							read = rch->read(ec, buff, 1024 );
							it = reinterpret_cast<const char*>(buff);
						} while( read && !ec);
					}
				}
			}
		}
	}
}
