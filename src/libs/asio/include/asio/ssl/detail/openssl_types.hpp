//
// ssl/detail/openssl_types.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_SSL_DETAIL_OPENSSL_TYPES_HPP
#define ASIO_SSL_DETAIL_OPENSSL_TYPES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

//
// Begining of PeerConnect patch to support BoringSSL instead of OpenSSL
//

#if defined(WIN32) || defined(_WIN32)
#pragma warning(disable: 4018 4002)
// Undefine the macros which conflict with OpenSSL and define replacements. See
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa378145(v=vs.85).aspx
#undef X509_CERT_PAIR
#undef X509_EXTENSIONS
#undef X509_NAME
#define WINCRYPT_X509_CERT_PAIR ((LPCSTR) 53)
#define WINCRYPT_X509_EXTENSIONS ((LPCSTR) 5)
#define WINCRYPT_X509_NAME ((LPCSTR) 7)
#endif // WIN32 || _WIN32

#ifdef __cplusplus
extern "C" {
#endif
#if !defined(SSL_R_SHORT_READ)
# define SSL_R_SHORT_READ    SSL_R_UNEXPECTED_RECORD
#endif // !defined(SSL_R_SHORT_READ)
  inline void CONF_modules_unload(int p) {}
#ifdef __cplusplus
}
#endif

//
// End of PeerConnect patch

#include "asio/detail/config.hpp"
#include <openssl/conf.h>
#include <openssl/ssl.h>
#if !defined(OPENSSL_NO_ENGINE)
# include <openssl/engine.h>
#endif // !defined(OPENSSL_NO_ENGINE)
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include "asio/detail/socket_types.hpp"

#endif // ASIO_SSL_DETAIL_OPENSSL_TYPES_HPP
