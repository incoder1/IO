#include "stdafx.hpp"
#include "statefull_test.hpp"

const char* stateful_test_fixture::TEST_DATA = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

const std::size_t stateful_test_fixture::BUFF_SIZE = 1024;

stateful_test_fixture::stateful_test_fixture():
	::testing::Test(),
	rch_(),
    wch_(),
    ec_()
{
}

void stateful_test_fixture::SetUp() {
	io::byte_buffer read_buff = io::byte_buffer::wrap(ec_, TEST_DATA, io_strlen(TEST_DATA));
	rch_ = io::memory_read_channel::open(ec_, std::move(read_buff) );
	EXPECT_FALSE(ec_);
	wch_ = io::memory_write_channel::open(ec_, BUFF_SIZE);
	EXPECT_FALSE(ec_);
}


TEST_F(stateful_test_fixture, should_synch_when_buffer_underflow) {
	auto tmp_buff = io::byte_buffer::allocate(ec_,16);
	EXPECT_FALSE(ec_);
    auto instance = io::buffered_channel_pump::create(ec_, rch_, std::move(tmp_buff) );
    EXPECT_FALSE(ec_);
    uint8_t data[33] = {0};
	std::size_t read = instance->pull(ec_, data, 32);
	ASSERT_STRCASEEQ("Lorem ipsum dolor sit amet, cons",reinterpret_cast<char*>(&data[0]));
	ASSERT_EQ(32, read);
    EXPECT_FALSE(ec_);
}

TEST_F(stateful_test_fixture, should_return_zerro_when_input_data_over) {
	auto tmp_buff = io::byte_buffer::allocate(ec_,64);
	EXPECT_FALSE(ec_);
    auto instance = io::buffered_channel_pump::create(ec_, rch_, std::move(tmp_buff) );
    EXPECT_FALSE(ec_);
    uint8_t data[124] = {0};
	std::size_t read = instance->pull(ec_, data, 123);
	ASSERT_STRCASEEQ(TEST_DATA,reinterpret_cast<char*>(&data[0]));
	ASSERT_EQ(123, read);
    EXPECT_FALSE(ec_);
    read = instance->pull(ec_, data, 124);
    ASSERT_EQ(0, read);
}

TEST_F(stateful_test_fixture, should_sync_buffered_channel_pump) {
    //auto test_pump = buffered_channel_pump::create(ec, read_channel, byte_buffer(20));
    //EXPECT_TRUE(test_pump->sync(ec_));
}

