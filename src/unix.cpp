/**
 * @package   libSocket: C++ sockets library.
 * @brief     Sockets of the UNIX domain
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#include "unix.h"
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <system_error>

namespace libSocket { namespace unx {

/** ----------------------------------------------------
 * @brief     Class unx::Address
 * ------ */

/**
 * @brief   Constructor with name
 * @details If the socket name begins with a slash '/', then a 'filesystem' address is created.
 *          Otherwise, an address of the 'abstract socket namespace' is created. (Linux extension).
 */
Address::Address (const std::string& name)
{
    addr_.sun_family = PF_UNIX;
    if (name[0] == '/')                                 // 'filesystem' namespace
        std::copy(name.begin(), name.begin() + std::min(name.size(), sizeof(addr_.sun_path)), addr_.sun_path);
    else                                                // Abstract sockets namespace
        std::copy(name.begin() + 1, name.begin() + std::min(name.size() - 1, sizeof(addr_.sun_path)), addr_.sun_path);
}

/** ----------------------------------------------------
 * @brief     Class DatagramSock
 * ------ */

/**
 * @brief     Server constructor. Create a socket and bind it to a name.
 */
DatagramSock::DatagramSock (const Address& address)
    : UnixBase(SOCK_DGRAM)
{
    if (bind (hsock_, address, address.size()) < 0)
    {
        terminate();
        THROW_SYSTEM_ERROR("bind()");
    }
}

/**
 * @brief     Read (dequeue) a datagram from the socket queue.
 */
std::shared_ptr<DatagramSock> DatagramSock::createPair()
{
    int pair[2];
    if (socketpair(PF_UNIX, SOCK_DGRAM, 0, pair) == 0)
    {
        terminate();                                    // Shutdown any previous socket
        hsock_ = pair[0];
        inode_ = getInode();
        return std::shared_ptr<DatagramSock>(new DatagramSock({pair[1]}));
    }
    return nullptr;
}

/**
 * @brief     Read a datagram from the queue.
 */
int DatagramSock::readMessage(void* buffer, unsigned buflen, Address* origin)
{
    if (buffer == nullptr || buflen == 0)
        THROW_INVALID_ARGUMENT("'buffer' is null or 'buflen' is 0.");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");

    int msg_len;
    if (origin == nullptr)
        msg_len = recvfrom(hsock_, buffer, buflen, 0, nullptr, nullptr);
    else
    {
        socklen_t orig_size = origin->size();
        msg_len = recvfrom(hsock_, buffer, buflen, 0, *origin, &orig_size);
    }
    if (msg_len < 0)
        THROW_SYSTEM_ERROR("recvfrom()");
    return msg_len;
}

/**
 * @brief     Write (enqueue) a message to send it to a remote endpoint.
 */
void DatagramSock::writeMessage (const void* buffer, unsigned buflen, Address* dest)
{
    if (buffer == nullptr || buflen == 0)
        THROW_INVALID_ARGUMENT("'buffer' is null or 'buflen' is 0.");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");

    int result;
    if (dest == nullptr)
        result = sendto(hsock_, buffer, buflen, 0, nullptr, 0);
    else
        result = sendto(hsock_, buffer, buflen, 0, *dest, dest->size());

    if (result < 0)
        THROW_SYSTEM_ERROR("sendto()");
}

/**
 * @brief     Connect the socket to a remote endpoint.
 */
void DatagramSock::connect(const Address& addr)
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    if (::connect(hsock_, addr, addr.size()) < 0)
        THROW_SYSTEM_ERROR("connect()");
}

/** ----------------------------------------------------
 * @brief     Class StreamSock
 * ------ */

/**
 * @brief     Create a socket connected to this socket.
 */
std::shared_ptr<StreamSock> StreamSock::createPair ()
{
    int pair[2];
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, pair) == 0)
    {
        terminate();                                    // Shutdown any previous socket
        hsock_ = pair[0];
        inode_ = getInode();
        return std::shared_ptr<StreamSock>(new StreamSock({pair[1]}));
    }
    return nullptr;
}

/** ----------------------------------------------------
 * @brief     Class StreamClientSock
 * ------ */

/**
 * @brief   Constructor with server connection.
 * @throws  std::system_error
 */
StreamClientSock::StreamClientSock (const Address& addr)
    : StreamSock()
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    if (::connect(hsock_, addr, addr.size()) < 0)
    {
        terminate();
        THROW_SYSTEM_ERROR("connect()");
    }
}

/**
 * @brief   Connect the socket to a remote server
 * @throws  std::system_error
 */
void StreamClientSock::connect (const Address& addr)
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    if (::connect(hsock_, addr, addr.size()) < 0)
        THROW_SYSTEM_ERROR("connect()");
}

/** ----------------------------------------------------
 * @brief     Class StreamServerSock
 * ------ */

/**
 * @brief     Constructor. Open the socket and bind it to the provided address.
 */
StreamServerSock::StreamServerSock(const Address& addr)
    : StreamSock()
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    if (bind(hsock_, addr, addr.size()) < 0)
    {
        terminate();
        THROW_SYSTEM_ERROR("bind()");
    }
}

/**
 * @brief     Destructor. Removes the socket from the filesystem, if exists.
 */
StreamServerSock::~StreamServerSock()
{
    if (hsock_ != INVALID_HANDLER)
    {
        sockaddr_un srv;
        socklen_t len = sizeof(sockaddr_un);
        int rt = getsockname(hsock_, reinterpret_cast<sockaddr*>(&srv), &len);
        terminate();
        if (rt == 0 && len > 2 && srv.sun_path[0])
          unlink(srv.sun_path);
    }
}

/**
 * @brief     Configure the size of the backlog and put the socket in listen mode.
 */
void StreamServerSock::setListen (int backlog)
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    if (listen(hsock_, backlog) < 0)
        THROW_SYSTEM_ERROR("listen()");
}

/**
 * @brief     Get a connection with a client.
 */
std::shared_ptr<StreamSock> StreamServerSock::getConnection (int timeout, Address* origin)
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
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
            THROW_SYSTEM_ERROR("accept()");
        return std::shared_ptr<StreamSock>(new StreamSock(haccept));
    }
    return nullptr;
}


} } // namespaces
