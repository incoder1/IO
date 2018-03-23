/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_BUFFER_HPP_INCLUDED__
#define __IO_BUFFER_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <cstring>
#include <iterator>
#include <limits>
#include <type_traits>

namespace io {

class byte_buffer;

namespace detail {

class mem_block {
    mem_block(const mem_block&) = delete;
    mem_block& operator=(const mem_block&) = delete;
public:

    constexpr explicit mem_block(uint8_t* const px) noexcept:
        px_(px)
    {}

    constexpr mem_block() noexcept:
        mem_block(nullptr)
    {}

    ~mem_block() noexcept {
        if(nullptr != px_)
            memory_traits::free( px_ );
    }

    mem_block(mem_block&& other) noexcept:
        px_( other.px_ )
	{
        other.px_ = nullptr;
    }

    mem_block& operator=(mem_block&& rhs) noexcept
    {
        px_ = rhs.px_;
        rhs.px_ = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept
    {
        return nullptr != px_;
    }

    inline uint8_t* get() const
    {
        return px_;
    }

    static inline mem_block allocate(const std::size_t size) noexcept
    {
        uint8_t *ptr = memory_traits::malloc_array<uint8_t>(size);
        if(nullptr == ptr)
            return mem_block();
        return mem_block( ptr );
    }

    static inline mem_block wrap(const uint8_t* arr,const std::size_t size) noexcept {
        mem_block ret = allocate(size);
        if(ret)
            std::memcpy( ret.get(), arr, size);
        return std::move(ret);
    }

    inline void swap(mem_block& with) noexcept {
        std::swap( px_, with.px_);
    }

private:
    uint8_t *px_;
};


} // namespace detail


/// \brief The buffer iterator
/// \details Bidirectional dynamic array iterator, STL compatible
class byte_buffer_iterator: public std::iterator<
    std::bidirectional_iterator_tag,
    uint8_t,
    std::ptrdiff_t,
    const uint8_t*,
    uint8_t&> {
public:

    typedef std::iterator<
    std::bidirectional_iterator_tag,
        uint8_t,
        std::ptrdiff_t,
        const uint8_t*,
        uint8_t&> base_type;

    typedef base_type::iterator_category iterator_category;

    typedef base_type::value_type value_type;
    typedef base_type::reference  reference;
    typedef base_type::pointer  pointer;
    typedef base_type::difference_type difference_type;

    constexpr byte_buffer_iterator() noexcept:
        base_type(),
        position_(nullptr)
	{}

    constexpr byte_buffer_iterator(uint8_t* const position) noexcept:
        base_type(),
        position_(position)
	{}

    inline uint8_t operator*() const noexcept {
        return *position_;
    }

    inline const uint8_t* get() const noexcept {
        return position_;
    }

    inline const char* cdata() const noexcept {
        return reinterpret_cast<char*>(position_);
    }

    inline const wchar_t* wdata() const noexcept {
        return reinterpret_cast<wchar_t*>(position_);
    }

    inline const char16_t* u16data() const noexcept {
        return reinterpret_cast<char16_t*>(position_);
    }

    inline const char32_t* u32data() const noexcept {
        return reinterpret_cast<char32_t*>(position_);
    }

    inline byte_buffer_iterator& operator++() noexcept {
        ++position_;
        return *this;
    }

    inline byte_buffer_iterator operator++(int) noexcept {
        byte_buffer_iterator tmp( *this );
        ++position_;
        return tmp;
    }

    inline byte_buffer_iterator& operator--() noexcept {
        --position_;
        return *this;
    }

    inline byte_buffer_iterator operator--(int) noexcept {
        byte_buffer_iterator tmp( *this );
        --position_;
        return tmp;
    }

    inline byte_buffer_iterator& operator+=(const std::size_t rhs) noexcept {
        position_ += rhs;
        return *this;
    }

    inline byte_buffer_iterator& operator-=(const std::size_t rhs) noexcept {
        position_ -= rhs;
        return *this;
    }

    inline bool operator==(const byte_buffer_iterator& rhs) const noexcept {
        return position_ == rhs.position_;
    }

    inline bool operator!=(const byte_buffer_iterator& rhs) const noexcept {
        return position_ != rhs.position_;
    }

    inline bool operator==(byte_buffer_iterator&& rhs) const noexcept {
        return position_ == rhs.position_;
    }

    inline bool operator!=(byte_buffer_iterator&& rhs) const noexcept {
        return position_ != rhs.position_;
    }

    inline difference_type operator+(const byte_buffer_iterator& lhs) const noexcept {
        return  reinterpret_cast<difference_type>(position_) +
                reinterpret_cast<difference_type>(lhs.position_);
    }

    inline difference_type operator-(const byte_buffer_iterator& lhs) const noexcept {
        return reinterpret_cast<difference_type>(position_)
               - reinterpret_cast<difference_type>(lhs.position_);
    }


    inline difference_type operator+(byte_buffer_iterator&& lhs) const noexcept {
        return  reinterpret_cast<difference_type>(position_) +
                reinterpret_cast<difference_type>(lhs.position_);
    }

    inline difference_type operator-(byte_buffer_iterator&& lhs) const noexcept {
        return reinterpret_cast<difference_type>(position_)
               - reinterpret_cast<difference_type>(lhs.position_);
    }

    inline bool operator<(const byte_buffer_iterator& rhs) const noexcept {
        return  position_ < rhs.position_;
    }

    inline bool operator>(const byte_buffer_iterator& rhs) const noexcept {
        return  position_ > rhs.position_;
    }
private:
    uint8_t* position_;
};

/// \brief Movable only dynamic array container, with uint8_t* underlying memory array
class IO_PUBLIC_SYMBOL byte_buffer {
    byte_buffer(const byte_buffer&) = delete;
    byte_buffer& operator=(byte_buffer&) = delete;
public:
    typedef byte_buffer_iterator iterator;

    /// Constructs an empty byte buffer without allocating any memory
    /// \see #allocate
    /// \see #extend
    constexpr byte_buffer() noexcept:
        arr_(),
        capacity_(0),
        position_(nullptr),
        last_(nullptr)
    {}

    /// Movement constructor (shallow copy)
    /// \param other buffer to move
    byte_buffer(byte_buffer&& other) noexcept;

    /// Movement assignment operator (shallow copy)
    /// \param rhs buffer to move
    byte_buffer& operator=(byte_buffer&& rhs) noexcept {
        byte_buffer( std::forward<byte_buffer>(rhs) ).swap( *this );
        return *this;
    }

    /// Destroys this buffer and releases memory allocated by this buffer
    /// If this buffer was moved to another object rval, do nothing
    /// Otherwise apply std::free to the underlying memory buffer
    ~byte_buffer() noexcept
    {}

    /// Returns iterator on an byte address of current buffer insert position
    /// \return position buffer iterator
    inline iterator position() const noexcept {
        return iterator(position_);
    }

    /// Returns iterator on an byte address after last byte put in this buffer
    /// \return last buffer iterator
    inline iterator last() const noexcept {
        return iterator(last_);
    }

    /// Returns whether this buffer position and last shows on buffer first byte
    /// \return whether this buffer empty
    inline bool empty() const noexcept {
        return (position_ == nullptr) || ( position_==arr_.get() && last_ == (position_ + 1) );
    }

    /// Returns count of bytes this buffer can store
    /// \return this buffer capacity
    inline std::size_t capacity() const noexcept {
        return capacity_;
    }

    /// Moves this buffer poistion to the buffer first byte and stays last on it's current address
    inline void flip() noexcept {
        position_ = arr_.get();
    }

    /// Returns count of bytes between buffer first byte and last iterator
    /// \return count of bytes between buffer first byte and last iterator
    inline std::size_t size() const noexcept {
        return empty() ? 0 : memory_traits::distance(arr_.get(), (last_-1) );
    }

    /// Returns bytes count between position and last iterators
    /// \return bytes count between position and last iterators
    inline std::size_t length() const noexcept {
        return memory_traits::distance(position_, (last_-1) );
    }

    /// Returns bytes count between buffer position and buffer capacity
    /// \return count available to put
    inline std::size_t available() const noexcept {
        return capacity_ - memory_traits::distance(arr_.get(), position_);
    }

    /// Checks whether buffer length equals to buffer capacity
    /// \return whether buffer is full
    inline bool full() const noexcept {
        return last_ == (arr_.get() + capacity_);
    }

    /// Puts single byte into this buffer current position, and increses buffer position and last
    /// \param byte a byte to put
    /// \return true whether byte was put and false if buffer was full before put attempt
    bool put(uint8_t byte) noexcept {
        if( !full() ) {
			*position_ = byte;
			last_ = (++position_) + 1;
			return true;
        }
        return false;
    }

    /// Puts single character into this buffer current position, and increses buffer position and last
    /// \param ch a byte to put
    /// \return true whether byte was put and false if buffer was full before put attempt
    inline bool put(char ch) noexcept {
        return put( static_cast<uint8_t>(ch) );
    }

    /// Moves buffer current position on offset bytes and set last on position+1
    /// If offset is larger then available bytes, sets position to the buffer end
    /// and sets last to the iterator after buffer end
    /// \param offset moving offset in bytes
    void move(std::size_t offset) noexcept;

    /// Moves current buffer position on offset bytes i.e. shift position right
    /// Unlike \see #move last iterator will stay on it's current position.
    /// This method is useful for scanning buffer bytes after you've applied #flip operation
    /// \param offset count of bytes to shift, if position+offset is larger then buffer capacity, buffer will be cleared
    /// \see #flip
    /// \see #clear
    inline void shift(std::size_t offset) noexcept {
        position_ += offset;
        if( position_ >= last_ || (position_+1) == last_ )
            clear();
    }

    /// Puts continues memory block (an array) into this buffer.
    /// If memory block size larger then available bytes returns 0 and don't puts anything into this buffer
    /// \param begin of memory block first byte
    /// \param end address of memory block last byte, must be larger then begin
    /// \return count of bytes put in this buffer, or 0 if memory block is to large end <= begin
    std::size_t put(const uint8_t* begin,const uint8_t* const end) noexcept;


    /// Puts continues memory block (an array) into this buffer.
    /// If memory block size larger then available bytes returns 0 and not puts anything into this buffer
    /// \param arr address of memory block first byte
    /// \param count count of bytes to copy from array
    /// \return count of bytes put in this buffer, or 0 if memory block is to large
    inline std::size_t put(const uint8_t* arr, std::size_t count) noexcept {
        return put(arr, (arr + count) );
    }

    /// Puts continues memory block (an array) into this buffer.
    /// If memory block size larger then available bytes returns 0 and not puts anything into this buffer
    /// \param arr address of memory block first element
    /// \param count count of element to copy from array
    /// \return count of elements put in this buffer, or 0 if memory block is to large
    template<typename T>
    inline std::size_t put(const T* arr, std::size_t count) noexcept {
        static_assert( std::is_fundamental<T>::value || std::is_trivial<T>::value, "Must be an array of trivial or fundamental type" );
        if(nullptr == arr || count == 0 )
            return 0;
        const uint8_t* b = reinterpret_cast<const uint8_t*>(arr);
        const uint8_t* e = reinterpret_cast<const uint8_t*>(arr + count);
        return put( b, e) / sizeof(T);
    }

    template<typename __char_t>
    std::size_t put(const __char_t* cstr) noexcept {
        static_assert( std::is_integral<__char_t>::value, " Must be integral type");
        return put( cstr,std::char_traits<__char_t>::length(cstr) );
    }

    /// Puts content between position and last bytes from another buffer
    /// \return count of bytes put from another buffer, or 0 if not enough available space in this buffer
    inline std::size_t put(byte_buffer& other) noexcept {
        if( available() < other.length() )
            return 0;
        return put( other.position_, other.last_ );
    }

    /// Puts STL string ( std::basic_sting<?,?>) content between begin() and end() iterators
    /// to this buffer
    /// \return count of bytes put from std::basic_string, or 0 if not enough available space in this buffer
    template<class _char_t>
    inline std::size_t put(const std::basic_string<_char_t>& s) noexcept {
        if(s.empty())
            return 0;
        return put( s.data(), s.length() * sizeof(_char_t) );
    }

    // binary primitives functions

// MS VC++ defining int8_t as typedef char int8_t
// and not strong type definition according to the C++ standard
#ifndef _MSC_VER
    /// Puts a small value in its binary representation into current buffer
    /// \param small a small value
    inline bool put(int8_t small) {
        return put( static_cast<uint8_t>(small) );
    }
#endif

    inline int8_t get_int8() {
        return binary_get<int8_t>();
    }

    /// Puts an unsigned short value in its binary representation into current buffer
    /// \param us an unsigned short value
    inline bool put(uint16_t us) {
        return binary_put(us);
    }

    inline uint16_t get_uint16() {
        return binary_get<uint16_t>();
    }

    /// Puts a signed short value in its binary representation into current buffer
    /// \param ss a short value
    inline bool put(int16_t ss) {
        return binary_put(ss);
    }

    inline int16_t get_int16() {
        return binary_get<int16_t>();
    }

    /// Puts an unsigned integer value in its binary representation into current buffer
    /// \param ui an unsigned integer value
    inline bool put(uint32_t ui) {
        return binary_put(ui);
    }

    inline uint32_t get_uint32() {
        return binary_get<uint32_t>();
    }

    /// Puts a signed integer value in its binary representation into current buffer
    /// \param si a signed integer value
    inline bool put(int32_t si) {
        return binary_put(si);
    }

    inline int32_t get_int32() {
        return binary_get<int32_t>();
    }

    /// Puts an unsigned long integer value in its binary representation into current buffer
    /// \param ull an unsigned long integer value
    inline bool put(uint64_t ull) {
        return binary_put(ull);
    }

    inline uint64_t get_uint64() {
        return binary_get<int64_t>();
    }

    /// Puts a signed long integer value in its binary representation into current buffer
    /// \param sll a signed long integer value
    inline bool put(int64_t sll) {
        return binary_put(sll);
    }

    inline int64_t get_int64() {
        return binary_get<int64_t>();
    }

    /// Puts a float value in its binary representation into current buffer
    /// \param f a float value
    inline bool put(float f) {
        return binary_put(f);
    }

    inline float get_float() {
        return binary_get<float>();
    }

    /// Puts a double value in its binary representation into current buffer
    /// \param d a double value
    inline bool put(double d) {
        return binary_put(d);
    }

    inline double get_double() {
        return binary_get<long double>();
    }

    /// Puts a long double value in its binary representation into current buffer
    /// \param ld a long double value
    inline bool put(long double ld) {
        return binary_put(ld);
    }

    inline long double get_long_double() {
        return binary_get<long double>();
    }

    /// Sets position and last iterator to the buffer's first byte
    inline void clear() noexcept {
        position_ = arr_.get();
        last_ = position_ + 1;
    }

    /// Swaps this buffer with another buffer
    /// \param other another buffer reference to swap with
    void swap(byte_buffer& other) noexcept {
        arr_.swap(other.arr_);
        std::swap(capacity_, other.capacity_);
        std::swap(position_, other.position_);
        std::swap(last_, other.last_);
    }

    /// Extends (i.e. realloc) this buffer by increasing this buffer capacity with extend_size (capacity+extend_size)
    /// If not enough memory, buffer will be kept in memory as is
    /// \param extend_size count of bytes to extend this buffer capacity
    /// \return true buffer was extended, false if not enough available memory
    /// \throw never throws
    bool extend(std::size_t extend_size) noexcept;

    /// Exponentially grow this buffer by sq this buffer capacity
    /// If not enough memory, buffer will be kept in memory as is
    /// \return true buffer was extended, false if not enough available memory
    /// \throw never throws
    bool exp_grow() noexcept;

    /// Natural logarithm grow this buffer by ln this buffer capacity
	/// If not enough memory, buffer will be kept in memory as is
    /// \return true buffer was extended, false if not enough available memory
    /// \throw never throws
    bool ln_grow() noexcept;

private:

    byte_buffer(detail::mem_block&& arr, std::size_t capacity) noexcept;

    template < typename T >
    bool binary_put(const T& v) {
        return 0 != put( reinterpret_cast<const uint8_t*> ( std::addressof( v ) ), sizeof(T) );
    }

    template< typename T >
    inline T binary_get() {
        static_assert( std::is_arithmetic<T>::value, "Must be an arithmetic type" );
        if( empty() )
            return static_cast<T>( 0 );
        T ret;
        io_memmove( &ret, position_, sizeof(T) );
        shift( sizeof(T) );
        return ret;
    }

public:
    /// Allocate a memory block for buffer from heap
    /// \param ec operation error code, will have out of memory in case of error
    /// \param capacity buffer capacity in bytes
    /// \return new buffer, or empty buffer if no more memory left
    static byte_buffer allocate(std::error_code& ec, std::size_t capacity) noexcept;

    /// Allocate a memory block from heap, and deep copy array of fundamental or trivial type
    /// \param T fundamental or trivial type
    /// \param arr pointer to the array first element
    /// \param size array length
    /// \return new buffer, or empty buffer if no more memory left
    template<typename T>
    static inline byte_buffer wrap(std::error_code& ec, const T* arr, std::size_t size) noexcept {
        static_assert( std::is_fundamental<T>::value || std::is_trivial<T>::value, "Must be an array of trivial or fundamental type" );
        if(0 == size)
            return byte_buffer();
        byte_buffer res = allocate(ec,  size * sizeof(T) );
        if( !ec ) {
        	res.put(arr, size);
        	res.flip();
        	return byte_buffer( std::move(res) );
        }
        return byte_buffer();
    }

    template<typename T>
    static byte_buffer wrap(std::error_code& ec, const T* begin,const T* end) noexcept {
        static_assert( std::is_fundamental<T>::value || std::is_trivial<T>::value, "Must be an array of trivail or fundamental type" );
        if(end <= begin) {
			ec = std::make_error_code(std::errc::argument_out_of_domain);
            return byte_buffer();
        }
        return wrap( ec, begin, memory_traits::distance(begin,end) );
    }

	/// Wrap C style zero ending string to buffer

    template<typename __char_type>
    static inline byte_buffer wrap(std::error_code& ec, const __char_type* str) noexcept {
    	static_assert( !std::is_same<__char_type, bool>::value && std::numeric_limits<__char_type>::is_integer, "__char_type must be integer type, and not bool");
        typedef std::char_traits<__char_type> traits;
        // for C style ending + 1
        return wrap( ec,  str, traits::length(str)+1 );
    }

private:
    detail::mem_block arr_;
    std::size_t capacity_;
    uint8_t* position_;
    uint8_t* last_;
};


/// Converts this buffer into STL string of provided type by deep copying all bytes between
/// buffer position and last iterators into STL string
/// \return STL string storing buffer content
/// \throw potential std::bad_alloc
template<class __char_type>
inline std::basic_string<__char_type> to_string(byte_buffer& buff)
{
    typedef std::basic_string<__char_type> str_type;
    typedef typename str_type::const_pointer char_ptr_t;
    if(buff.empty())
        return str_type();
    char_ptr_t f = reinterpret_cast<char_ptr_t>(buff.position().get());
    char_ptr_t l = reinterpret_cast<char_ptr_t>(buff.last().get());
    str_type result(f,l-1);
    result.shrink_to_fit();
    return std::move(result);
}

} // namespace io

#endif // __IO_BUFFER_HPP_INCLUDED__
