#include "stdafx.hpp"
#include "statefull_test.hpp"

const char* stateful_test_fixture::TEST_DATA = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

const std::size_t stateful_test_fixture::BUFF_SIZE = 1024;

stateful_test_fixture::stateful_test_fixture():
	::testing::Test(),
	rch_(),
	ec_()
{
}

void stateful_test_fixture::SetUp()
{
	io::byte_buffer read_buff = io::byte_buffer::wrap(ec_, TEST_DATA, io_strlen(TEST_DATA));
	rch_ = io::memory_read_channel::open(ec_, std::move(read_buff) );
	EXPECT_FALSE(ec_);
}


TEST_F(stateful_test_fixture, should_synch_when_buffer_underflow)
{
	// Given
	auto tmp_buff = io::byte_buffer::allocate(ec_,16);
	EXPECT_FALSE(ec_);
	auto instance = io::buffered_channel_pump::create(ec_, rch_, std::move(tmp_buff) );
	EXPECT_FALSE(ec_);
	// When
	uint8_t data[33] = {0};
	std::size_t read = instance->pull(ec_, data, 32);
	// Then
	ASSERT_STRCASEEQ("Lorem ipsum dolor sit amet, cons",reinterpret_cast<char*>(&data[0]));
	ASSERT_EQ(32, read);
	EXPECT_FALSE(ec_);
}

TEST_F(stateful_test_fixture, should_return_zerro_when_input_data_over)
{
	// Given
	auto tmp_buff = io::byte_buffer::allocate(ec_,64);
	EXPECT_FALSE(ec_);
	io::s_pump instance = io::buffered_channel_pump::create(ec_, rch_, std::move(tmp_buff) );
	EXPECT_FALSE(ec_);
	// When
	uint8_t data[124] = {0};
	std::size_t read = instance->pull(ec_, data, 123);
	ASSERT_STREQ(TEST_DATA,reinterpret_cast<char*>(&data[0]));
	ASSERT_EQ(123, read);
	EXPECT_FALSE(ec_);
	// Then
	read = instance->pull(ec_, data, 124);
	ASSERT_EQ(0, read);
}

TEST_F(stateful_test_fixture, should_funnel_buffer_overflow)
{
	// Given
//	io::s_memory_write_channel wch = io::memory_write_channel::open(ec_, BUFF_SIZE);
//	EXPECT_FALSE(ec_);
//	io::s_funnel intance = io::buffered_channel_funnel::create(ec_, wch, 32);
//	EXPECT_FALSE(ec_);
//	// When
//	std::size_t actual = intance->push(ec_, reinterpret_cast<const uint8_t*>(TEST_DATA), io_strlen(TEST_DATA));
//	EXPECT_FALSE(ec_);
	// TODO: Change API to begin/end instead
//	io::byte_buffer written = wch->data(ec_);
//	EXPECT_FALSE(ec_);
// const char* actual_str = written.position().cdata();
	// Then
//	EXPECT_FALSE(ec_);
//	ASSERT_EQ(io_strlen(TEST_DATA), actual);
//	ASSERT_STREQ(TEST_DATA,  actual_str);
}

