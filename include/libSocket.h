/**
 * @package   libSocket: C++ sockets library.
 * @brief     General library header.
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#ifndef _LIBSOCKET_H_
#define _LIBSOCKET_H_

#include "libSocket/socket_base.h"
#include "libSocket/functions.h"
#include "libSocket/inet.h"
#include "libSocket/unix.h"
#include "libSocket/rawpacket.h"

namespace libSocket {
/**
 * @brief   Library version identifier
 */
const char* version();
}

#endif // _LIBSOCKET_H_
