// This example demonstrate memory buffer read and write synchronous channels


#include <console.hpp>
#include <memory_channel.hpp>

// A data to be source of memory read channel
const char* umessage = "Hello!\nПривет!\nПривіт!\nΧαιρετίσματα!\nHelló!\nHallå!\n";

int main()
{
	std::error_code ec;

	// Open a memory buffer read channel
	io::s_read_channel rch = io::memory_read_channel::open(ec, io::byte_buffer::wrap( ec, umessage) );
	io::check_error_code(ec);

	// Open a memory buffer write channel
    io::s_memory_write_channel wch = io::memory_write_channel::open(ec);
	io::check_error_code(ec);

	// transmit data from memory read channel to memory write channel
	io::transmit(ec, rch, wch, 24);
    io::check_error_code(ec);

	// Obtain memory buffer from write channel
    io::byte_buffer written = wch->data(ec);
    io::check_error_code(ec);

	// Display data in console
    std::ostream& cout = io::console::out_stream();
    cout <<  written.position().cdata() << std::endl;

    return 0;
}
