/**
 * @package   libSocket: C++ sockets library.
 * @brief     Sockets of the UNIX domain
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#ifndef _LIBSOCKET_UNX_UNIX_H_
#define _LIBSOCKET_UNX_UNIX_H_

#include "socket_base.h"
#include <netinet/if_ether.h>           // ether_header
#include <netinet/in.h>                 // in_addr_t

namespace libSocket { namespace raw {

/** ----------------------------------------------------
 * @brief   Class EtherPacket: A raw ethernet packet.
 * @details ether_header is a struct defined in net/ethernet.h this way:
 *          struct ether_header
 *          {
 *            u_int8_t  ether_dhost[ETH_ALEN];      // destination eth addr
 *            u_int8_t  ether_shost[ETH_ALEN];      // source ether addr
 *            u_int16_t ether_type;                 // packet type ID field
 *          } __attribute__ ((__packed__));
 * @note    Deriving from a conventional struct works as long as there are no virtual members.
 * ------ */
struct EtherPacket : public ether_header
{
    uint8_t payload [ETH_DATA_LEN];                     //!< Payload of the packet.
    uint16_t packet_len;                                //!< Full length of the payload.

    /**
     * @brief   Default constructor.
     * @details Initial length of the packet is that of the header with no payload.
     */
    EtherPacket() : payload{}, packet_len(ETH_HLEN) {};

    /**
     * @brief   Parametric constructor.
     * @details Copies the provided data into the payload buffer.
     */
    EtherPacket (
        const std::basic_string<uint8_t>& data          //!< Data to copy as payload.
    )   {
            setPayload(data);
        };

    /**
     * @brief   Get a pointer to the payload of the packet
     */
    const uint8_t*
    getPayload() const
        {
            return payload;
        };

    /**
     * @brief   Get length of the payload.
     */
    int
    getPayloadLen() const
        {
            return packet_len - ETH_HLEN;
        };

    /**
     * @brief   Copy the buffer into the payload
     * @details Payload length is limited to ETH_DATA_LEN; data beyond that limit won't be copied.
     */
    int                                                 /** @return Bytes actually copied. */
    setPayload (
        const std::basic_string<uint8_t>& data          //!< Data to copy as payload.
    );

    /**
     * @brief   Set the destination MAC address.
     * @details MAC address length is ETH_ALEN.
     */
    void
    setDestination (
        const std::basic_string<uint8_t>& dest_mac      //!< Data to copy as destination MAC address
    );

    /**
     * @brief   Get a pointer to the struct.
     * @details Used by read/write functions.
     */
    operator void* ()
        {
            return this;
        };

} __attribute__((packed));

/** ----------------------------------------------------
 * @brief   Class PacketRawSock: Raw ethernet packet socket.
 * ------ */
class PacketRawSock : public SocketBase
{
public:
    /**
     * @brief   Open the socket and bind it to a network interface and a protocol.
     * @throws  std::system_error
     * @see     libSocket::getInterfaceList for a list of available interfaces.
     * @see     include/linux/if_ether.h for a list of standard ethernet protocols.
     */
    PacketRawSock (
        const std::string& iface,                       //!< Network interface name ("eth0", "lo", etc)
        uint16_t protocol                               //!< Ethernet protocol (0x0800 = IP, etc)
    );

    /**
     * @brief   Read a packet from the socket
     * @throws  std::system_error on read error.
     */
    void
    readPacket (
        EtherPacket& pkt                                //!< Struct that will contain the received packet
    );

    /**
     * @brief   Read a packet from the socket without extracting it from the receive buffer.
     * @throws  std::system_error on read error.
     */
    void
    peekPacket (
        EtherPacket& pkt                                //!< Struct to contain the read packet
    );

    /**
     * @brief   Write a packet to the socket
     * @details The destination MAC address must be already copied into the packet. @see EtherPacket::setDestination.
     * @throws  std::system_error on write error.
     */
    void
    writePacket (
        EtherPacket& pkt                                //!< Packet to send
    );

    /**
     * @brief   Get the MAC address of the bound interface.
     */
    std::basic_string<uint8_t>                          /** @return MAC address of the bound interface. */
    getMac ();

    /**
     * @brief   Get the IP address of the bound interface.
     */
    in_addr_t                                           /** @return Local IP address as integer (network byte order). */
    getLocalAddr ();

    /**
     * @brief   Get the MTU of the local network interface.
     * @details This is the MTU due to media limits of the local interface. The network to which the interface is connected
     *          may impose additional restrictions.
     */
    int                                                 /** @return MTU of the interface. */
    getMtu ();

protected:
    uint16_t  protocol_;                                //!< Ethernet protocol of the socket.
    uint8_t   mac_[ETH_ALEN];                           //!< MAC address of the bound interface.
    int       mtu_;                                     //!< MTU of the bound network.
    in_addr_t local_addr_;                              //!< Local address (IP)
};

} } // namespaces

#endif  // _LIBSOCKET_UNX_UNIX_H_
