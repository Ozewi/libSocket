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
#include <sys/un.h>                                     // sockaddr_un
#include <memory>                                       // std::shared_ptr
#include <string>
#include <optional>                                     //std::optional

namespace libSocket { namespace unx {

/** ----------------------------------------------------
 * @brief   Class unix::Address: Unix addresses
 * ------ */
class Address : public libSocket::Address<sockaddr_un>
{
public:
    /**
     * @brief   Default constructor.
     */
    Address() : libSocket::Address<sockaddr_un>() {};

    /**
     * @brief   Constructor with name
     * @details If the socket name begins with a slash '/', then a 'filesystem' address is created.
     *          Otherwise, an address of the 'abstract socket namespace' is created. (Linux extension).
     */
    Address (
        const std::string& name                         //!< Socket name
    );
};

/** ----------------------------------------------------
 * @brief   Class UnixBase: Base class of UNIX socket classes.
 * ------ */
class UnixBase : public libSocket::SocketBase
{
protected:
    /**
     * @brief   Constructor with handler
     * @details To be used by derived classes only.
     */
    UnixBase (
        HandleSocket hs
    )   : SocketBase(hs) {};

    /**
     * @brief   Constructor specifying protocol.
     * @details To be used by derived classes only.
     */
    UnixBase (
        int protocol                                    //!< Unix protocol of the socket.
    )   : SocketBase(PF_UNIX, protocol) {};
};

/** ----------------------------------------------------
 * @brief   Class DatagramSock: Unix Datagram sockets.
 * @details Unique class for clients and servers.
 * ------ */
class DatagramSock : public UnixBase
{
public:
    /**
     * @brief   Client constructor. Create a socket with no connection.
     * @details Use this constructor to create a client socket.
     */
    DatagramSock ();

    /**
     * @brief   Server constructor. Create a socket and bind it to a name.
     * @throws  std::system_error if the binding fails.
     */
    DatagramSock (
        const Address& address                          //!< Socket name to which the socket will be bound.
    );

    /**
     * @brief   Destructor. Removes the socket from the filesystem, if exists.
     */
    ~DatagramSock();

    /**
     * @brief   Create a socket connected to this socket.
     * @details A pair of connected sockets is created. This object gets one of them, and a new object is created with the other.
     * @note    If there was a valid socket descriptor, it is shutdown and a new one is assigned.
     */
    std::shared_ptr<DatagramSock>                       /** @return Socket connected to this socket. */
    createPair ();

    /**
     * @brief   Read (dequeue) a datagram from the socket queue.
     * @details If the 'origin' argument is provided, it will be filled with the address of the message originator.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while receiving data.
     * @note    If the input queue is empty, this function will block waiting for data.
     */
    int                                                 /** @return Message size. */
    readMessage (
        void* buffer,                                   //!< Pointer to the buffer that will contain the datagram.
        unsigned buflen,                                //!< Size of the buffer.
        std::optional<std::reference_wrapper<Address>> origin = std::nullopt    //!< Pointer to the object to contain the origin address. [= none]
    );

    /**
     * @brief   Send a message to a listening socket
     * @details If the socket is connected, the destination address is ignored and can be absent.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while sending data.
     */
    void
    writeMessage (
        const void* buffer,                             //!< Pointer to the message to send.
        unsigned buflen,                                //!< Size of the message.
        std::optional<std::reference_wrapper<Address>> dest = std::nullopt      //!< Destination address. Ignored (may be absent) on connected sockets.
    );

    /**
     * @brief   Connect the socket to a listening socket.
     * @details Once connected, the socket will receive datagrams from the specified endpoint only.
     * @throws  std::system_error
     */
    void
    connect (
        const Address& addr                             //!< Endpoint to connect to.
    );

protected:
    /**
     * @brief   Constructor with handle.
     * @details Used internally.
     */
    DatagramSock (
        HandleSocket hs
    )   : UnixBase(hs) {};
};

/** ----------------------------------------------------
 * @brief   Class StreamSock: Unix stream sockets.
 * @details Base class for server and client stream sockets.
 * ------ */
class StreamSock : public UnixBase
{
public:
    /**
     * @brief   Create a socket connected to this socket.
     * @details A pair of connected sockets is created. This object gets one of them, and a new object is created with the other.
     * @note    If there was a valid socket descriptor, it is shutdown and a new one is assigned.
     */
    std::shared_ptr<StreamSock>                         /** @return Socket connected to this socket. */
    createPair ();

protected:

    friend class StreamServerSock;                      //!< Allow the derived class to create new objects of this class.

    /**
     * @brief   Default constructor.
     * @details To be used by the derived classes.
     */
    StreamSock ()
        : UnixBase(SOCK_STREAM) {};

    /**
     * @brief   Constructor with handler.
     * @details To be used by the derived classes when a new socket is created by the system (eg. on accept()).
     */
    StreamSock (
        HandleSocket hs                                 //!< Handler of the connected socket.
    )   : UnixBase(hs) {};
};

/** ----------------------------------------------------
 * @brief   Class StreamClientSock: Unix stream sockets, client version.
 * ------ */
class StreamClientSock : public StreamSock
{
public:
    /**
     * @brief   Default constructor.
     * @details The socket is created but it is not connected.
     * @see     connect
     */
    StreamClientSock ()
        : StreamSock() {};

    /**
     * @brief   Constructor with server connection.
     * @throws  std::system_error
     */
    StreamClientSock (
        const Address& addr                             //!< Network address to which the socket will connect.
    );

    /**
     * @brief   Connect the socket to a remote server
     * @throws  std::system_error
     */
    void
    connect (
        const Address& addr                             //!< Network address to which the socket will connect.
    );
};

/** ----------------------------------------------------
 * @brief   Class StreamServerSock: Unix stream sockets, server version.
 * ------ */
class StreamServerSock : public StreamSock
{
public:
    /**
     * @brief   Constructor. Open the socket and bind it to the provided address.
     * @throws  std::system_error
     * @see     bind
     */
    StreamServerSock (
        const Address& addr                             //!< Socket name
    );

    /**
     * @brief   Destructor. Removes the socket from the filesystem, if exists.
     */
    ~StreamServerSock();

    /**
     * @brief   Configure the size of the backlog and put the socket in listen mode.
     * @details The backlog argument defines the maximum length to which the queue of pending connections may grow. If a connection request
     *          arrives when the queue is full, the client may receive an error or the request may be ignored.
     * @throws  std::system_error
     */
    void
    setListen (
        int backlog = DEFAULT_MAX_BACKLOG               //!< Maximum length of the pending connections queue [ = DEFAULT_MAX_BACKLOG ]
    );

    /**
     * @brief   Get a connection with a client.
     * @details If timeout is DONT_WAIT and there are no pending connections, the function returns immediately.
     *          If timeout is WAIT_DATA_FOREVER, it waits forever for an incoming connection.
     * @throws  std::system_error
     */
    std::optional<StreamSock>                           /** @return Socket connected to the remote endpoint, or none. */
    getConnection (
        int timeout = WAIT_DATA_FOREVER,                //!< Max time waiting for a connection, DONT_WAIT or WAIT_DATA_FOREVER [ = wait forever ]
        std::optional<std::reference_wrapper<Address>> origin = std::nullopt    //!< Pointer to the object to contain the origin address. [= none]
    );

protected:
    enum
    {
        DEFAULT_MAX_BACKLOG         = 32                //!< Default backlog size
    };
};

} } // namespaces

#endif // _LIBSOCKET_UNX_UNIX_H_
