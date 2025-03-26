/**
 * @package   libSocket: C++ sockets library.
 * @brief     Sockets of the UNIX domain
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#include "packet_raw.h"
#include <algorithm>                    //
#include <system_error>                 // std::system_error

#include <string.h>                     // memset, strncpy
#include <sys/ioctl.h>                  // ioctl
#include <unistd.h>                     // close
#include <net/if.h>                     // ifreq
#include <netpacket/packet.h>           // sockaddr_ll

namespace libSocket { namespace raw {

/** ----------------------------------------------------
 * @brief     Class EtherPacket: A raw ethernet packet.
 * ------ */

/**
 * @brief     Copy the buffer into the payload
 */
int EtherPacket::setPayload (const std::basic_string<uint8_t>& data)
{
    int length = std::min(data.size(), sizeof(payload));
    std::copy(data.begin(), data.begin() + length, payload);
    packet_len = ETH_HLEN + length;
    return length;
}

/**
 * @brief     Set the destination MAC address
 */
void EtherPacket::setDestination (const std::basic_string<uint8_t>& dest_mac)
{
    int length = std::min(dest_mac.size(), static_cast<size_t>(ETH_ALEN));
    std::copy(dest_mac.begin(), dest_mac.begin() + length, ether_dhost);
}

/** ----------------------------------------------------
 * @brief     Class PacketRawSock: Raw ethernet packet socket.
 * ------ */

/**
 * @brief     Open the socket and bind it to a network interface and a protocol.
 */
PacketRawSock::PacketRawSock (const std::string& iface, uint16_t protocol)
    : SocketBase(PF_PACKET, SOCK_RAW, htons(protocol)), protocol_(htons(protocol)), mac_()
{
    ifreq ifr = {};
    std::copy(iface.begin(), iface.begin() + std::min(iface.size(), sizeof(ifr.ifr_name)), ifr.ifr_name);
    if (ioctl(hsock_, SIOCGIFINDEX, &ifr) < 0)
        throw std::system_error(errno, std::generic_category(), "PacketRawSock: ioctl(SIOCGIFINDEX)");

    sockaddr_ll addr = {};
    addr.sll_family   = PF_PACKET;
    addr.sll_protocol = protocol_;
    addr.sll_ifindex  = ifr.ifr_ifindex;
    addr.sll_pkttype  = PACKET_HOST;
    if (::bind(hsock_, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_ll)) != 0)
        throw std::system_error(errno, std::generic_category(), "PacketRawSock: bind");

    if (ioctl(hsock_, SIOCGIFADDR, &ifr) != 0)          // Get local address (IP)
        throw std::system_error(errno, std::generic_category(), "PacketRawSock: ioctl(SIOCGIFHWADDR)");
    local_addr_ = reinterpret_cast<sockaddr_in*>(ifr.ifr_addr.sa_data)->sin_addr.s_addr;

    if (ioctl(hsock_, SIOCGIFHWADDR, &ifr) != 0)        // Get hardware address (MAC)
        throw std::system_error(errno, std::generic_category(), "PacketRawSock: ioctl(SIOCGIFHWADDR)");
    std::copy(ifr.ifr_hwaddr.sa_data, ifr.ifr_hwaddr.sa_data + ETH_ALEN, mac_);

    if (ioctl(hsock_, SIOCGIFMTU, &ifr) != 0)           // Get interface MTU
        throw std::system_error(errno, std::generic_category(), "PacketRawSock: ioctl(SIOCGIFMTU)");
    mtu_ = ifr.ifr_mtu;
}

/**
 * @brief     Read a packet from the socket
 */
void PacketRawSock::readPacket(EtherPacket& pkt)
{
    checkValid();
    auto lesen = ::read(hsock_, pkt, sizeof(pkt));
    if (lesen < 0)
        throw std::system_error(errno, std::generic_category(), "PacketRawSock::readPacket: read");
    pkt.packet_len = lesen;
}

/**
 * @brief     Read a packet from the socket without extracting it from the receive buffer.
 */
void PacketRawSock::peekPacket(EtherPacket& pkt)
{
    checkValid();
    auto lesen = ::recv(hsock_, pkt, sizeof(pkt), MSG_PEEK);
    if (lesen < 0)
        throw std::system_error(errno, std::generic_category(), "PacketRawSock::peekPacket: recv");
    pkt.packet_len = lesen;
}

/**
 * @brief     Write a packet to the socket
 */
void PacketRawSock::writePacket (EtherPacket& pkt)
{
    checkValid();
    std::copy(mac_, mac_ + sizeof(mac_), pkt.ether_shost);
    pkt.ether_type = protocol_;
    if (::write(hsock_, pkt, pkt.packet_len) < 0)
        throw std::system_error(errno, std::generic_category(), "PacketRawSock::writePacket: write");
}

/**
 * @brief     Get the MAC address of the bound interface.
 */
std::basic_string<uint8_t> PacketRawSock::getMac ()
{
    std::basic_string<uint8_t> retval;
    retval.assign(mac_, sizeof(mac_));
    return retval;
}

/**
 * @brief     Get the IP address of the bound interface.
 */
in_addr_t PacketRawSock::getLocalAddr()
{
    return local_addr_;
}

/**
 * @brief     Get the MTU of the local network interface.
 */
int PacketRawSock::getMtu ()
{
    return mtu_;
}

} } // namespaces
