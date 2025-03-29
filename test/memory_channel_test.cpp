#include "stdafx.hpp"
#include "memory_channel_test.hpp"

const char* memory_read_channel_fixture::TEST_DATA = "0123456789ABCDEF";

// memory_read_channel_fixture
memory_read_channel_fixture::memory_read_channel_fixture():
	::testing::Test(),
	rch_(),
	ec_()
{}

void memory_read_channel_fixture::SetUp()
{
	io::byte_buffer buff = io::byte_buffer::wrap(ec_, TEST_DATA, 16);
	EXPECT_FALSE(ec_);
    rch_ = io::memory_read_channel::open(ec_, std::move(buff));
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

// Tests

TEST_F(memory_read_channel_fixture, should_open_from_memory_block)
{
    EXPECT_FALSE(ec_);
    EXPECT_TRUE(rch_);
}

TEST_F(memory_read_channel_fixture,  should_read_from_memory_block)
{
    uint8_t buffer[9] = {0};
    std::size_t actual = rch_->read(ec_, buffer, 8);
    EXPECT_FALSE(ec_);
   	ASSERT_EQ(actual,8);
   	ASSERT_STREQ("01234567",reinterpret_cast<char*>(&buffer[0]));
}

TEST_F(memory_read_channel_fixture,  should_return_0_when_eof)
{
    uint8_t buffer[9] = {0};
    std::size_t actual = rch_->read(ec_, buffer, 8);
    EXPECT_FALSE(ec_);
   	ASSERT_EQ(actual,8);
   	ASSERT_STREQ("01234567",reinterpret_cast<char*>(&buffer[0]));
   	actual = rch_->read(ec_, buffer, 8);
	ASSERT_EQ(actual,8);
   	ASSERT_STREQ("89ABCDEF",reinterpret_cast<char*>(&buffer[0]));
	actual = rch_->read(ec_, buffer, 8);
	ASSERT_EQ(actual,0);
}

TEST_F(memory_write_channel_fixture, should_open_to_memory_block)
{
    EXPECT_TRUE(wch_);
    EXPECT_FALSE(ec_);
}

TEST_F(memory_write_channel_fixture, should_write_into_memory_block)
{
    const char* message = "Test message";
    wch_->write(ec_, reinterpret_cast<const uint8_t*>(message), io_strlen(message));
    EXPECT_FALSE(ec_);
    io::byte_buffer written = wch_->data(ec_);
    EXPECT_FALSE(ec_);
    EXPECT_EQ(written.length(), io_strlen(message));
	ASSERT_STREQ(message, written.position().cdata());
}
