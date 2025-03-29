#ifndef __IO_STATEFULL_TEST_HPP_INCLUDED__
#define __IO_STATEFULL_TEST_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>

#include <system_error>

#include <io/core/stateful.hpp>
#include <io/core/memory_channel.hpp>

class stateful_test_fixture : public ::testing::Test {
public:
	static const char* TEST_DATA;
	static const std::size_t BUFF_SIZE;
public:
	stateful_test_fixture();
    void SetUp() override;
protected:
    io::s_read_channel rch_;
    io::s_memory_write_channel wch_;
    std::error_code ec_;
};

#endif // __IO_STATEFULL_TEST_HPP_INCLUDED__
