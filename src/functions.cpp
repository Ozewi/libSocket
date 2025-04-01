/**
 * @package   libSocket: C++ sockets library.
 * @brief     General-purpose independent functions.
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#include "functions.h"
#include <sys/ioctl.h>
#include <net/if.h>                                     // if_nameindex, ifreq
#include <arpa/inet.h>                                  // inet_ntop
#include <vector>
#include <system_error>
#include <netinet/in.h>
namespace libSocket {

/** ----------------------------------------------------
 * @brief   Independent functions
 * ------ */

/**
 * @brief   Get a list of the network interfaces
 */
std::vector<std::string> getInterfaceList()
{
    std::vector<std::string> retval;
    auto name_ix = if_nameindex();
    if (name_ix == nullptr)
        throw std::system_error(errno, std::generic_category(), "if_nameindex");
    for (auto ptr = name_ix; ptr->if_index != 0 || ptr->if_name != nullptr; ptr++)
        retval.push_back(ptr->if_name);
    return retval;
}

/**
 * @brief   Get the MAC address of the interface.
 */
std::basic_string<uint8_t> getMac(const std::string& iface) noexcept
{
    std::basic_string<uint8_t> mac;
    auto hsock = socket(PF_INET, SOCK_DGRAM, 0);
    ifreq ifr = {};
    std::copy(iface.begin(), iface.begin() + std::min(iface.size(), sizeof(ifr.ifr_name)), ifr.ifr_name);
    if (ioctl(hsock, SIOCGIFHWADDR, &ifr) == 0)
        for (int ix = 0; ix < IFHWADDRLEN; ix++)
            mac.push_back(ifr.ifr_ifru.ifru_hwaddr.sa_data[ix]);
    return mac;
}

/**
 * @brief   Get the local IP address of the interface.
 */
std::string getLocalAddr (const std::string& iface)
{
    ifreq ifr = {};
    std::copy(iface.begin(), iface.begin() + std::min(iface.size(), sizeof(ifr.ifr_name) - 1), ifr.ifr_name);
    auto hsock = socket(PF_INET, SOCK_DGRAM, 0);
    if (ioctl(hsock, SIOCGIFADDR, &ifr) < 0)
        throw std::system_error(errno, std::generic_category(), "SIOCGIFADDR");
    char tmp[128];
    if (inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in*>(&ifr.ifr_addr)->sin_addr), tmp, sizeof(tmp)) == nullptr)
        return "";
    return tmp;
}

} // namespace
