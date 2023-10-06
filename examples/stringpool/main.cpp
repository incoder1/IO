#include <iostream>
#include <core/stringpool.hpp>

struct strings {
	io::const_string s1;
	io::const_string s2;
	io::const_string s3;
	io::const_string s4;
	io::const_string s5;
};

strings init()
{
	std::error_code ec;
	io::s_string_pool pool = io::string_pool::create(ec);
	if(!pool) {
		std::cerr<< ec.message() << std::endl;
		std::exit(ec.value());
	}
	strings res;
	const char M1[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	res.s1 = pool->get(M1, 26);
	res.s2 = pool->get("abcdefghijklmnopqrstuvwxyz");
	res.s3 = pool->get(M1, 26);
	res.s4 = pool->get("abcdefghijklmnopqrstuvwxyz");
	res.s5 = pool->get("0123");
	std::cout<<"Pool size: " << pool->size() << std::endl;
	return res;
}

int main()
{
	io::const_string s1,s2,s3,s4,s5;
	{
		strings s = init();
		s1 = s.s1;
		s2 = s.s2;
		s3 = s.s3;
		s4 = s.s4;
		s5 = s.s5;
	}

//	std::pmr::polymorphic_allocator

	std::cout<<"Equal: " << ( (s1 == s3 && s2 == s4) ? "yes" : "no" );
	std::cout<< std::endl << std::endl;

	std::cout<<"S1 hash: "  << s1.hash();
	std::cout<<" S3 hash: " << s3.hash() << std::endl;
	std::cout<<"S2 hash: "  << s2.hash();
	std::cout<<" S4 hash: " << s4.hash() << std::endl;
	std::cout<<"S5 hash: " << s5.hash();
	std::cout<< std::endl << std::endl;

	std::cout<<"S1: " << s1 << std::endl;
	std::cout<<"S2: " << s2 << std::endl;
	std::cout<<"S3: " << s3 << std::endl;
	std::cout<<"S4: " << s4 << std::endl;
	std::cout<<"S5: " << s5 << std::endl;

	return 0;
}
