#include <xml_lexcast.hpp>
#include <text.hpp>

#include <iostream>

int main()
{
	typedef io::xml::lexical_cast<char32_t> lcast;
	std::cout << lcast::str_to_float(U"0.33") << std::endl;
    return 0;
}
