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
#ifndef __IO_TUPLE_META_REFLECT_HPP_INCLUDED__
#define __IO_TUPLE_META_REFLECT_HPP_INCLUDED__

#include "config.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace meta {

namespace {

typedef int index_t;

// tuple for each util
template<index_t...>
struct index_tuple {};

template<index_t I, typename __index_tuple, typename... __types>
struct make_indexes_impl;

template<index_t I, index_t... __indexes, typename T, typename ... __types>
struct make_indexes_impl<I, index_tuple<__indexes...>, T, __types...> {
	typedef typename make_indexes_impl<I + 1, index_tuple<__indexes..., I>, __types...>::type type;
};

template<index_t I, index_t... __indexes>
struct make_indexes_impl<I, index_tuple<__indexes...> > {
	typedef index_tuple<__indexes...> type;
};

template<typename ... __types>
struct make_indexes : make_indexes_impl<0, index_tuple<>, __types...>
{};

template<typename __functor, typename __last>
static constexpr inline void for_each_impl(__functor&& f, __last&& last)
{
	f(last);
}

template<typename __functor, typename __first, typename ... __rest>
static constexpr inline void for_each_impl(__functor&& f, __first&& first, __rest&&...rest)
{
	f(first);
	for_each_impl( std::forward<__functor>(f), rest...);
}

template<typename __functor, int ... __indexes, typename ... __args>
static constexpr inline void for_each_helper(__functor&& f, index_tuple<__indexes...>, std::tuple<__args...>&& tup)
{
	for_each_impl( std::forward<__functor>(f), std::forward<__args>(std::get<__indexes>(tup))... );
}

} // namespace {

template<typename __functor, typename ... __args>
static constexpr inline void for_each( std::tuple<__args...>& tup, __functor&& f)
{
	for_each_helper(std::forward<__functor>(f),
	                typename make_indexes< __args... >::type(),
	                std::forward< std::tuple<__args...> >(tup) );
}

template<typename __functor, typename ... __args>
static constexpr inline void for_each( std::tuple<__args...>&& tup, __functor&& f)
{
	for_each_helper(std::forward<__functor>(f),
	                typename make_indexes<__args...>::type(),
	                std::forward<std::tuple<__args...> >(tup) );
}

// compile time util

typedef std::true_type  true_type;
typedef std::false_type false_type;

} // namesapce meta

} // namesapce io

#endif // __IO_TUPLE_META_REFLECT_HPP_INCLUDED__
