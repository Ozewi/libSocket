/**
 * @package   libSocket: C++ sockets library.
 * @brief     Version info.
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

constexpr char LIBRARY_VERSION[] = "libSocket v2.0";

namespace libSocket {

/**
 * @brief   Library version identifier
 */
const char* version()
{
  return LIBRARY_VERSION;
}

} // namespace
