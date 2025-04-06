/**
 * @package   libSocket: C++ sockets library.
 * @brief     Base class of all socket classes.
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the BSD 3-Clause License and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#ifndef _LIBSOCKET_SOCKET_BASE_H_
#define _LIBSOCKET_SOCKET_BASE_H_

#include <sys/poll.h>                                   // POLLIN, POLLOUT
#include <sys/socket.h>                                 // SO_SNDBUF, SO_RCVBUF
#include <vector>                                       // waitevent(...vector<SockBase>)
#include <string>
#include <functional>                                   // std::reference_wrapper
#include <system_error>                                 // std::system_error, std::invalid_argument

// Useful macros for throwing exceptions with source info
#define QUOTE(x) MACRO_TO_STRING(x)
#define MACRO_TO_STRING(x) #x
#define CONTEXTUALIZE(txt) std::string() + __FILE__ ":" QUOTE( __LINE__ )  " (" + __FUNCTION__ + ") - " txt

#define THROW_SYSTEM_ERROR(txt) throw std::system_error(errno, std::generic_category(), CONTEXTUALIZE(txt))
#define THROW_INVALID_ARGUMENT(txt) throw std::invalid_argument(CONTEXTUALIZE(txt))

namespace libSocket {

/** ----------------------------------------------------
 * @brief   Class template Address: Socket addresses.
 * ------ */
template <typename SockClass>
class Address
{
public:
    /**
     * @brief   Default constructor.
     */
    Address() : addr_() {};

    /**
     * @brief   Get the size of the BSD socket address struct.
     */
    size_t
    size () const
        {
            return sizeof(SockClass);
        };

    /**
     * @brief   Get the object as sockaddr*
     */
    operator sockaddr* ()
        {
            return reinterpret_cast<sockaddr*>(&addr_);
        };

    /**
     * @brief   Get the object as const sockaddr*
     */
    operator const sockaddr* () const
        {
            return reinterpret_cast<const sockaddr*>(&addr_);
        };

    /**
     * @brief   Get the object as pointer to its base type
     */
    operator SockClass* ()
        {
            return reinterpret_cast<SockClass*>(&addr_);
        };

    /**
     * @brief   Get the object as const pointer to its base type
     */
    operator const SockClass* () const
        {
            return reinterpret_cast<const SockClass*>(&addr_);
        };

    /**
     * @brief   Get the object as pointer to its base type
     */
    SockClass*
    get()
        {
            return reinterpret_cast<SockClass*>(&addr_);
        };

protected:
    SockClass addr_;                                    //!< Socket address in BSD format
};

/** ----------------------------------------------------
 * @brief   Class SocketBase: Base class of socket classes.
 * @details Generic socket functions.
 * ------ */
class SocketBase
{
public:
    enum ReadTimeouts                                   /** @brief Special timeouts waiting for data. @see read, waitData */
    {
        WAIT_DATA_FOREVER = -1,                         //!< Wait data forever.
        DONT_WAIT = 0                                   //!< Don't wait for data.
    };

    enum WaitModes                                      /** @brief Wait modes.  @see waitEvent */
    {
        WAIT_READ = POLLIN,                             //!< Wait for data to read.
        WAIT_WRITE = POLLOUT,                           //!< Wait for room in the write queue.
        WAIT_READWRITE = (POLLIN | POLLOUT)             //!< Wait for any of the two above.
    };

    enum WriteModes                                     /** @brief Write modes. @see write */
    {
        WRITE_WAIT_QUEUED = 1,                          //!< Blocking write. Function blocks until the buffer is in the output queue.
        WRITE_DONT_WAIT   = 0                           //!< Non-blocking write.
    };

    enum IoModes                                        /** @brief I/O modes. @see setIoMode */
    {
        IOMODE_BLOCK = 1,                               //!< Set input/output to blocking mode
        IOMODE_NONBLOCK = 0                             //!< Set input/output to non-blocking mode
    };

    enum BufferTypes                                    /** @brief Buffer types. @see getBufferLength, setBufferLength */
    {
        SEND_BUFFER = SO_SNDBUF,                        //!< Sending buffer
        RECEIVE_BUFFER = SO_RCVBUF                      //!< Receiving buffer
    };

    /**
     * @brief   Default constructor.
     */
    SocketBase ()
        : hsock_(INVALID_HANDLER), inode_(INVALID_INODE) {};

    /**
     * @brief   Move constructor
     */
    SocketBase (
        SocketBase&& other                              //!< The socket to move from
    );

    /**
     * @brief   Copy constructor: deleted
     * @note    Sockets are closed on destruction, so making copies is risky.
     */
    SocketBase (SocketBase&) = delete;

    /**
     * @brief   Socket destructor.
     * @note    Socket is shutdown and closed.
     */
    ~SocketBase ();

    /**
     * @brief   Read data from the socket.
     * @details If timeout == DONT_WAIT, the function reads any data pending in the input buffer and returns immediately.
     *          If timeout == WAIT_DATA_FOREVER, the function waits until all the requested data is received.
     *          Otherwise, data is read until the buffer is full or time runs out, whichever comes first.
     * @throws  std::invalid_argument, std::system_error
     */
    int                                                 /** @return Bytes read. \n 0: Nothing to read. */
    read (
        void* buffer,                                   //!< Pointer to the read buffer
        int max_bytes,                                  //!< Number of bytes to read
        int timeout = DONT_WAIT                         //!< Reading timeout (in milliseconds). Also @see ReadTimeouts for other values. [ = DONT_WAIT ]
    );

    /**
     * @brief   Write data to the socket.
     * @details If there is no room in the system queue for the entire buffer, the function either waits for space (WRITE_WAIT_QUEUED) or
     *          writes only enough to fill the buffer (WRITE_DONT_WAIT). In both cases, the return value is the number of bytes written.
     * @throws  std::invalid_argument, std::system_error
     */
    int                                                 /** @return Number of bytes sent. */
    write (
        const void* buffer,                             //!< Write buffer
        int buf_len,                                    //!< Length of the write buffer
        WriteModes write_mode = WRITE_DONT_WAIT         //!< Waiting mode [ = don't wait ] - @see WriteModes
    );

    /**
     * @brief   Write data to the socket.
     * @details Simplified version using std::string. @see write
     * @throws  std::invalid_argument, std::system_error
     */
    int                                                 /** @return Number of bytes sent. */
    write (
        const std::string& buffer,                      //!< Write buffer
        WriteModes write_mode = WRITE_DONT_WAIT         //!< Waiting mode [ = don't wait ] - @see WriteModes
    )   {
            return write(buffer.c_str(), buffer.size() + 1, write_mode);
        };

    /**
     * @brief   Close the socket.
     * @details A common pattern of network servers is to listen for incoming connections and fork a child to attend them.
     *          In these patterns, the child process must call close() on the listening socket so that the destructor does not shut it down.
     */
    void
    close ();

    /**
     * @brief   Get the number of pending bytes in the read queue.
     * @details On DGRAM sockets, the size is that of the datagram waiting (if any). On STREAM sockets, the value may vary.
     * @throws  std::invalid_argument if the socket is in listening mode (i.e. server socket waiting for connections).
     */
    int                                                 /** @return Pending bytes in the read queue. */
    pending ();

    /**
     * @brief   Wait for incoming data.
     * @details If timeout == DONT_WAIT, the function returns immediately, returning 0 if no data is pending and >0 otherwise.
     *          If timeout == WAIT_DATA_FOREVER, the function waits until there's data in the input queue.
     *          Otherwise, the function waits until there's any data pending or time runs out, whichever comes first.
     * @throws  std::system_error if the handler is invalid or an error happens while waiting.
     */
    int                                                 /** @return Remaining time. \n 0: Timeout without receiving data. */
    waitData (
        int timeout                                     //!< Max time waiting for data (milliseconds). Also @see ReadTimeouts for other values.
    );

    /**
     * @brief   Get the length of the i/o buffers.
     * @throws  std::system_error
     */
    int                                                 /** @return Buffer size, in bytes. */
    getBufferLength (
        BufferTypes buf_type                            //!< What buffer to read. @see BufferTypes.
    );

    /**
     * @brief   Set the length of the socket buffers
     * @throws  std::system_error
     */
    void
    setBufferLength (
        BufferTypes buf_type,                           //!< What buffer to read. @see BufferTypes.
        int buf_len                                     //!< New buffer size, in bytes.
    );

    /**
     * @brief   Set general i/o blocking mode
     * @details Read operations get blocked when the input queue is empty.
     * @throws  std::system_error
     */
    void
    setIomode (
        IoModes ioMode                                  //!< Read/write mode to set
    );

    /**
     * @brief   Wait for an event on any of the sockets provided.
     * @note    If multiple sockets get an event at time, only the first one is returned.
     */
    friend
    SocketBase*                                         /** @return Pointer to the socket that received the event \n nullptr: Timeout waiting. */
    waitEvent (
        WaitModes event_type,                           //!< Event that is expected.
        unsigned  timeout,                              //!< Max waiting time (milliseconds)
        const std::vector<std::reference_wrapper<SocketBase>>& socklist  //!< List of pointers to the sockets to wait for
    );

protected:
    struct HandleSocket { int hf; };                    //!< Rename the socket handler type to do overloading by handle type.

    enum
    {
        INVALID_HANDLER = -1,                           //!< Value of invalid socket handler
        INVALID_INODE = 0                               //!< Value of invalid inode identifier
    };

    /**
     * @brief   Constructor with handler.
     * @note    To be used only by derived classes when a new socket is created (f.i. by accept())
     */
    SocketBase (
        HandleSocket hs                                 //!< Socket handler
    )   : hsock_(hs.hf), inode_(getInode()) {};

    /**
     * @brief   Constructor with family and protocol.
     * @note    To be used only by derived classes.
     */
    SocketBase (
        int family,                                     //!< Socket family: PF_INET, PF_UNIX, ...
        int type,                                       //!< Socket type: SOCK_STREAM, SOCK_DGRAM, ...
        int protocol = 0                                //!< Socket protocol (usually 0)
    );

    /**
     * @brief   Close and shutdown the socket.
     */
    void
    terminate ();

    /**
     * @brief   Get the inode of the socket.
     */
    ino_t                                               /** @return inode of the socket. \n 0: Error or invalid socket. */
    getInode () const;

    /**
     * @brief   Invalidate the socket handler.
     * @note    isValid will return false after calling this function.
     * @note    The socket won't be shut down in the destructor.
     */
    void
    invalidate ();

    int    hsock_;                                      //!< Socket handler
    ino_t  inode_;                                      //!< Socket inode
};

} // namespace

#endif  // _LIBSOCKET_SOCKET_BASE_H_
