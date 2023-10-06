/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __URI_HPP_INCLUDED__
#define __URI_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <functional>
#include <ostream>
#include <type_traits>

#include <io/core/conststring.hpp>

namespace io {

namespace net {


class uri;
DECLARE_IPTR(uri);

/// \brief Unique Resource Identifier object
class IO_PUBLIC_SYMBOL uri final:public object {
public:
    /// Parses URI from string
    /// \param ec hold parsing error if any, or out of memory error
    /// \param str string to parse
    static s_uri parse(std::error_code& ec, const char* str) noexcept;

    /// Parses URI from string
    /// \param ec hold parsing error if any, or out of memory error
    /// \param str string to parse
    static inline s_uri parse(std::error_code& ec, const std::string& str) noexcept
    {
        return parse(ec, str.c_str() );
    }

    /**
    * Lookup a default port number for well know protocol schema
    *   Known schemas:
    *   <ul>
    *      <li>echo 7</li>
    *       <li>daytime 13</li>
    *       <li>ftp 21</li>
    *       <li>ssh 22</li>
    *       <li>telnet 23</li>
    *       <li>mailto 25</li>
    *       <li>time 37</li>
    *       <li>name 42</li>
    *       <li>domain 53</li>
    *       <li>gopher 70</li>
    *       <li>http 80</li>
    *       <li>https 443</li>
    *       <li>npp 92</li>
    *       <li>sftp 115</li>
    *       <li>irc 6697</li>
    *   </ul>
    *   \param  scheme protocol scheme name
    *   \return default port number for know scheme, or 0 for unknown
    */
    static uint16_t __noinline default_port_for_scheme(const char* scheme) noexcept;

private:


    friend class nobadalloc<uri>;

    uri(
        const_string&& scheme,
        uint16_t port,
        const_string&& host,
        const_string&& user_info,
        const_string&& path,
        const_string&& query,
        const_string&& fragment) noexcept;

public:
    /// Returns this URI scheme
    /// \return scheme
    inline const const_string& scheme() const noexcept {
        return scheme_;
    }
    /// Returns this URI port
    /// \return port number from URI or default port for scheme
    uint16_t port() const noexcept {
        return port_;
    }

    /// Returns this URI host, if any
    /// \return host
    inline const const_string& host() const noexcept {
        return host_;
    }

    /// Returns this URI user_info, if any
    /// \return user_info
    inline const const_string& user_info() const noexcept {
        return user_info_;
    }

    /// Returns this URI path
    /// \return path if present
    inline const const_string& path() const noexcept {
        return path_;
    }

    /// Returns this URI query
    /// \return query if present
    inline const const_string& query() const noexcept {
        return query_;
    }

    /// Returns this URI fragment
    /// \return fragment if present
    inline const const_string& fragment() const noexcept {
        return fragment_;
    }

    /// Making hash value for this URI
    /// \return hash value for this URI
    std::size_t hash() const noexcept;

private:
    uint16_t port_;
    const_string scheme_;
    const_string host_;
    const_string user_info_;
    const_string path_;
    const_string query_;
    const_string fragment_;
};

inline std::ostream& operator<<(std::ostream& ret, const uri& uri)
{
    if(!uri.scheme().empty()) {
        ret << uri.scheme() << ':';
        ret << "//";
        ret << uri.host();
    } else {
        ret << "//";
    }
    if(0 != uri.port() &&
            uri.port() != uri::default_port_for_scheme( uri.scheme().data() ) )
        ret << ':'<< uri.port();
    ret << uri.path();
    if( !uri.user_info().empty() )
        ret << '@' << uri.user_info();
    if( !uri.query().empty() )
        ret << '?' << uri.query();
    if( !uri.fragment().empty() )
        ret << '#' << uri.fragment();
    return ret;
}

} // namespace net

} // namespace io

#endif // __URI_HPP_INCLUDED__
