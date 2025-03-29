#include "stdafx.hpp"
#include "memory_reead_channel_fixture.hpp"

const char* memory_read_channel_fixture::TEST_DATA = "Test message";

memory_read_channel_fixture::memory_read_channel_fixture():
	::testing::Test(),
	rch_(),
	ec_()
{}

void memory_read_channel_fixture::SetUp()
{
	io::byte_buffer buff = io::byte_buffer::wrap(ec_, TEST_DATA);
	EXPECT_FALSE(ec_);
    rch_ = io::memory_read_channel::open(ec_, std::move(buff));
}


// Test case for memory_read_channel::open
TEST_F(memory_read_channel_fixture, should_open_from_memory_block) {
    EXPECT_FALSE(ec_);
    EXPECT_TRUE(rch_);
}

// Test case for memory_read_channel::read
TEST_F(memory_read_channel_fixture,  should_read_from_memory_block) {
    uint8_t buffer[20];
    std::size_t bytes_read = rch_->read(ec_, buffer, sizeof(buffer));
    EXPECT_FALSE(ec_);
    EXPECT_GT(bytes_read, 0);
}

// memory_write_channel_fixture
memory_write_channel_fixture::memory_write_channel_fixture():
	::testing::Test(),
	wch_(),
	ec_()
{}

void memory_write_channel_fixture::SetUp()
{
	wch_ = io::memory_write_channel::open(ec_);
}

// Test case for memory_write_channel::open
TEST_F(memory_write_channel_fixture, should_open_to_memory_block) {
    EXPECT_TRUE(wch_);
    EXPECT_FALSE(ec_);
}

// Test case for memory_write_channel::should_wirite_into_memory
TEST_F(memory_write_channel_fixture, should_write_into_memory_block) {
    const char* message = "Test message";
    wch_->write(ec_, reinterpret_cast<const uint8_t*>(message), io_strlen(message));
    EXPECT_FALSE(ec_);

    io::byte_buffer written = wch_->data(ec_);
    EXPECT_FALSE(ec_);
    EXPECT_EQ(written.length(), io_strlen(message));
    EXPECT_EQ(io_memcmp(written.position().cdata(), message, io_strlen(message)), 0);
}
