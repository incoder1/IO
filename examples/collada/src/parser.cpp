#include "parser.hpp"

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

parser::~parser() noexcept
{
}


} // namespace collada
