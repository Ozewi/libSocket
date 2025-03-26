/**
 * @package   libSocket: C++ sockets library.
 * @brief     General-purpose independent functions.
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#ifndef _LIBSOCKET_FUNCTIONS_H_
#define _LIBSOCKET_FUNCTIONS_H_

#include <sys/socket.h>
#include <netinet/in.h>                 // sockaddr_in
#include <sys/un.h>                     // sockaddr_un
#include <string>
#include <vector>

namespace libSocket {

/** ----------------------------------------------------
 * @brief   Independent functions
 * ------ */

/**
 * @brief   Get a list of the network interfaces
 */
std::vector<std::string>                                /** @return List of local network interfaces */
getInterfaceList();

/**
 * @brief   Get the MAC address of the interface.
 * @see     getInterfaceList to get a list of available interfaces.
 */
std::basic_string<uint8_t>                              /** @return Hardware Address */
getMac (
    const std::string& iface                            //!< Network interface name ("eth0", "lo", etc)
);

} // namespace

#endif  // _LIBSOCKET_FUNCTIONS_H_
