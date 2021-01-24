/*
 * This example demonstrate IO library character set detector functionality
 */
#include <errorcheck.hpp>
#include <charsetdetector.hpp>
#include <iostream>


// A latin1 character, code page name is platform specific, for example for Windows it is CP1252
const char* latin1str = "0Aa\n\t1Bb\n\t2Cc\n\t3Dd\n\t4Ee\n\t5Ff\n\t6Gg\n\t7Hh\n\t8Ii\n\t9Jj";
// A UNICODE character set with UTF-8 code page
const char* u8str = "Hello!\nПривет!\nПривіт!\nΧαιρετίσματα!\nHelló!\nHallå!\nこんにちは!\n你好!\n";


int main(int atgc, const char** argv)
{
	// construct a new character set detector
	std::error_code ec;
	io::s_charset_detector chdet = io::charset_detector::create(ec);
	io::check_error_code(ec);

	// now detect lantin 1 single byte code page
	std::cout<< "Detecting latin1 character set\n";
	io::charset_detect_status status = chdet->detect(ec,
									   reinterpret_cast<const uint8_t*>(latin1str),
									   io_strlen(latin1str)
													);

	io::check_error_code(ec);
	std::cout << "Detected character set: " << status.character_set().name() << " \n";
	std::cout << "Type: " << ( status ? "for sure\n" : "guested\n");
	if(!status)
		std::cout << "Confidence : " << ( status.confidence() * 100.0F ) << '%' << std::endl;

	// now detect UTF-8 UNICODE
	std::cout<< "\nDetecting UTF-8 character set\n";
	status = chdet->detect(ec,
						   reinterpret_cast<const uint8_t*>(u8str),
						   io_strlen(u8str) );
	io::check_error_code(ec);
	std::cout << "Detected character set: " << status.character_set().name() << " \n";
	std::cout << "Type: " << ( status ? "for sure\n" : "guested\n");
	if(!status)
		std::cout << "Confidence : " << ( status.confidence() * 100.0F ) << '%' << std::endl;

	return 0;
}

