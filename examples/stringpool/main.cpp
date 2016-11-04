#include <iostream>
#include <thread>

#include <cstring>
#include <stringpool.hpp>

struct strings {
	io::cached_string s1;
	io::cached_string s2;
	io::cached_string s3;
	io::cached_string s4;
};

strings init() {
	std::error_code ec;
	io::s_string_pool pool = io::string_pool::create(ec);
	if(!pool) {
		std::cerr<< ec.message() << std::endl;
		std::exit(ec.value());
	}
	strings res;
	const char M1[12] = {'H','e','l','l','o',32,'w','o','r','l','d','!'};
	res.s1 = pool->get(M1, 12);
	res.s2 = pool->get("Hi programmer!");
	res.s3 = pool->get(M1, 12);
	res.s4 = pool->get("Hi programmer!");
	std::cout<<"Pool size: " << pool->size() << std::endl;
	return res;
}

int main()
{
	try {
	io::cached_string s1,s2,s3,s4;
	{
		strings s = init();
		s1 = std::move(s.s1);
		s2 = std::move(s.s2);
		s3 = std::move(s.s3);
		s4 = std::move(s.s4);
	}

	std::cout<<"Equal: " << (s1 == s3 && s2 == s4 ? "yes" : "no") << std::endl << std::endl;
	std::cout<<"S1 hash: "  << s1.hash();
	std::cout<<" S3 hash: " << s3.hash() << std::endl;
	std::cout<<"S2 hash: "  << s2.hash();
	std::cout<<" S4 hash: " << s4.hash() << std::endl << std::endl;

	std::cout<<"S1: " << s1 << std::endl;
	std::cout<<"S2: " << s2 << std::endl;
	std::cout<<"S3: " << s3 << std::endl;
	std::cout<<"S4: " << s4 << std::endl;
	} catch(std::exception& exc) {
		std::cerr<< exc.what() << std::endl;
	}
    return 0;
}
