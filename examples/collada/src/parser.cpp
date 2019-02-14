#include "parser.hpp"
#include <cmath>


namespace collada {


static io::xml::reader open_reader(io::s_read_channel&& src)
{
	std::error_code ec;
	io::xml::s_event_stream_parser parser = io::xml::event_stream_parser::open(ec, std::move(src) );
	io::check_error_code( ec );
	return io::xml::reader( std::move(parser) );
}

parser::parser(io::s_read_channel&& src) noexcept:
	io::object(),
	rd_( open_reader( std::forward<io::s_read_channel>(src) ) )
{
}


typedef io::xml::lexical_cast_traits<float> float_cast;

static float next_float(const char* str,char** endp)
{
	static constexpr const char* SPACES = "\t\n\v\f\r ";
	static constexpr const char* DGTS = "0123456789";
	str = std::strpbrk(str, DGTS);
	float ret = NAN;
	if(nullptr != str) {
		*endp = const_cast<char*>(str) + std::strspn(str, SPACES);
		if('\0' != *endp[0])
			ret = float_cast::from_string( str );
	}
	return ret;
}


static io::scoped_arr<float> parse_string_list(const io::const_string& val, std::size_t size)
{
	if( ! val.blank() ) {
		io::scoped_arr<float> ret( size );
		char *s = const_cast<char*>( val.data() );
		std::size_t i = 0;
		while( nullptr !=  s) {
			ret[i++] = next_float(s, &s);
		}
		return ret;
	}
	return io::scoped_arr<float>();
}

std::vector<material> parser::read_materials()
{

	// skip to <library_effects>

	// read <effect id="Material-effect">

	// skip <profile_COMMON>

	// skip <technique sid="common">

	// read color from <phong>

	// skip index_of_refraction
}

model parser::load()
{
	return model();
}

parser::~parser() noexcept
{
}


} // namespace collada
