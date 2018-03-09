#include "stdafx.hpp"
#include "memory_channel.hpp"

namespace io {

s_memory_read_write_channel memory_read_write_channel::new_channel(std::error_code& ec, std::size_t buff_size) noexcept
{
   uint8_t* buff =  memory_traits::calloc_temporary<uint8_t>(buff_size);
   if(nullptr == buff) {
   		ec = std::make_error_code(std::errc::not_enough_memory);
   		return s_memory_read_write_channel();
   }
   memory_read_write_channel *ret = nobadalloc<memory_read_write_channel>::construct(ec, buff, buff_size);
   return nullptr != ret ? s_memory_read_write_channel(ret) : s_memory_read_write_channel() ;
}

memory_read_write_channel::memory_read_write_channel(uint8_t* const block,const std::size_t capacity) noexcept:
	read_write_channel(),
	block_( block ),
	end_ (block + capacity),
	read_pos_(block),
	write_pos_(block),
	rwb_()
{}

memory_read_write_channel::~memory_read_write_channel()
{
	memory_traits::free_temporary( block_ );
}

bool memory_read_write_channel::empty() const noexcept {
	read_lock lock(rwb_);
	return (block_ == read_pos_) && (block_ == write_pos_);
}

bool memory_read_write_channel::full() const noexcept {
	read_lock lock(rwb_);
	return end_ == write_pos_;
}

void memory_read_write_channel::clear() noexcept
{
	write_lock lock(rwb_);
	read_pos_ = block_;
	write_pos_ = block_;
}

std::size_t memory_read_write_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	read_lock lock(rwb_);
    std::size_t avail = memory_traits::distance(read_pos_, write_pos_);
    if(0 != avail) {
		std::size_t ret = (avail <= bytes) ? bytes : avail;
		io_memmove(buff, block_, bytes);
		read_pos_ += ret;
		return ret;
    }
    return 0;
}

std::size_t memory_read_write_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	write_lock lock(rwb_);
    std::size_t avail = memory_traits::distance(write_pos_, end_);
	if(0 != avail) {
 		std::size_t ret = (avail < size) ? size : avail;
 		io_memmove( write_pos_, buff, ret);
 		write_pos_ += ret;
	}
	return 0;
}

} // namespace io
