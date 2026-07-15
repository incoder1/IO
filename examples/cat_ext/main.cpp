/*
 * Copyright (c) 2016-2026
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <vector>
#include <io/core/files.hpp>
#include <io/console/console.hpp>
#include <io/textapi/charset_detector.hpp>
#include <io/textapi/charset_converter.hpp>

/// Prints an error into console if any
void print_error(io::console_error_stream& err, const char* reason, const char* f, const char* what);
/// Prints a text file into console, detect character set automatically and reconvert into system UNICODE
void file_to_console(std::error_code& ec, io::file&& f, io::console_out_writer& out);

int main(int argc, const char** argv)
{
    int ret = ERROR_SUCCESS;

    io::console cons;
    io::console_error_stream cerr(cons);
    io::console_out_writer out(cons);

    if (argc < 2) {
        cerr << io::cclr::light_red << "No files to display." << io::cclr::reset << std::endl << std::endl;
        cerr << "\tCommand usage : ";
        cerr << io::cclr::navy_green << "cat_ext " << io::cclr::reset;
        cerr << '[' << io::cclr::magenta << "file..." << io::cclr::reset << ']' << std::endl;
        return EINVAL;
    }

    std::error_code ec;
    for (std::size_t i = 1; i < static_cast<std::size_t>(argc); i++) {
        io::file src_file(argv[i]);
        if (!src_file.exist()) {
            ret = ENOENT;
            print_error(cerr, "Requested", argv[i], "not found");
            break;
        }

        file_to_console(ec, std::move(src_file), out);
        if (ec) {
            ret = ec.value();
            print_error(cerr, "Issue with", argv[i], ec.message().data());
            break;
        }
    }

    return ret;
}

void print_error(io::console_error_stream& err, const char* reason, const char* f, const char* what)
{
    err << io::cclr::light_red << "Error" << io::cclr::reset << ": ";
    err << reason << " file ";
    err << io::cclr::yellow << f << io::cclr::reset;
    err << ' ' << what << io::cclr::reset << std::endl;
}

io::s_pump data_pump_for_charset(std::error_code& ec, const io::s_read_channel& rch, const io::charset* chst, io::byte_buffer&& buff)
{
    if (io::code_pages::utf8() == chst) {
        return io::buffered_channel_pump::create(ec, rch, std::move(buff));
    }
    return io::charset_converting_channel_pump::create(ec, rch, chst, io::code_pages::utf8(), std::move(buff));
}

void file_to_console(std::error_code& ec, io::file&& f, io::console_out_writer& out)
{
    io::s_read_channel rch = f.open_for_read(ec);
    if (ec)
        return;

    io::byte_buffer buff = io::byte_buffer::allocate(ec, 1024);
    if (ec)
        return;

    // Read initial block to detect character set
    std::size_t bytes_read = rch->read(ec, const_cast<uint8_t*>(buff.position().get()), buff.capacity());
    if (ec || bytes_read == 0)
        return;

    io::s_charset_detector chdet = io::charset_detector::create(ec);
    if (ec)
        return;

    buff.move(bytes_read);
    buff.flip();

    auto detect_status = chdet->detect(ec, buff.position().get(), bytes_read);
    if (ec)
        return;

    if (!detect_status) {
        ec = std::make_error_code(std::errc::illegal_byte_sequence);
        return;
    }

    // Print detected charset
    out.write("Detected : ");
    out.writeln(detect_status.character_set()->name());

    io::s_pump pmp = data_pump_for_charset(ec, rch, detect_status.character_set(), std::move(buff));
    if (ec)
        return;

    // Stream the converted content directly to the console
    io::reader in(std::move(pmp));
    char tmp[1024];
    while ((bytes_read = in.read(ec, tmp, sizeof(tmp))) > 0 && !ec) {
        out.write(tmp, bytes_read);
    }
}
