#ifndef __MEMORY_REEAD_CHANNEL_FIXTURE_HPP_INCLUDED__
#define __MEMORY_REEAD_CHANNEL_FIXTURE_HPP_INCLUDED__


#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>

#include <io/core/memory_channel.hpp>
#include <io/core/buffer.hpp>

// Test fixture for memory_read_channel
class memory_read_channel_fixture: public ::testing::Test {
private:
	static const char* TEST_DATA;
public:
	memory_read_channel_fixture();
    void SetUp() override;
protected:
    io::s_read_channel rch_;
    std::error_code ec_;
};

// Test fixture for memory_write_channel
class memory_write_channel_fixture: public ::testing::Test {
public:
 	memory_write_channel_fixture();
    void SetUp() override;
protected:
    io::s_memory_write_channel wch_;
    std::error_code ec_;
};


#endif // __MEMORY_REEAD_CHANNEL_FIXTURE_HPP_INCLUDED__
