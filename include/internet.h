/**
 * @package   libSocket: C++ sockets library.
 * @brief     Sockets of the INET domain (TCP/IP)
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#ifndef _LIBSOCKET_INET_INTERNET_H_
#define _LIBSOCKET_INET_INTERNET_H_

#include "socket_base.h"
#include <netinet/in.h>                                 // sockaddr_in, in_addr_t
#include <string>
#include <stdexcept>
#include <memory>                                       // std::shared_ptr

namespace libSocket { namespace inet {

/** ----------------------------------------------------
 * @brief   Class inet::Address: Internet (ipv4) addresses
 * ------ */
class Address : public libSocket::Address<sockaddr_in>
{
public:
    /**
     * @brief   Constructor with integers
     * @details 'addr' can be set to an address and to the special values INADDR_ANY and INADDR_BROADCAST.
     */
    Address (
        in_addr_t addr,                                 //!< Address in integer format. Bytes in native (host) order.
        in_port_t port                                  //!< Port in integer format. Bytes in native (host) order.
    );

    /**
     * @brief   Constructor with name resolver
     * @throws  std::system_error if the name can't be resolved
     */
    Address (
        const std::string& addr,                        //!< Address in string format.
        in_port_t port                                  //!< Port in integer format. Bytes in native (host) order.
    );
};

/** ----------------------------------------------------
 * @brief   Class InetBase: Base class of INET socket classes.
 * ------ */
class InetBase : public libSocket::SocketBase
{
public:
    /**
     * @brief   Get the local address of the socket.
     * @throws  std::system_error if the socket is invalid or an error occurs.
     */
    Address
    getAddress ();

    /**
     * @brief   Get the address of the remote endpoint.
     * @note    The socket must be connected, otherwise an exception is triggered.
     * @throws  std::system_error if the socket is invalid or not connected.
     */
    Address
    getPeerAddress ();

    /**
     * @brief   Get the MTU value of the underlying media, as it is known by the socket.
     * @note    The socket must be connected, otherwise an exception is triggered.
     * @throws  std::system_error if the socket is invalid or not connected.
     */
    int                                                 /** @return Known MTU \n 0: Error */
    getMtu ();

    /**
     * @brief   Bind the socket to a network interface, or unbind it.
     * @throws  std::system_error if the binding fails.
     * @note    If a socket is bound to an interface, only packets received from that particular interface are processed by the socket.
     * @see     libSocket::getInterfaceList to get a list of available interfaces.
     */
    void
    bindToDevice (
        const std::string& iface                        //!< Network interface name to link with, or empty to unbind the socket.
    );

protected:
    /**
     * @brief   Constructor with handler
     * @details To be used by derived classes only.
     */
    InetBase (
        HandleSocket hs
    )   : SocketBase(hs) {};

    /**
     * @brief   Constructor specifying protocol.
     * @details To be used by derived classes only.
     */
    InetBase (
        int protocol                                    //!< Internet protocol of the socket.
    )   : SocketBase(PF_INET, protocol) {};
};

/** ----------------------------------------------------
 * @brief   Class DatagramSock: Internet Datagram (UDP) sockets.
 * @details Unique class for clients and servers.
 * ------ */
class DatagramSock : public InetBase
{
public:
    /**
     * @brief   Client constructor. Create a socket with no connection.
     * @details Use this constructor to create a client socket.
     */
    DatagramSock ()
        : InetBase(SOCK_DGRAM) {};

    /**
     * @brief   Server constructor. Create a socket and bind it to a local interface and local port.
     * @throws  std::system_error if the binding fails.
     */
    DatagramSock (
        const Address& address                          //!< Network address to which the socket will connect.
    );

    /**
     * @brief   Read (dequeue) a datagram from the socket queue.
     * @details If the 'origin' pointer is provided (i.e. is not null), it will be filled with the address of the message originator.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while receiving data.
     * @note    If the input queue is empty, this function will block waiting for data.
     */
    int                                                 /** @return Message size. */
    readMessage (
        void* buffer,                                   //!< Pointer to the buffer that will contain the datagram.
        unsigned buflen,                                //!< Size of the buffer.
        Address* origin = nullptr                       //!< Pointer to the object to contain the origin address. [= nullptr]
    );

    /**
     * @brief   Get a datagram without dequeuing it.
     * @details If the 'origin' pointer is provided (i.e. is not null), it will be filled with the address of the message originator.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while receiving data.
     * @note    If the input queue is empty, this function will block waiting for data.
     */
    int                                                 /** @return Message size. */
    peekMessage (
        void* buffer,                                   //!< Pointer to the buffer that will contain the datagram.
        unsigned buflen,                                //!< Size of the buffer.
        Address* origin = nullptr                       //!< Pointer to the object to contain the origin address. [= nullptr]
    );

    /**
     * @brief   Write (enqueue) a message to send it to a remote endpoint.
     * @details If the socket is connected, the destination address is ignored and can be null.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while sending data.
     */
    void
    writeMessage (
        const void* buffer,                             //!< Pointer to the message to send.
        unsigned buflen,                                //!< Size of the message.
        Address* dest                                   //!< Destination address, or nullptr for connected sockets.
    );

    /**
     * @brief   Connect the socket to a remote endpoint.
     * @details Once connected, the socket will receive datagrams from the specified endpoint only.
     * @throws  std::system_error
     * @note    A connected socket will get a local port assigned and its MTU is calculated.
     */
    void
    connect (
        const Address& addr                             //!< Endpoint to connect to.
    );

protected:
    /**
     * @brief   Get a datagram from the queue.
     * @details Function called by other specialized functions of the class.
     * @throws  std::system_error
     */
    int                                                 /** @return Message size. */
    getMessage (
        void* buffer,                                   //!< Pointer to the buffer that will contain the datagram.
        unsigned buflen,                                //!< Size of the buffer.
        int flags = 0,                                  //!< Reading flags [= 0]. @see recvfrom
        Address* origin = nullptr                       //!< Pointer to the object to contain the origin address. [= nullptr]
    );
};

/** ----------------------------------------------------
 * @brief   Class Multicast: Multicast UDP sockets.
 * ------ */
class MulticastSock : public DatagramSock
{
public:
    /**
     * @brief   Constructor. Only default constructor allowed.
     */
    MulticastSock ()
        : DatagramSock() {};

    /**
     * @brief   Join multicast group.
     * @details The network interface parameter is optional. If set, datagrams will be sent through that interface.
     *          Otherwise, the system will decide which interface to use according to the routing rules.
     *          Once the socket joins the group, it will start receiving messages addressed to the group.
     * @throws  std::system_error
     */
    void
    join (
        const Address& group,                           //!< Address of the multicast group to subscribe to
        const std::string& iface = {}                   //!< Network interface to link [= let the system decide ]
    );

    /**
     * @brief   Leave multicast group.
     * @throws  std::system_error
     */
    void
    leave (
        const Address& group                            //!< Address of the multicast group to leave
    );

    /**
     * @brief   Set TTL (Time To Live) of the outgoing multicast packets.
     * @details TTL is the maximum number of hops a packet can travel. It is decremented by 1 for each hop,
     *          and the packet is discarded when the value drops to 0.
     * @throws  std::system_error
     * @note    This setting only affects outgoing multicast packets.
     */
    void
    setOutgoingTtl (
        int ttl                                         //!< TTL to set
    );
};

/** ----------------------------------------------------
 * @brief   Class Broadcast: Special broadcast UDP sockets
 * @note    Usually, enabling a broadcast socket requires special privileges.
 * ------ */
class BroadcastSock : public DatagramSock
{
public:
    /**
     * @brief   Constructor. Creates the socket and enables broadcast.
     * @throws  std::system_error (usually due to lack of permissions).
     */
    BroadcastSock();

    /**
     * @brief   Send a broadcast message.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while sending data.
     */
    void
    writeMessage (
        const void* buffer,                             //!< Pointer to the message to send.
        unsigned buflen                                 //!< Size of the message.
    );
};

/** ----------------------------------------------------
 * @brief   Class StreamSock: Internet Stream (TCP) sockets.
 * @details Base class for server and client stream sockets.
 * ------ */
class StreamSock : public InetBase
{
public:
    /**
     * @brief   Configure the automatic send of keep-alive packets
     * @details The system sends periodic keep-alive messages to the other end to know if it is still connected.
     *          This function allows changing the parameters of that feature.
     * @throws  std::system_error
     */
    void
    setKeepAlive (
        bool mode,                                      //!< Enable (true) or disable (false) sending keep-alive messages.
        int idletime  = kaDefaultIdleTime,              //!< Inactivity time to start sending keep-alive messages (seconds). [ 180 ]
        int interval  = kaDefaultInterval,              //!< Interval between consecutive keep-alive messages (seconds). [15]
        int dropcount = kaDefaultDropCount              //!< Number of unanswered messages at which the connection is considered lost. [9]
    );

    /**
     * @brief   Configure the 'linger' option.
     * @details When enabled, the socker destructor will not return until all queued messages for the socket have been successfully sent
     *          or the linger timeout has been reached. Otherwise, the call returns immediately and the closing is done in the background.
     *          When the socket is closed as part of exit(), it always lingers in the background.
     * @throws  std::system_error
     * @note    If a negative timeout is provided, the linger option is disabled.
     */
    void
    setLinger (
        int timeout                                     //!< Max time to wait for the socket to exit clean (seconds), or negative for disable lingering.
    );

    /**
     * @brief   Configure the 'no delay' option.
     * @details If set, disable the Nagle algorithm. This means that segments are always sent as soon as possible, even if there is only a
     *          small amount of data. When not set, data is buffered until there is a sufficient amount to send out, thereby avoiding the
     *          frequent sending of small packets, which results in poor utilization of the network.
     * @throws  std::system_error
     */
    void
    setNodelay (
        bool mode                                       //!< Enable (true) or disable (false) the no-delay mode.
    );

protected:
    enum                                                // KeepAlive default parameters
    {
        kaDefaultIdleTime  = 180,                       //!< setKeepAlive: Default inactivity timer (seconds).
        kaDefaultInterval  =  15,                       //!< setKeepAlive: Default interval between two consecutive keep-alive messages (seconds).
        kaDefaultDropCount =   9                        //!< setKeepAlive: Default max miss counter.
    };

    friend class StreamServerSock;                      //!< Allow the derived class to create new objects of this class.

    /**
     * @brief   Default constructor.
     * @details To be used by the derived classes.
     */
    StreamSock ()
        : InetBase(SOCK_STREAM) {};

    /**
     * @brief   Constructor with handler.
     * @details To be used by the derived classes when a new socket is created by the system (eg. on accept()).
     */
    StreamSock (
        HandleSocket hs                                 //!< Handler of the connected socket.
    )   : InetBase(hs) {};
};

/** ----------------------------------------------------
 * @brief   Class StreamClientSock: Internet Stream (TCP) sockets, client version
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
 * @brief   Class StreamServerSock: Internet Stream (TCP) sockets, server version
 * ------ */
class StreamServerSock : public StreamSock
{
public:
    enum ReuseOptions                                   /** @brief  Options for reusing addresses. @see StreamServerSock, bind */
    {
        DONT_REUSE_ADDRESS = 0,                         //!< Don't reuse the requested address (actually, port).
        REUSE_ADDRESS      = 1                          //!< Reuse the requested port if it is in TIME_WAIT state.
    };

    /**
     * @brief   Constructor. Open the socket and bind it to the provided address.
     * @throws  std::system_error
     * @see     bind
     */
    StreamServerSock (
        const Address& addr,                            //!< Address and port to bind the socket to.
        ReuseOptions reuse = DONT_REUSE_ADDRESS         //!< Specify if the port must tried to be reused.
    );

    /**
     * @brief   Bind the socket to the provided address.
     * @details Use the address INADDR_ANY to bind the port to all network interfaces.
     *          The option REUSE_ADDRESS allows binding to a port that is in TIME_WAIT state.
     *          The TCP stack keeps the ports in TIME_WAIT state for a while after shutting the socket down, just in case a spurious packet
     *          addressed to that socket is received. During this time, the port can't be reused unless the REUSE_ADDRESS option is set.
     *          Default is to not reuse addresses.
     *          If the port is in listening status (i.e. bound by another socket), the binding will fail anyway.
     * @throws  std::system_error
     */
    void
    bind (
        const Address& addr,                            //!< Address and port to bind the socket to.
        ReuseOptions reuse = DONT_REUSE_ADDRESS         //!< Specify if the port must tried to be reused.
    );

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
    std::shared_ptr<StreamSock>                         /** @return New socket connected to the remote endpoint. */
    getConnection (
        int timeout = WAIT_DATA_FOREVER,                //!< Max time waiting for a connection, DONT_WAIT or WAIT_DATA_FOREVER [ = wait forever ]
        Address* origin = nullptr                       //!< Address of the connection originator, or nullptr for discarding it [ = discard ]
    );

protected:
    enum
    {
        DEFAULT_MAX_BACKLOG         = 32                //!< Default backlog size
    };
};

} } // namespaces

#endif // _LIBSOCKET_INET_INTERNET_H_
