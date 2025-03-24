/**
 * @package   libSocket: C++ sockets library.
 * @brief     Sockets of the INET domain (TCP/IP)
 * @author    José Luis Sánchez Arroyo
 * @section   License
 * Copyright (c) 1998-2025 José Luis Sánchez Arroyo
 * This software is distributed under the terms of the LGPL version 2.1 and comes WITHOUT ANY WARRANTY.
 * Please read the file LICENSE for further details.
 */

#ifndef _SOCKET_INTERNET_H_
#define _SOCKET_INTERNET_H_

#include "socket_base.h"
#include <netinet/in.h>                                 // sockaddr_in, in_addr_t
#include <string>
#include <stdexcept>

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
     * @brief   Constructor specifying protocol.
     * @details To be used by the derived classes.
     */
    InetBase (
        int protocol                                    //!< Internet protocol of the socket.
    )   : SocketBase(PF_INET, protocol) {};
};

/** ----------------------------------------------------
 * @brief   Class Datagram: Internet Datagram (UDP) sockets.
 * @details Unique class for clients and servers.
 * ------ */
class Datagram : public InetBase
{
public:
    /**
     * @brief   Client constructor. Create a socket with no connection.
     * @details Use this constructor to create a client socket.
     */
    Datagram ()
        : InetBase(SOCK_DGRAM) {};

    /**
     * @brief   Server constructor. Create a socket and bind it to a local interface and local port.
     * @throws  std::system_error if the binding fails.
     */
    Datagram (
        const Address& address                          //!< Network address to which the socket will connect.
    );

    /**
     * @brief   Get and dequeue a datagram from the socket queue.
     * @details If the 'origin' pointer is provided (i.e. is not null), it will be filled with the address of the message originator.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while receiving data.
     * @note    If the input queue is empty, this function will block waiting for data.
     */
    int                                                 /** @return Message size. */
    getMessage (
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
     * @brief   Send a message to a remote endpoint.
     * @details If the socket is connected, the destination address is ignored and can be null.
     * @throws  std::invalid_argument if the 'buffer' pointer is null or 'buflen' is 0.
     *          std::system_error if any error occurs while sending data.
     */
    void
    sendMessage (
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

    /**
     * @brief   Configure the socket to allow sending broadcast datagrams.
     * @details This function enables the socket to broadcast datagrams. To do it, use the address INADDR_BROADCAST when sending
     *          out the datagrams. @see inet::Address::Address.
     * @throws  std::system_error.
     * @note    Usually, enabling a socket for broadcasting requires special privileges.
     */
    void
    allowBroadcast (
        bool mode                                       //!< New broadcast-enable mode: on (true) or off (false).
    );
protected:
    /**
     * @brief   Read a datagram from the queue.
     * @details Function called by other specialized functions of the class.
     */
    int                                                 /** @return Message size. */
    readMessage (
        void* buffer,                                   //!< Pointer to the buffer that will contain the datagram.
        unsigned buflen,                                //!< Size of the buffer.
        int flags = 0,                                  //!< Reading flags [= 0]. @see recvfrom
        Address* origin = nullptr                       //!< Pointer to the object to contain the origin address. [= nullptr]
    );
};

/** ----------------------------------------------------
 * @brief   Clase SockInetDgram: Sockets de tipo Datagram del dominio Internet, basados en UDP.
 * @details Clase única para el manejo de "clientes" y "servidores".
 * ------ */
class Multicast : public Datagram
{
public:
    /**
     * @brief   Conectar el socket a un grupo de multicast
     * @note    Si no se especifica interfaz, el sistema escoge el que le parezca mejor.
     * @note    Sólo funciona en sockets que no estuvieran previamente enlazados (sockets cliente).
     */
    bool                                                  /** @return true si todo fue bien, false si error. */
    multicastJoin (
        sockaddr_in* addr,                                  /** @param   Dirección del grupo de multicast al que conectar. */
        const char* iface = 0                               /** @param   Interfaz de red que se conecta. */
    );

    /**
     * @brief   Desconectar el socket de un grupo de multicast
     * @note    Si la conexión está activa a través de varios interfaces, se desconecta el primero de ellos.
     */
    bool                                                  /** @return true si todo fue bien, false si error. */
    multicastLeave (
        sockaddr_in* addr                                   /** @param addr Dirección del grupo de multicast. */
    );

    /**
     * @brief   Establecer TTL de los paquetes multicast del socket
     */
    bool                                                  /** @return true si todo fue bien, false si error. */
    multicastSetTtl (
        int ttl                                             /** @param ttl Nuevo TTL a establecer */
    );


};

#if 0

/** ----------------------------------------------------
 * @brief   Clase SockInetStream: Sockets de tipo Stream del dominio Internet, basados en TCP.
 * @details Clase base para sockets de tipo cliente y servidor
 * ------ */
class Stream : public Inet
{
public:
    /**
     * @brief Función set_keepalive, parámetro status
     */
    enum KeepAliveModes
    {
        KEEPALIVE_OFF = 0,                                  //!< Desactivar el envío de mensajes "keep alive"
        KEEPALIVE_ON = 1                                    //!< Activar el envío de mensajes "keep alive"
    };

    /**
     * @brief   Apertura genérica de un socket stream.
     * @note    Normalmente no se utiliza explícitamente este constructor.
     */
    Stream (
        int32_t hs = INVALID_HANDLER                         /** @param hs Descriptor del socket (opcional) */
    );

    /**
     * @brief   Programar el envío de mensajes "keep alive"
     */
    bool                                                  /** @return true: el cambio se completó con éxito | false: error */
    setKeepAlive (
        KeepAliveModes mode,                                   /** @param mode      Activar o desactivar el envío de mensajes "keep alive" */
        int32_t idletime  = kaDefaultIdleTime,              /** @param idletime  Tiempo (en segundos) de inactividad del socket antes de empezar a enviar mensajes [180] */
        int32_t interval  = kaDefaultInterval,              /** @param interval  Tiempo (en segundos) entre mensajes de "keep alive" [15] */
        int32_t dropcount = kaDefaultDropCount              /** @param dropcount Número de mensajes no respondidos para considerar que la conexión se ha perdido [9] */
    );

    /**
     * @brief   Establecer el modo y timeout de "linger"
     * @note    Si timeout es menor que 0, se desactiva el linger.
     */
    bool                                                  /** @return true: el cambio se completó con éxito | false: error */
    setLinger (
        int32_t timeout                                     /** @param timeout Tiempo máximo de espera de linger */
    );

    /**
     * @brief   Habilitar o deshabilitar el modo de envío de datos inmediato
     * @note    Cuando se activa el modo de envío inmediato, los datos son enviados al extremo sin esperar
     *          a llenar un segmento de datos. Debe activarse para aplicaciones de tiempo real.
     * @note    Por omisión, el envío inmediato está deshabilitado y se usa el algoritmo de Nagle.
     */
    bool                                                  /** @return true: el cambio se completó con éxito | false: error */
    setNodelay (                                         /** @param mode true: activa el envío inmediato | false: desactiva el envío inmediato */
        bool mode
    );

protected:
    static constexpr int kaDefaultIdleTime  = 180;        //!< set_keepalive: Tiempo de inactividad por omisión
    static constexpr int kaDefaultInterval  =  15;        //!< set_keepalive: Intervalo entre mensajes de keepalive por omisión
    static constexpr int kaDefaultDropCount =   9;        //!< set_keepalive: Número por omisión de mensajes no respondidos
};

/**-------------------------------------------------------------------------------------------------
 * @brief   Clase SockInetStreamCli: Sockets cliente de tipo Stream del dominio Internet
 * ------ */
class StreamClient : public Stream
{
public:
    /**
     * @brief   Apertura genérica de un socket stream.
     * @desc    Se crea el socket pero no se conecta.
     */
    StreamClient ()
        : Stream(INVALID_HANDLER)
    {};

    /**
     * @brief   Constructor con conexión al servidor
     */
    StreamClient (
        const char* target,                                 /** @param target  Nombre (DNS) o dirección (numbers-and-dots) del servidor con el que conectar */
        in_port_t port                                      /** @param port    Puerto del servidor al que se conecta (en orden de host) */
    );

    /**
     * @brief   Constructor con conexión al servidor
     * @note    Dirección especificada en forma de entero.
     */
    StreamClient (
        in_addr_t target,                                   /** @param target  Dirección del servidor con el que conectar (en orden de host) */
        in_port_t port                                      /** @param port    Puerto del servidor al que se conecta (en orden de host) */
    );

    /**
     * @brief   Constructor con conexión al servidor
     * @note    Dirección especificada en forma de estructura de dirección
     */
    StreamClient (
        sockaddr_in* addr                                   /** @param addr  Puntero a la estructura con la dirección del servidor (en formato de red) */
    );

    /**
     * @brief   Conexión a un servidor remoto
     * @note    Dirección especificada en forma de entero.
     */
    bool                                                  /** @return true: Conexión correcta | false: Destino no alcanzado o error general (véase errno) */
    connect (
        sockaddr_in* addr                                   /** @param addr  Puntero a la estructura con la dirección del servidor (en formato de red) */
    );
};

/**-------------------------------------------------------------------------------------------------
 * @brief   Clase SockInetStreamSrv: Sockets servidor de tipo Stream del dominio Internet
 * ------ */
class StreamServer : public Stream
{
public:
    /**
     * @brief   Función stream_srv, parámetro reuseMode
     */
    enum EnReuseMode
    {
        DONT_REUSE_ADDR = 0,                                //!< No intentar reutilizar la dirección (puerto)
        REUSE_ADDR      = 1                                 //!< Reutilizar la dirección (puerto) si es posible
    };

    /**
     * @brief   Enlazar el socket con un interfaz local
     * @note    Si el primer parámetro es INADDR_ANY, el socket se enlaza a todos los interfaces.
     * @note    Por omisión, reusemode vale DONT_REUSE_ADDR.
     */
    StreamServer (
        in_addr_t iface,                                    /** @param iface      Dirección del interfaz con el que se enlaza (en orden de host) (INADDR_ANY = enlaza a todos los interfaces) */
        in_port_t port,                                     /** @param port       Puerto TCP (en orden de host) */
        EnReuseMode reuseMode = DONT_REUSE_ADDR             /** @param reuseMode  Modalidad de reutilización de puerto: REUSE_ADDR, DONT_REUSE_ADDR [no reutilizar]*/
    );

    /**
     * @brief   Enlazar el socket con un interfaz local
     * @note    Por omisión, reusemode vale DONT_REUSE_ADDR.
     */
    StreamServer (
        sockaddr_in* addr,                                  /** @param addr       Estructura del interfaz de enlace (en orden de red) */
        EnReuseMode reuseMode = DONT_REUSE_ADDR             /** @param reuseMode  Modalidad de reutilización de puerto: REUSE_ADDR, DONT_REUSE_ADDR [no reutilizar]*/
    );

    /**
     * @brief   Enlazar el socket con un interfaz local
     */
    bool                                                  /** @return true: Enlace correcto | false: no se pudo enlazar (permisos, dirección en uso...) (véase errno) */
    bind (
        sockaddr_in* addr,                                  /** @param addr      Estructura del interfaz de enlace (en orden de red) */
        EnReuseMode reuseMode                               /** @param reuseMode Modalidad de reutilización de puerto: REUSE_ADDR, DONT_REUSE_ADDR [no reutilizar]*/
    );

    /**
     * @brief   Detener la ejecución del programa hasta recibir una petición de conexión.
     * @note    Llamada bloqueante. El proceso queda bloqueado en espera de una conexión.
     * @note    Especifica el espacio reservado para conexiones sin atender; por omisión DEF_MAX_PENDING (20).
     * @note    Si 'origin' no es nulo, se rellena con la dirección de origen de la petición.
     * @note    Función obsoleta: utilizar set_listen y get_connection en su lugar.
     */
    Stream*                                       /** @return  Socket creado para atender la petición. */
    getRequest (
        int32_t backlog = DEF_MAX_PENDING,                  /** @param backlog  Máximo permitido de conexiones pendientes. [valor por defecto]*/
        sockaddr_in* origin = nullptr                       /** @param origin   Dirección de origen de la petición. [descartar dirección de origen]*/
    );

    /**
     * @brief   Poner el socket a escuchar y establecer el backlog de conexiones pendientes
     */
    bool                                                  /** @return true: operación correcta | false: error */
    setListen (
        int32_t backlog = DEF_MAX_PENDING                   /** @param backlog Máximo permitido de peticiones de conexión pendientes. [valor por defecto]*/
    );

    /**
     * @brief   Obtener una conexión con un cliente
     * @note    Si timeout es 0 y no hay conexiones pendientes, se vuelve inmediatamente.
     * @note    Si timeout es WAIT_DATA_FOREVER, se espera hasta que haya una conexión.
     * @note    @obsoleta
     */
    Stream*                       /** @return  Socket creado para atender la petición. */
    getConnection (
        int32_t timeout = WAIT_DATA_FOREVER,                /** @param timeout Tiempo máximo de espera para una conexión [espera indefinidamente]*/
        sockaddr_in* origin = nullptr                       /** @param origin  Dirección de origen de la conexión [descartar dirección de origen]*/
    );
};

#endif // if 0

} } // namespaces

#endif // _SOCKET_INTERNET_H_
