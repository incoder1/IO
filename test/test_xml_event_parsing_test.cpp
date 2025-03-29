#include <gtest/gtest.h>
#include <io/core/files.hpp>
#include <io/core/char_cast.hpp>
#include <io/core/memory_channel.hpp>
#include <io/xml/event_stream_parser.hpp>

using namespace io;

// Helper function to read XML data from a string
static xml::s_event_stream_parser create_parser_from_string(const char* xml_data) {
    std::error_code ec;
    io::byte_buffer raw = byte_buffer::wrap(ec, xml_data);
    io::check_error_code(ec);
    auto src = memory_read_channel::open(ec, std::move(raw));
    io::check_error_code(ec);
    xml::s_event_stream_parser result = xml::event_stream_parser::open(ec, src);
    io::check_error_code(ec);
    return result;
}

// Test parsing start document event
TEST(xml_event_parsing_fixture, ParseStartDocument) {
    const char* xml_data = "<?xml version='1.0' encoding=\"UTF-8\" standalone='yes'?><test><msg attr0=\"key\" attr1=\"value\">Content</msg></test>";

    auto xs = create_parser_from_string(xml_data);

    xml::state_type state = xs->scan_next();
    EXPECT_EQ(xml::state_type::initial, state);
    state = xs->scan_next();
    EXPECT_EQ(xml::state_type::event, state);
    EXPECT_EQ(xml::event_type::start_document, xs->current_event());

    xml::document_event ev = xs->parse_start_doc();
    ASSERT_FALSE(xs->is_error());

    EXPECT_EQ(ev.version(), "1.0");
    EXPECT_EQ(ev.encoding(), "UTF-8");
    EXPECT_TRUE(ev.standalone());
}

// Test parsing processing instruction event
TEST(xml_event_parsing_fixture, ParseProcessingInstruction) {

    const char* xml_data = "<?xml version='1.0' encoding=\"UTF-8\" standalone='yes'?><?xml-stylesheet type=\"text/xsl\" href=\"style.xsl\" ?><test></test>";

    auto xs = create_parser_from_string(xml_data);
    xml::state_type state = xs->scan_next();
    EXPECT_EQ(xml::state_type::initial, state);
    state = xs->scan_next();
    EXPECT_EQ(xml::state_type::event, state);
    state = xs->scan_next();
    EXPECT_EQ(xml::state_type::event, state);
    EXPECT_EQ(xml::event_type::processing_instruction, xs->current_event());

    xml::instruction_event ev = xs->parse_processing_instruction();
    ASSERT_FALSE(xs->is_error());

    EXPECT_EQ(ev.target(), "xml-stylesheet");
    EXPECT_EQ(ev.data(), "type=\"text/xsl\" href=\"style.xsl\"");
}

// Test parsing start element event
TEST(xml_event_parsing_fixture, ParseStartElement) {
    const char* xml_data = "<?xml version='1.0' encoding=\"UTF-8\" standalone='yes'?><test xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"></test>";

    auto xs = create_parser_from_string(xml_data);
    xml::state_type state = xs->scan_next();
    EXPECT_EQ(xml::state_type::initial, state);
    state = xs->scan_next();
    EXPECT_EQ(xml::state_type::event, state);
    state = xs->scan_next();
    EXPECT_EQ(xml::state_type::event, state);
    EXPECT_EQ(xml::event_type::start_element, xs->current_event());

    xml::start_element_event e = xs->parse_start_element();
    ASSERT_FALSE(xs->is_error());
}

// Test parsing end element event
TEST(xml_event_parsing_fixture, ParseEndElement) {
    const char* xml_data = "<?xml version='1.0' encoding=\"UTF-8\" standalone='yes'?><test><msg attr0=\"key\" attr1=\"value\">Content</msg></test>";

    auto psr = create_parser_from_string(xml_data);
    psr->scan_next(); // Move to start document event
    psr->scan_next(); // Move to test element start
    psr->scan_next(); // Move to msg element start
    psr->scan_next(); // Move to characters event
    psr->scan_next(); // Move to msg element end

    xml::end_element_event e = psr->parse_end_element();
    ASSERT_FALSE(psr->is_error());

    EXPECT_EQ(e.name().local_name(), "msg");
}

// Test parsing characters event
TEST(xml_event_parsing_fixture, ParseCharacters) {
    const char* xml_data = "<?xml version='1.0' encoding=\"UTF-8\" standalone='yes'?><test><msg attr0=\"key\" attr1=\"value\">Content</msg></test>";

    auto psr = create_parser_from_string(xml_data);
    psr->scan_next(); // Move to start document event
    psr->scan_next(); // Move to test element start
    psr->scan_next(); // Move to msg element start
    psr->scan_next(); // Move to characters event

    const_string chars = psr->read_chars();
    ASSERT_FALSE(psr->is_error());
    EXPECT_EQ(chars, "Content");
}
