#ifndef __IO_H_ALLOCATOR_HPP_INCLUDED__
#define __IO_H_ALLOCATOR_HPP_INCLUDED__

#include <memory>
#include <new>
#include <utility>

namespace io {

namespace {

class no_exept_mode {
	no_exept_mode() = delete;
	~no_exept_mode() = delete;
	no_exept_mode(const no_exept_mode&) = delete;
	no_exept_mode operator=(const no_exept_mode&) = delete;
public:
#ifdef IO_NO_EXCEPTIONS
	static constexpr bool is_nothrow = true;
#else
	static constexpr bool is_nothrow = false;
#endif // IO_NO_EXCEPTIONS
};


} // namesapace

template<typename T, class __memory_traits>
class h_allocator {
private:
	template<typename _Tp>
	static constexpr inline _Tp* address_of(_Tp& __r) noexcept
	{
		return std::addressof( __r );
	}
	template< typename _T>
	static constexpr _T* uncast_void(void * const ptr) {
#ifdef __GNUC__
		return static_cast<_T*>( __builtin_assume_aligned(ptr, sizeof(_T) ) );
#else
		return static_cast<_T*>( ptr );
#endif
	}
public:
	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T&  reference;
	typedef const T& const_reference;
	typedef T value_type;

	template<typename T1>
	struct rebind {
		typedef h_allocator<T1, __memory_traits> other;
	};

	constexpr h_allocator() noexcept
	{ }

	constexpr h_allocator(const h_allocator&) noexcept
	{ }

	template<typename T1>
	constexpr h_allocator(const h_allocator<T1, __memory_traits>&) noexcept
	{ }

	~h_allocator() noexcept = default;

	constexpr pointer address(reference __x) const noexcept
	{
		return address_of<pointer>(__x);
	}

	constexpr const_pointer address(const_reference __x) const noexcept
	{
		return address_of<const_pointer>(__x);
	}

	pointer allocate(size_type __n, const void* = 0) noexcept(no_exept_mode::is_nothrow)
	{
		assert( 0 != __n );
		void *result;
		const size_t bytes_size = sizeof(value_type) * __n;
		result = __memory_traits::malloc(bytes_size);
		if(nullptr != result)
			return uncast_void<value_type>(result);
#ifdef __GNUC__
		while ( __builtin_expect( (result = __memory_traits::malloc(bytes_size) ) == nullptr, false) )
#else
		while( nullptr == (result = __memory_traits::malloc(bytes_size) ) )
#endif // __GNUC__
		{
			std::new_handler handler = std::get_new_handler();
			if (nullptr == handler)
#ifdef IO_NO_EXCEPTIONS
				return nullptr;
#else
				throw std::bad_alloc();
#endif // IO_NO_EXCEPTIONS
			handler();
		}
		return uncast_void<value_type>(result);
	}

	// __p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type) noexcept
	{
		assert(nullptr != __p);
		__memory_traits::free(__p);
	}

	// addon to std::allocator make this noexcept(true) if constructor is also noexcept
	template<typename _Up, typename... _Args>
	__forceinline void construct(_Up* __p, _Args&&... __args) noexcept( noexcept( _Up(std::forward<_Args>(__args)...) ) )
	{
		assert(nullptr != __p);
		::new( static_cast<void *>(__p) ) _Up(std::forward<_Args>(__args)...);
	}

	template<typename _Up>
	__forceinline void  destroy(_Up* __p) noexcept( noexcept( __p->~_Up() ) )
	{
		__p->~_Up();
	}

	constexpr size_type max_size() const noexcept
	{
		return SIZE_MAX / sizeof(value_type);
	}
};

template<typename _Tp, class _Mt>
constexpr inline bool operator==(const h_allocator<_Tp,_Mt>&, const h_allocator<_Tp,_Mt>&)
{
	return true;
}

template<typename _Tp, class _Mt>
constexpr inline bool operator!=(const h_allocator<_Tp,_Mt>&, const h_allocator<_Tp,_Mt>&)
{
	return false;
}

} // namespace io

#endif // __IO_H_ALLOCATOR_HPP_INCLUDED__
