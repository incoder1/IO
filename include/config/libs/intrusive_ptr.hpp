/*
 *
 * Copyright (c) 2016-2021
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __INSTRUSIVE_PTR_HPP_INCLUDED__
#define __INSTRUSIVE_PTR_HPP_INCLUDED__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <assert.h>
#include <functional>

namespace boost {

namespace detail
{

template< class Y, class T >
struct sp_convertible
{
    typedef uint8_t (&yes) [1];
    typedef uint8_t (&no)  [2];

    static yes f( T* );
    static no  f( ... );

    enum _vt { value = sizeof( (f)( static_cast<Y*>(0) ) ) == sizeof(yes) };
};

template< class Y, class T > struct sp_convertible< Y, T[] >
{
    enum _vt { value = false };
};

template< class Y, class T > struct sp_convertible< Y[], T[] >
{
    enum _vt { value = sp_convertible< Y[1], T[1] >::value };
};

template< class Y, size_t N, class T > struct sp_convertible< Y[N], T[] >
{
    enum _vt { value = sp_convertible< Y[1], T[1] >::value };
};

struct sp_empty
{
};

template< bool > struct sp_enable_if_convertible_impl;

template<> struct sp_enable_if_convertible_impl<true>
{
    typedef sp_empty type;
};

template<> struct sp_enable_if_convertible_impl<false>
{
};

template< class Y, class T >
	struct sp_enable_if_convertible: public sp_enable_if_convertible_impl< sp_convertible< Y, T >::value >
{
};

} // namespace detail

template<class T>
class intrusive_ptr
{
private:

    typedef intrusive_ptr this_type;

public:

    typedef T element_type;

    constexpr intrusive_ptr() noexcept:
    	px_(nullptr)
    {}

    intrusive_ptr(T * p, bool add_ref = true ) noexcept:
    	px_(p)
    {
        if(nullptr != px_ && add_ref )
			intrusive_ptr_add_ref(px_);
    }

	intrusive_ptr(intrusive_ptr const & rhs) noexcept:
        	px_( rhs.px_ )
    {
        if( nullptr != px_ )
			intrusive_ptr_add_ref(px_);
    }

    template<class U>
    intrusive_ptr( intrusive_ptr<U> const & rhs,
		typename detail::sp_enable_if_convertible<U,T>::type = detail::sp_empty()
				) noexcept:
    	px_(rhs.get())
    {
    	 if( px_ != nullptr )
			intrusive_ptr_add_ref(px_);
    }

    inline ~intrusive_ptr() noexcept
    {
        if( px_ != nullptr )
			intrusive_ptr_release(px_);
    }

	intrusive_ptr(intrusive_ptr && rhs) noexcept:
		px_(rhs.px_)
    {
        rhs.px_ = nullptr;
    }

	intrusive_ptr & operator=(intrusive_ptr && rhs) noexcept
    {
        this_type( static_cast< intrusive_ptr && >( rhs ) ).swap(*this);
        return *this;
    }

	explicit operator bool () const noexcept
    {
        return nullptr != px_;
    }

	intrusive_ptr& operator=(intrusive_ptr const & rhs) noexcept
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(T * rhs) noexcept
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset() noexcept
    {
        this_type().swap(*this);
    }

    void reset(T * rhs ) noexcept
    {
        this_type( rhs ).swap(*this);
    }

    void reset(T * rhs, bool add_ref ) noexcept
    {
        this_type(rhs,add_ref).swap(*this);
    }

    inline T * get() const noexcept
    {
        return px_;
    }

    inline T* detach() noexcept
    {
        T * ret = px_;
        px_ = nullptr;
        return ret;
    }

    inline T& operator*() const noexcept
    {
        assert( px_ != 0 );
        return *px_;
    }

    inline T * operator->() const noexcept
    {
        assert( px_ != nullptr);
        return px_;
    }

    inline void swap(intrusive_ptr & rhs) noexcept
    {
        T* tmp = px_;
        px_ = rhs.px_;
        rhs.px_ = tmp;
    }

private:
    T * px_;
};

template<class T, class U>
inline bool operator==(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U>
inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U>
inline bool operator==(intrusive_ptr<T> const & a, U * b)
{
    return a.get() == b;
}

template<class T, class U>
inline bool operator!=(intrusive_ptr<T> const & a, U * b)
{
    return a.get() != b;
}

template<class T, class U>
inline bool operator==(T * a, intrusive_ptr<U> const & b)
{
    return a == b.get();
}

template<class T, class U>
inline bool operator!=(T * a, intrusive_ptr<U> const & b)
{
    return a != b.get();
}

namespace detail {
	typedef decltype(nullptr) nullptr_t;
}

template<class T>
inline bool operator==( intrusive_ptr<T> const & p, detail::nullptr_t ) noexcept
{
    return p.get() == nullptr;
}

template<class T>
inline bool operator==( detail::nullptr_t, intrusive_ptr<T> const & p ) noexcept
{
    return p.get() == nullptr;
}

template<class T>
inline bool operator!=( intrusive_ptr<T> const & p, detail::nullptr_t ) noexcept
{
    return p.get() != nullptr;
}

template<class T>
inline bool operator!=(detail::nullptr_t, intrusive_ptr<T> const & p ) noexcept
{
    return p.get() != nullptr;
}

template<class T>
inline bool operator<(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b)
{
    return a.get() < b.get();
}

template<class T>
void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs)
{
    lhs.swap(rhs);
}

template<class T>
T * get_pointer(intrusive_ptr<T> const & p)
{
    return p.get();
}

template<class T, class U>
intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U>
intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const & p)
{
    return const_cast<T *>(p.get());
}

#ifndef IO_NO_RTTI
template<class T, class U>
intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const & p)
{
    return dynamic_cast<T *>( p.get() );
}
#endif // IO_NO_RTTI

} // namesapce boost

namespace std {

	template< class T > std::size_t hash_value(boost::intrusive_ptr<T> const & p )
	{
		return std::hash< T* >() ( p.get() );
	}

	template<typename T>
	struct hash< boost::intrusive_ptr<T> >
	{
	public:
		constexpr std::size_t operator()(boost::intrusive_ptr<T> const & p) const {
			return hash_value<T>(p);
		}
	};

} // namesapce std

#endif // __INSTRUSIVE_PTR_HPP_INCLUDED__
