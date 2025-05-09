/**
 * @package   libSocket: C++ sockets library.
 * @brief     Base class of all socket classes.
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the BSD 3-Clause License and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#include "socket_base.h"
#include <unistd.h>                     // close
#include <sys/ioctl.h>                  // ioctl
#include <sys/socket.h>                 // send, recv
#include <fcntl.h>                      // fcntl
#include <sys/stat.h>                   // fstat
#include <cerrno>                       // errno ...
#include <system_error>                 // std::system_error, std::invalid_argument

namespace libSocket {

/** ----------------------------------------------------
 * @brief     Class SocketBase: Public functions
 * ------ */

/**
 * @brief     Move constructor
 */
SocketBase::SocketBase (SocketBase&& other)
{
    hsock_ = other.hsock_;
    inode_ = other.inode_;
    other.invalidate();
}

/**
 * @brief     Destructor
 */
SocketBase::~SocketBase ()
{
    terminate();
}

/**
 * @brief     Close the socket.
 */
void SocketBase::close()
{
    inode_ = 0;
    if (hsock_ != INVALID_HANDLER)
    {
        ::close(hsock_);
        hsock_ = INVALID_HANDLER;
    }
}

/**
 * @brief     Get the number of bytes waiting in the input queue.
 */
int SocketBase::pending()
{
    int count = 0;
    if (ioctl(hsock_, FIONREAD, &count) < 0)
        THROW_INVALID_ARGUMENT("The socket is in an invalid state");
    if (count == 0)
        /* ---This is a hack---
         * On datagram sockets, this ioctl retrieves the payload length of the next datagram in the queue.
         * However, this length can be 0, and there's no way to know if the queue is empty or if there's a
         * datagram waiting with 0 bytes of payload. Worse, the ioctl will keep returning 0 until the empty
         * datagram is removed, even if there are more datagrams pending.
         * Hence this function call. It removes the empty datagram if present and does nothing otherwise.
         */
        recvfrom(hsock_, 0, 0, MSG_DONTWAIT, 0, 0);
    return count;
}

/**
 * @brief     Wait for incoming data.
 */
int SocketBase::waitData(int timeout)
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");

    fd_set socklist;
    FD_ZERO(&socklist);
    FD_SET(hsock_, &socklist);
    timeval tm = { .tv_sec = timeout / 1000, .tv_usec = (timeout - (tm.tv_sec * 1000)) * 1000 };
    timeval* tm_ptr = (timeout == WAIT_DATA_FOREVER)? nullptr : &tm;
    int rt = 0;

    do
        rt = select(hsock_+1, &socklist, 0, 0, tm_ptr); // Linux-specific: 'select' updates the timeval struct
    while (rt < 0 && errno == EINTR);                   // If interrupted by a signal, continue
    if (rt < 0)
        THROW_SYSTEM_ERROR("select()");
    if (rt > 0 && tm_ptr != nullptr)                    // Calculate remaining time
        rt = ((tm.tv_sec * 1000) + (tm.tv_usec / 1000));
    return rt;
}

/**
 * @brief     Get the length of the i/o buffers.
 */
int SocketBase::getBufferLength (BufferTypes bufType)
{
    if (bufType != SEND_BUFFER && bufType != RECEIVE_BUFFER)
        THROW_INVALID_ARGUMENT("'bufType' is invalid.");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");

    int retval;
    socklen_t size = sizeof(retval);
    if (getsockopt(hsock_, SOL_SOCKET, bufType, reinterpret_cast<void*>(&retval), &size) != 0)
        THROW_SYSTEM_ERROR("getsockopt()");
    return retval;
}

/**
 * @brief     Establecer el tamaño del buffer del socket
 */
void SocketBase::setBufferLength (BufferTypes bufType, int buflen)
{
    if (bufType != SEND_BUFFER && bufType != RECEIVE_BUFFER)
        THROW_INVALID_ARGUMENT("setBufferLength: 'buftype' is invalid.");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    if (setsockopt(hsock_, SOL_SOCKET, bufType, reinterpret_cast<void*>(&buflen), sizeof(buflen)) < 0)
        THROW_SYSTEM_ERROR("setBufferLength: error in setsockopt()");
}

/**
 * @brief     Set general i/o blocking mode
 */
void SocketBase::setIomode(IoModes iomode)
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    int flags = fcntl(hsock_, F_GETFL);
    if (iomode == IOMODE_BLOCK)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    fcntl(hsock_, F_SETFL, flags);
}

/**
 * @brief     Constructor with family and protocol.
 */
SocketBase::SocketBase (int family, int type, int protocol)
    : hsock_(socket(family, type, protocol)), inode_(getInode())
{
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
}

/**
 * @brief     Read data from the socket
 */
int SocketBase::read(void* buffer, int bytes, int timeout)
{
    if (buffer == nullptr || bytes <= 0)
        THROW_INVALID_ARGUMENT("'buffer' is null or 'bytes' is 0");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");;

    int lesen = 0;

    if (timeout == DONT_WAIT)                           // don't wait: read as much as you can and exit
        lesen = recv(hsock_, buffer, bytes, MSG_NOSIGNAL | MSG_DONTWAIT);
    else if (timeout == WAIT_DATA_FOREVER)              // wait until all data is received
        lesen = recv(hsock_, buffer, bytes, MSG_NOSIGNAL | MSG_WAITALL);
    else                                                // Regular timeout
    {
        fd_set socklist;
        timeval tm = { .tv_sec = timeout / 1000, .tv_usec = (timeout - (tm.tv_sec * 1000)) * 1000 };

        while (lesen < bytes && tm.tv_sec + tm.tv_usec > 0)
        {
            if (!pending())                             // If there's no pending data, let's wait
            {
                FD_ZERO(&socklist);
                FD_SET(hsock_, &socklist);
                auto result = select(hsock_+1, &socklist, 0, 0, &tm);   // Linux-specific: 'select' updates the timeval struct.
                if (result == 0)                        // Timeout -- terminate the loop.
                    break;
                if (result < 0)                         // Error
                {
                    if (errno == EINTR)                 // Interrupted by a signal: continue.
                        continue;
                    THROW_SYSTEM_ERROR("select()");
                }
            }
            int err = recv(hsock_, reinterpret_cast<char*>(buffer) + lesen, bytes - lesen, MSG_NOSIGNAL);
            if (err < 0)
                THROW_SYSTEM_ERROR("recv()");
            if (err == 0)                               // EOF -- socket was probably closed on the other end
                break;
            lesen += err;
        }
    }
    return lesen;
}

/**
 * @brief     Write data to the socket.
 */
int SocketBase::write(const void* buffer, int bytes, WriteModes writeMode)
{
    if (buffer == nullptr || bytes <= 0)
        THROW_INVALID_ARGUMENT("'buffer' is null or 'bytes' is 0");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");;

    int flags = MSG_NOSIGNAL;
    if (writeMode == WRITE_DONT_WAIT)
        flags |= MSG_DONTWAIT;
    auto bytes_sent = send(hsock_, buffer, bytes, flags);
    if (bytes_sent < 0)                                 // negative result indicates an error
        THROW_SYSTEM_ERROR("send()");
    return bytes_sent;
}

/**
 * @brief     Get a datagram from the queue.
 */
int SocketBase::readDatagram(void* buffer, unsigned buflen, int flags, sockaddr* origin, socklen_t orig_size)
{
    if (buffer == nullptr || buflen == 0)
        THROW_INVALID_ARGUMENT("'buffer' is null or 'buflen' is 0.");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");

    int msg_len = recvfrom(hsock_, buffer, buflen, flags, origin, &orig_size);
    if (msg_len < 0)
        THROW_SYSTEM_ERROR("recvfrom()");
    return msg_len;
}

/**
 * @brief     Write (enqueue) a datagram to send it to a remote endpoint.
 */
void SocketBase::writeDatagram(const void* buffer, unsigned buflen, const sockaddr* dest, int dest_size)
{
    if (buffer == nullptr || buflen == 0)
        THROW_INVALID_ARGUMENT("'buffer' is null or 'buflen' is 0.");
    if (hsock_ == INVALID_HANDLER || inode_ == INVALID_INODE)
        THROW_SYSTEM_ERROR("Invalid socket handler");
    if (sendto(hsock_, buffer, buflen, 0, dest, dest_size) < 0)
        THROW_SYSTEM_ERROR("sendto()");
}

/**
 * @brief     Close and shutdown the socket.
 */
void SocketBase::terminate()
{
    inode_ = 0;
    if (hsock_ != INVALID_HANDLER)
    {
        shutdown(hsock_, SHUT_RDWR);
        ::close(hsock_);
        hsock_ = INVALID_HANDLER;
    }
}

/**
 * @brief     Get the inode of the socket.
 */
ino_t SocketBase::getInode() const
{
    struct stat st;
    if (hsock_ != INVALID_HANDLER && fstat(hsock_, &st) >= 0)
        return st.st_ino;
    return 0;
}

/**
 * @brief     Invalidate the socket handler
 */
void SocketBase::invalidate ()
{
    hsock_ = INVALID_HANDLER;
    inode_ = 0;
}

/**
 * @brief     Wait for an event on any of the sockets provided.
 */
SocketBase* waitEvent(SocketBase::WaitModes eventType, unsigned timeout, const std::vector<std::reference_wrapper<SocketBase>>& socklist)
{
    if (socklist.empty() || eventType <= 0)
        return nullptr;

    int count = socklist.size();
    pollfd p_list[count];
    pollfd* ptr = p_list;
    for (auto& sock : socklist)
    {
        ptr->fd = sock.get().hsock_;
        ptr->events = eventType;
        ptr++;
    }

    int err;
    do
        err = poll(p_list, count, timeout);
    while (err < 0 && errno == EINTR);

    if (err > 0)
        for (int idx = 0; idx < count; idx++)
            if (p_list[idx].revents & eventType)        // Got the event
                return &socklist[idx].get();

    return nullptr;
}

} // namespace
