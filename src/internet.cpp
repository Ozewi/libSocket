/**
 * @package   libSocket: C++ sockets library.
 * @brief     Sockets of the INET domain (TCP/IP)
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#include "internet.h"
#include <netdb.h>
#include <net/if.h>                                     // ifreq
#include <sys/ioctl.h>                                  // ioctl
#include <netinet/tcp.h>                                // TCP constants
#include <algorithm>
#include <system_error>
#include <string>

namespace libSocket { namespace inet {

using namespace std::string_literals;

/** ----------------------------------------------------
 * @brief     Class inet::Address
 * ------ */

/**
 * @brief   Constructor with integers
 */
Address::Address (in_addr_t addr, in_port_t port)
{
    addr_.sin_family = PF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(addr);
    std::fill_n(addr_.sin_zero, sizeof(addr_.sin_zero), 0);
}

/**
 * @brief   Constructor with name resolver
 */
Address::Address (const std::string& addr, in_port_t port)
{
    hostent* host = gethostbyname(addr.c_str());
    if (host == nullptr)
        throw std::system_error(EIO, std::generic_category(), "inet::Address: gethostbyname:"s + hstrerror(h_errno));
    addr_.sin_family = PF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = *reinterpret_cast<in_addr_t*>(host->h_addr_list[0]);
    std::fill_n(addr_.sin_zero, sizeof(addr_.sin_zero), 0);
}

/** ----------------------------------------------------
 * @brief     Class InetBase
 * ------ */

/**
 * @brief     Get the local address of the socket
 */
Address InetBase::getAddress ()
{
    checkValid();
    sockaddr_in addr;
    socklen_t sz = sizeof(sockaddr_in);
    if (getsockname(hsock_, reinterpret_cast<sockaddr*>(&addr), &sz) < 0)
        throw std::system_error(errno, std::generic_category(), "InetBase::getAddress: getsockname");
    return Address(addr.sin_addr.s_addr, addr.sin_port);
}

/**
 * @brief     Get the address of the remote endpoint
 */
Address InetBase::getPeerAddress ()
{
    checkValid();
    sockaddr_in addr;
    socklen_t sz = sizeof(sockaddr_in);
    if (getpeername(hsock_, reinterpret_cast<sockaddr*>(&addr), &sz) < 0)
        throw std::system_error(errno, std::generic_category(), "InetBase::getPeerAddress: getsockname");
    return Address(addr.sin_addr.s_addr, addr.sin_port);
}

/**
 * @brief     Get the MTU value of the underlying media, as it is known by the socket.
 */
int InetBase::getMtu()
{
    checkValid();
    int vmtu;
    socklen_t len = sizeof(vmtu);
    if (getsockopt(hsock_, SOL_IP, IP_MTU, &vmtu, &len) < 0)
        throw std::system_error(errno, std::generic_category(), "InetBase::getMtu: getsockopt");
    return vmtu;
}

/**
 * @brief     Bind the socket to a network interface, or unbind it.
 */
void InetBase::bindToDevice(const std::string& iface)
{
    checkValid();
    ifreq ifr;
    std::copy(iface.begin(), iface.begin() + std::min(iface.size(), sizeof(ifr.ifr_name)), ifr.ifr_name);
    if (setsockopt(hsock_, SOL_SOCKET, SO_BINDTODEVICE, reinterpret_cast<void*>(&ifr), sizeof(ifr)) < 0)
        throw std::system_error(errno, std::generic_category(), "InetBase::bindToDevice: setsockopt");
}

/** ----------------------------------------------------
 * @brief     Class DatagramSock
 * ------ */

/**
 * @brief     Server constructor. Create a socket and bind it to a local interface and local port.
 */
DatagramSock::DatagramSock (const Address& address)
    : InetBase(SOCK_DGRAM)
{
    if (bind (hsock_, address, address.size()) < 0)
    {
        terminate();
        throw std::system_error(errno, std::generic_category(), "DatagramSock: bind");
    }
}

/**
 * @brief     Get and dequeue a message from the socket queue.
 */
int DatagramSock::getMessage(void* buffer, unsigned buflen, Address* origin)
{
    return readMessage(buffer, buflen, 0, origin);
}

/**
 * @brief     Get a message without dequeuing it.
 */
int DatagramSock::peekMessage (void* buffer, unsigned buflen, Address* origin)
{
    return readMessage(buffer, buflen, MSG_PEEK, origin);
}

/**
 * @brief     Send a message to a remote endpoint.
 */
void DatagramSock::sendMessage (const void* buffer, unsigned buflen, Address* dest)
{
    if (buffer == nullptr || buflen == 0)
        throw std::invalid_argument("DatagramSock::writeMessage: 'buffer' is null or 'buflen' is 0.");
    checkValid();

    int result;
    if (dest == nullptr)
        result = sendto(hsock_, buffer, buflen, 0, nullptr, 0);
    else
        result = sendto(hsock_, buffer, buflen, 0, *dest, dest->size());

    if (result < 0)
        throw std::system_error(errno, std::generic_category(), "DatagramSock::writeMessage: sendto");
}

/**
 * @brief     Connect the socket to a remote endpoint.
 */
void DatagramSock::connect(const Address& addr)
{
    checkValid();
    if (::connect(hsock_, addr, addr.size()) < 0)
        throw std::system_error(errno, std::generic_category(), "DatagramSock::connect: connect");
}

/**
 * @brief     Read a datagram from the queue.
 */
int DatagramSock::readMessage(void* buffer, unsigned buflen, int flags, Address* origin)
{
    if (buffer == nullptr || buflen == 0)
        throw std::invalid_argument("DatagramSock::getMessage: 'buffer' is null or 'buflen' is 0.");
    checkValid();

    int msg_len;
    if (origin == nullptr)
        msg_len = recvfrom(hsock_, buffer, buflen, flags, nullptr, nullptr);
    else
    {
        socklen_t orig_size = origin->size();
        msg_len = recvfrom(hsock_, buffer, buflen, flags, *origin, &orig_size);
    }
    if (msg_len < 0)
        throw std::system_error(errno, std::generic_category(), "DatagramSock::getMessage: recvfrom");
    return msg_len;
}

/** ----------------------------------------------------
 * @brief     Class MulticastSock
 * ------ */

/**
 * @brief     Join multicast group.
 */
void MulticastSock::join(const Address& group, const std::string& iface)
{
    checkValid();
    int value = 1;
    if (setsockopt(hsock_, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int)) < 0)
        throw std::system_error(errno, std::generic_category(), "MulticastSock::join: setsockopt");
    if (bind (hsock_, group, group.size()) < 0)
        throw std::system_error(errno, std::generic_category(), "MulticastSock::join: bind");

    const sockaddr_in* ptr = group;                     // The magic is in the operator type()
    ip_mreqn req {};
    req.imr_multiaddr = ptr->sin_addr;
    if (iface.size() > 0)
    {
        ifreq ifr {};
        std::copy(iface.begin(), iface.begin() + std::min(iface.size(), sizeof(ifr.ifr_name)), ifr.ifr_name);
        if (ioctl(hsock_, SIOCGIFINDEX, &ifr) < 0)
            throw std::system_error(errno, std::generic_category(), "MulticastSock::join: ioctl(SIOCGIFINDEX)");

        req.imr_ifindex = ifr.ifr_ifindex;
        if (setsockopt(hsock_, SOL_IP, IP_MULTICAST_IF, &req, sizeof(ip_mreqn)) < 0)
            throw std::system_error(errno, std::generic_category(), "MulticastSock::join: setsockopt(IP_MULTICAST_IF)");
    }
    if (setsockopt(hsock_, SOL_IP, IP_ADD_MEMBERSHIP, &req, sizeof(ip_mreqn)) < 0)
        throw std::system_error(errno, std::generic_category(), "MulticastSock::join: setsockopt(IP_ADD_MEMBERSHIP)");
}

/**
 * @brief     Leave multicast group.
 */
void MulticastSock::leave(const Address& group)
{
    checkValid();
    const sockaddr_in* ptr = group;                     // The magic is in the operator type()
    ip_mreqn req {};
    req.imr_multiaddr = ptr->sin_addr;
    if (setsockopt(hsock_, SOL_IP, IP_DROP_MEMBERSHIP, &req, sizeof(ip_mreq)) < 0)
        throw std::system_error(errno, std::generic_category(), "MulticastSock::leave: setsockopt(IP_DROP_MEMBERSHIP)");
}

/**
 * @brief     Set TTL (Time To Live) of the outgoing multicast packets.
 * @details   TTL is the maximum number of hops a packet can travel. It is decremented by 1 for each hop,
 *            and the packet is discarded when the value drops to 0.
 * @note      This setting only affects outgoing multicast packets.
 */
void MulticastSock::setOutgoingTtl (int ttl)
{
    checkValid();
    if (setsockopt(hsock_, SOL_IP, IP_MULTICAST_TTL, &ttl, sizeof(int)) < 0)
        throw std::system_error(errno, std::generic_category(), "MulticastSock::setOutgoingTtl: setsockopt(IP_MULTICAST_TTL)");
}

/** ----------------------------------------------------
 * @brief     Class BroadcastSock
 * ------ */

/**
 * @brief     Constructor. Creates the socket and enables broadcast mode.
 */
BroadcastSock::BroadcastSock()
    : DatagramSock({INADDR_BROADCAST, 0})
{
    int value = 1;
    if (setsockopt(hsock_, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value)) < 0)
        throw std::system_error(errno, std::generic_category(), "BroadcastSock::BroadcastSock: setsockopt");
}

/**
 * @brief     Send a BroadcastSock message.
 * @throws    std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
 *            std::system_error if any error occurs while sending data.
 */
void BroadcastSock::sendMessage (const void* buffer, unsigned buflen)
{
    DatagramSock::sendMessage(buffer, buflen, nullptr);
}

/** ----------------------------------------------------
 * @brief     Class StreamBase
 * ------ */

/**
 * @brief     Configure the automatic send of keep-alive packets
 */
void StreamBase::setKeepAlive (bool mode, int idletime, int interval, int dropcount)
{
    checkValid();
    if (setsockopt(hsock_, SOL_SOCKET, SO_KEEPALIVE, &mode, sizeof(mode)) < 0)
        throw std::system_error(errno, std::generic_category(), "StreamBase::setKeepAlive: SO_KEEPALIVE");
    if (mode == true)
    {
        if (setsockopt(hsock_, SOL_TCP, TCP_KEEPIDLE, &idletime, sizeof(int)) < 0)
            throw std::system_error(errno, std::generic_category(), "StreamBase::setKeepAlive: TCP_KEEPIDLE");
        if (setsockopt(hsock_, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) < 0)
            throw std::system_error(errno, std::generic_category(), "StreamBase::setKeepAlive: TCP_KEEPINTVL");
        if (setsockopt(hsock_, SOL_TCP, TCP_KEEPCNT, &dropcount, sizeof(int)) < 0)
            throw std::system_error(errno, std::generic_category(), "StreamBase::setKeepAlive: TCP_KEEPCNT");
    }
}

/**
 * @brief     Configure the linger option.
 */
void StreamBase::setLinger (int timeout)
{
    checkValid();
    linger ling = {};                                   // Default to 0.
    if (timeout > 0)
    {
        ling.l_onoff = 1;
        ling.l_linger = timeout;
    }
    if (setsockopt(hsock_, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)) < 0)
        throw std::system_error(errno, std::generic_category(), "StreamBase::setLinger: setsockopt");
}

/**
 * @brief     Configure the 'no delay' option.
 */
void StreamBase::setNodelay (bool mode)
{
    checkValid();
    int m = mode;
    if (setsockopt(hsock_, SOL_SOCKET, TCP_NODELAY, &m, sizeof(m)) < 0)
        throw std::system_error(errno, std::generic_category(), "StreamBase::setNodelay: setsockopt");
}

/** ----------------------------------------------------
 * @brief     Class StreamClientSock
 * ------ */

/**
 * @brief   Constructor with server connection.
 * @throws  std::system_error
 */
StreamClientSock::StreamClientSock (const Address& addr)
    : StreamBase()
{
    checkValid();
    if (::connect(hsock_, addr, addr.size()) < 0)
    {
        terminate();
        throw std::system_error(errno, std::generic_category(), "StreamClientSock: connect");
    }
}

/**
 * @brief   Connect the socket to a remote server
 * @throws  std::system_error
 */
void StreamClientSock::connect (const Address& addr)
{
    checkValid();
    if (::connect(hsock_, addr, addr.size()) < 0)
        throw std::system_error(errno, std::generic_category(), "StreamClientSock::connect: connect");
}

/** ----------------------------------------------------
 * @brief     Class StreamServerSock
 * ------ */

/**
 * @brief   Constructor. Open the socket and bind it to the provided address.
 */
StreamServerSock::StreamServerSock(const Address& addr, ReuseOptions reuse)
    : StreamBase()
{
    checkValid();
    bind(addr, reuse);
}

/**
 * @brief   Bind the socket to the provided address.
 */
void StreamServerSock::bind (const Address& addr, ReuseOptions reuse)
{
    if (reuse == REUSE_ADDRESS)
        setsockopt(hsock_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (::bind (hsock_, addr, addr.size()) < 0)
        throw std::system_error(errno, std::generic_category(), "StreamServerSock::bind: bind");
}

/**
 * @brief   Configure the size of the backlog and put the socket in listen mode.
 */
void StreamServerSock::setListen (int backlog)
{
    checkValid();
    if (listen(hsock_, backlog) < 0)
        throw std::system_error(errno, std::generic_category(), "StreamServerSock::setListen: listen");
}

/**
 * @brief   Get a connection with a client.
 */
std::shared_ptr<StreamBase> StreamServerSock::getConnection (int timeout, Address* origin)
{
    if (waitData(timeout) > 0)                          // Got something
    {
        HandleSocket haccept;
        if (origin == nullptr)
            haccept.hf = accept(hsock_, nullptr, nullptr);
        else
        {
            socklen_t orig_size = origin->size();
            haccept.hf = accept(hsock_, *origin, &orig_size);
        }
        if (haccept.hf < 0)
            throw std::system_error(errno, std::generic_category(), "StreamServerSock::getConnection: accept");
        return std::shared_ptr<StreamBase>(new StreamBase(haccept));
    }
    return nullptr;
}

} } // namespaces
