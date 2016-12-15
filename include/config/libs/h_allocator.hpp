#ifndef __IO_H_ALLOCATOR_HPP_INCLUDED__
#define __IO_H_ALLOCATOR_HPP_INCLUDED__

#include <utility>

#include <new>

namespace io {

template<typename T>
class h_allocator {
private:
	template<typename _Tp>
	static constexpr inline _Tp* address_of(_Tp& __r) noexcept
	{
		return reinterpret_cast<_Tp*>
		       (&const_cast<uint8_t&>(reinterpret_cast<const volatile uint8_t&>(__r)));
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
		typedef h_allocator<T1> other;
	};

	constexpr h_allocator() noexcept
	{ }

	constexpr h_allocator(const h_allocator&) noexcept
	{ }

	template<typename T1>
	constexpr h_allocator(const h_allocator<T1>&) noexcept
	{ }

	~h_allocator() noexcept = default;

	constexpr inline pointer address(reference __x) const noexcept
	{
		return address_of(__x);
	}

	constexpr inline const_pointer address(const_reference __x) const noexcept
	{
		return address_of<const_pointer>(__x);
	}
#ifndef IO_NO_EXCEPTIONS
	pointer allocate(size_type __n, const void* = 0)
	{
		void * result = io::h_malloc(__n * sizeof(value_type));
		if(NULL == result)
			throw std::bad_alloc();
		return static_cast<pointer>(result);
	}
#else
	pointer allocate(size_type __n, const void* = 0) noexcept
	{
		void *result = nullptr;
		const std::size_t size = __n * sizeof(value_type);
#ifdef __GNUC__
		while ( __builtin_expect( (result = io::h_malloc(size) ) == nullptr, false) ) {
#else
		result = io::h_malloc(size);
		for(; nullptr == result; result = io::h_malloc(size) ) { ;
#endif // __GNUC__
			if(nullptr != result) {
				std::new_handler handler = std::get_new_handler();
				if(nullptr == handler)
					return nullptr;
				handler();
			}
		}
		return static_cast<pointer>(result);
	}
#endif // IO_NO_EXCEPTIONS

	// __p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type) noexcept
	{
		assert(nullptr != __p);
		io::h_free(__p);
	}

	// addon to std::allocator make this noexcept(true) if constructing type is also noexcept
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

	size_type max_size() const noexcept
	{
		return SIZE_MAX / sizeof(value_type);
	}
};

template<typename _Tp>
constexpr inline bool operator==(const h_allocator<_Tp>&, const h_allocator<_Tp>&)
{
	return true;
}

template<typename _Tp>
constexpr inline bool operator!=(const h_allocator<_Tp>&, const h_allocator<_Tp>&)
{
	return false;
}

} // namespace io

#endif // __IO_H_ALLOCATOR_HPP_INCLUDED__
