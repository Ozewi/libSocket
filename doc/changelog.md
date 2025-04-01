# libSocket library changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)

## [2.0] - 2025.04.01

Major revamp of the library:

- Updated to modern C++.
  - Functions renamed to camelCase.
  - C++ version updated to C++17.
  - Extensive use of exceptions.
- Better code documentation.
  - Descriptions, comments, and error texts translated into english.
  - Fixed doxygen errors.
  - Doxygen-generated docs included in the repo.
- Restructuring.
  - Classes grouped in files by socket domain.
  - Socket addresses encapsulated on their own classes.

## [1.4.0] - 2023.12.05

### Changed
- Functions read and write now get a 'void *' instead of 'uint8_t *' to avoid the library user to do a type cast.

## [1.3.3] - 2023.11.30

### Changed
- Removed the dependency of libUtility.
- Version identifier is now obtained through libSocket::version().

## [1.3.2] - 2019.10.16

### Changed
- Constant SockBase::DEF_MAX_PENDING is now public again.

## [1.3.1] - 2019.07.12

### Changed
- SockBase: Now the function 'readall' returns 0 when the other endpoint closes, instead of -1.

## [1.3.0] - 2019.06.29

### Added
- SockInetStreamCli:
  - New function 'connect' to reconnect the socket if it was not possible to connect in the constructor.
  - New constructor with no parameters available; it is initialized but not connected.
- SockInetStreamSrv: New function 'bind' to bind the socket if this step failed in the constructor.

## [1.2.3] - 2017.12.19

### Changed
- Function set_keepalive: Parameter 'mode' is now an enumerated type.

### Added
- The functionality of SockUnixStreamSrv::get_request is now split into two new functions:
SockUnixStreamSrv::set_listen and SockUnixStreamSrv::get_connection.

### Deprecated
- Función SockUnixStreamSrv::get_request is now deprecated.

## [1.2.2] - 2016.04.01

### Changed
- Version identifier moved to an separate file 'version.cpp'.

## [1.2.1] - 2016.03.09

### Changed
- Changes due to the interface update of the version management module.

## [1.2.0] - 2014.04.20

### Changed
- Code structure reordered. Include directory moved to the root of the library.
- Doxygen comments moved to the headers.
- Macros changed to enums.
- Single header libSocket.h split in several headers, one per socket type.

## [1.1.0] - 2011.09.07

### Added
- The functionality of SockInetStreamSrv::get_request is now split into two new functions:
SockInetStreamSrv::set_listen and SockInetStreamSrv::get_connection.

### Deprecated
- SockInetStreamSrv::get_request is now deprecated.

## [1.0.5] - 2011.08.22

### Added
- Added the function SockInetDgram::allow_broadcast to obtain and release permissions for sending broadcast packets.
SockInetDgram constructors no longer set this permission; it has to be done manually using this function.

### Changed
- Linking to the address INADDR_BROADCAST for sending broadcasta packets is no longer needed.

## [1.0.4] - 2011.08.13

### Added
- Function SockInetStream::set_nodelay added to enable or disable the Nagle algorithm.

## [1.0.3] - 2011.08.09

### Changed
- Removed the dependency of the class template Container.

  The waitevent method, which was the only method using it, now uses the template std::vector.

## [1.0.2] - 2009.02.20

### Fixed
- Fixed an error in SockInetDgram when creating a socket broadcast.
- Fixed the return value of the function set_iomode.

### Added
- Added the function bind_to_device.

## [1.0.1] - 2007.11.15

### Changed
- In function pending(), added a non-blocking reading call of 0 bytes when there are no pending bytes. This solves the problem
of the spurious UDP packets with 0-bytes payload.
- Function wait_data() now returns with value -1 if the socket is invalid.

## [1.0.0] - 2006.10.18

**Release 1.0**

### Changed
- Documentation updated.
- Class names changed to reflect its real content, keeping the old names as typedef for backwards compatibility.

### Added
- New function SockPacketRaw::peek_pkt for reading packets without getting them out of the queue.
- New function SockInetDgram::peek_msg for reading messages without getting them out of the queue.

## [0.10.2] - 2006.05.05

### Added
- Nueva función compare_addr_in.

## [0.10.1] - 2006.02.22

### Changed
- packet_raw: La estructura ethpkt deriva ahora de ether_header, definida en glibc, en lugar de ethhdr definida en las cabeceras de linux.

## [0.10.0] - 2006.02.02

### Added
- Nueva clase unix_dgram para gestión de sockets UNIX Datagram.
- Nueva función create_unix_addr para crear objetos de clase sockaddr_un.

### Changed
- Renombrados los archivos de cada tipo de socket para indicar familia y tipo de conexión: inet_dgram, inet_stream, unix_dgram, unix_stream, packet_raw.

## [0.9.23] - 2005.10.19

### Changed
- sock_dgram: read_msg: recvfrom puede devolver un valor negativo en caso de error.

## [0.9.22] - 2005.10.14

### Fixed
- Correcciones generales (supuestamente inocuas) sugeridas por valgrind.

## [0.9.21] - 2005.07.04

### Changed
- sock_dgram: SO_REUSEADDR en mcast_join, para permitir más de una conexión multicast al mismo grupo.

## [0.9.20] - 2005.06.13

### Changed
- sock_dgram: mcast_join ahora admite como parámetro el nombre del interfaz, en lugar de la dirección (que no estaba funcionando).

### Added
- Nueva función mcast_set_ttl.

## [0.9.19] - 2005.04.22

### Fixed
- sock_base: Corregido un error en readall con lecturas de gran tamaño.
- sock_dgram: write_msg admite un puntero nulo como dirección de destino, para enviar mensajes a la dirección por defecto en sockets conectados.
- sock_dgram: connect ahora devuelve el valor correcto.

## [0.9.18] - 2005.04.18

### Fixed
- functions: Corregido un error en waitevent.

## [0.9.17] - 2005.04.05

### Fixed
- sock_base: Los métodos get_localport y get_peerport ahora devuelven los valores en orden de red, como decían hacer.

## [0.9.16] - 2005.04.04

### Added
- Añadida la cabecera sys/stat.h en sock_base.

## [0.9.15] - 2005.03.30

### Added
- sock_dgram: Añadido soporte de multicast mediante las funciones mcast_join y mcast_leave.

## [0.9.14] - 2005.03.29

### Added
- Añadido constructor con estructura sockaddr_in en stream_srv y sock_dgram.
- Nueva función connect(sockaddr_in*).

## [0.9.13] - 2005.03.28

### Changed
- Cambiados varios parámetros de tipo bool a entero, con constantes indicativas de lo que hacen.

## [0.9.12] - 2005.03.22

### Changed
- JCSeijo - Se asigna el inodo en el constructor de los socket ethernet.

## [0.9.11] - 2005.03.15

### Changed
- wait_data vuelve a entrar (conservando el timeout) si se recibe un error EINTR.
- waitevent vuelve a entrar si se recibe un error EINTR.

## [0.9.10] - 2005.03.14

### Changed
- Obtención y comprobación del inodo asociado al socket. Así se soslaya un problema debido a la reutilización de descriptores de archivo del sistema.

## [0.9.9] - 2005.03.11

### Changed
- Eliminadas de la cabecera todas las definiciones de función, que pasan al cuerpo de sus .cpp respectivos.
- isValid ahora comprueba la validez del descriptor.

## [0.9.8] - 2005.03.09

### Changed
- waitevent ahora hace caso del parámetro event.

## [0.9.7] - 2005.03.08

### Changed
- Eliminada la invalidación de socket.

### Added
- La función stream_base::set_keepalive permite establecer parámetros de keep alive.
  NOTA: Non-portable Linux extension.
- Nuevo fichero functions.cpp, con las funciones generales extraídas de sock_base.
- Nuevo fichero options_base.cpp, con las funciones de manejo de opciones extraídas de sock_base.

## [0.9.6] - 2005.03.07

### Fixed
- La comprobación de error de retorno de algunas funciones provoca invalidación del socket.

### Changed
- sock_base::wait_data devuelve -1 en caso de error.

## [0.9.5] - 2005.03.02

### Fixed
- Corregido un error en el constructor de stream_srv: el parámetro reuseaddr no era efectivo.

## [0.9.4] - 2005.03.01

### Added
- Añadida la función de código de revisión de la librería.

## [0.9.3] - 2005.02.25

### Fixed
- Corregido un error en waitevent.

## [0.9.2] - 2005.01.26

### Changed
- Cambiadas algunas cabeceras <linux/*> por su equivalente de <ioctl/*>

## [0.9.1] - 2005.01.20

### Fixed
- create_addr: gethostbyname no admite parámetros nulos.

## [0.9.0] - 2005.01.19

### Changed
- Cambiado el uso de sys/types.h por stdint.h
- Cambio de ABI.

## [0.8.13] - 2005.01.18

### Added
- Nueva función create_net_addr para valores en orden de red
- Nuevo constructor de stream_cli usando la estructura sockaddr_in.

## [0.8.12] - 2005.01.13

### Fixed
- create_addr: El puerto 0 sí es válido para una dirección.

## [0.8.11] - 2005.01.11

### Added
- sock_base: Añadida la función invalidate() y los operadores = y ==.

## [0.8.10] - 2005.01.10

### Added
- Doxygenado del código.
- Añadida la función sock_unix::create_pair para crear sockets unix conectados.
- sock_base: Añadido nuevo constructor de copia.

### Fixed
- Correcciones en las funciones waitevent.

## [0.8.9] - 2005.01.04

### Changed
- sock_base: Las funciones waitevent pueden esperar también excepciones en los sockets (como un cierre del socket).

## [0.8.8] - 2004.12.03

### Fixed
- create_addr: La dirección 0 sí es válida (INADDR_ANY).

## [0.8.7] - 2004.11.25

### Changed
- sock_base: La función create_addr requiere ahora todos los parámetros con los bytes en orden de host (antes sólo estaba así el parámetro port).

## [0.8.6] - 2004.11.22

### Fixed
- sock_base: Corregido un error en el valor de retorno de wait_data.

## [0.8.5] - 2004.11.16

### Added
- sock_base: Añadidas las funciones get_peeraddr y get_peerport.

### Changed
- La función wait_data ahora devuelve el tiempo restante de timeout.

## [0.8.4] - 2004.11.02

### Changed
- stream_srv: La función get_request acepta un parámetro opcional más, que es rellenado con la dirección de origen de la conexión.

## [0.8.3] - 2004.10.25

### Added
- Añadida la función friend waitevent con una lista (container) de sockets.

## [0.8.2] - 2004.10.19

### Changed
- Eliminado el constructor de sock_ether de la cabecera (no estaba en el cuerpo).

## [0.8.1] - 2004.09.29

### Changed
- Añadida la función sock_ether::get_localaddr.

## [0.8.0] - 2004.09.27

### Added
- Implementación de sockets Ethernet RAW mediante la clase sock_ether.

## [0.7.3] - 2004.09.24

### Changed
- El parámetro stream_srv::stream_srv(reuseaddr) ahora toma valor por omisión.

## [0.7.2] - 2004.09.22

### Fixed
- Corregido un error en la función sock_base::get_mac

## [0.7.1] - 2004.09.07

### Added
- Implementación de sockets UDP Broadcast.

## [0.7.0] - 2004.08.10

### Changed
- Cambios de interfaz.
  - La función sock_base::iomode pasa a ser sock_base::set_iomode, con nuevos parámetros: IOCTL_BLOCK, IOCTL_NONBLOCK.
  - La función sock_stream::set_bufsize pasa a ser sock_base::set_bufsize, con nuevos parámetros: SO_SNDBUF, SO_RCVBUF.
  - La función sock_stream::keep_alive pasa a ser sock_stream::set_keepalive
  - La función sock_dgram::getmsg pasa a ser sock_dgram::read_msg
  - La función sock_dgram::putmsg pasa a ser sock_dgram::write_msg
  - La función sock_dgram::mtu pasa a ser sock_base::get_mtu
  - Nueva función sock_base::get_bufsize.
  - Nueva función sock_base::get_localaddr.
  - Nueva función sock_base::get_localport.
  - Nueva función sock_dgram::connect.
  - Cambiados los tipos u_int32_t por in_addr_t y u_int16_t por in_port_t donde resultaba adecuado.

## [0.6.12] - 2004.08.03

### Changed
- Reunidas todas las cabeceras en un solo fichero.

## [0.6.11] - 2004.07.22

### Added
- Añadido el método close para cerrar el socket sin hacer shutdown.

## [0.6.10] - 2004.05.07

### Added
- Añadido el método get_mac para obtener la dirección MAC de un interfaz.
- Añadido el método readall para lectura de buffers de tamaño fijo con timeout.

### Fixed
- Corregido un error en el método sock_dgram::getmsg (¡gracias Juan Carlos!)

## [0.6.9] - 2004.04.28

### Fixed
- Corregidos algunos solapamientos de variable local.
- Makefile actualizado.

## [0.6.8] - 2004.04.26

### Added
- Constante WAIT_DATA_FOREVER para esperar indefinidamente.

### Changed
- wait_data ahora acepta un entero con signo.

## [0.6.7] - 2004.04.19

### Changed
- Cambiado el prototipado de funciones read y write de (char *) a (u_int8_t*)

## [0.6.6] - 2004.01.29

### Added
- Añadido el flag MSG_NOSIGNAL en las funciones que lo soportan, para evitar la necesidad de rutar la señal SIGPIPE.

## [0.6.5] - 2003.05.20

### Changed
- Implementada la función waitevent para esperar múltiples objetos.
- El método pending pasa de sock_dgram a sock_base.

### Added
- Compilación para el coldfire.

## [0.6.4] - 2003.05.14

### Changed
- Rediseño general:
  - Constructores con parámetros.  Los sockets se conectan al construirse y se desconectan al destruirse.
  - Eliminación de los métodos de conexión y desconexión.
  - Los métodos de espera de conexión devuelven objetos de la clase base.

### Added
- Añadido soporte de sockets dgram.
- Primera versión de la documentación general de uso (incompleta).

## [0.6.3] - 2003.05.06

### Fixed
- Depuración de sockets unix.

### Changed
- El tipo devuelto por open es ahora un entero.

## [0.6.2] - 2003.04.30

### Added
- Clase base general.
- Añadido soporte de sockets unix.

## [0.6.1] - 2003.04.29

### Added
- Incorporado y adaptado el código de SocketMgr.

## [0.6.0] - 2003.04.24

### Changed
- Iniciado el branch libSocket con un nuevo diseño general.
- SocketMgr queda como código obsoleto y no soportado.

## [0.5.1] - 2002.07.08

### Changed
- El método Read añade un \0 al final del bloque leído para facilitar su tratamiento como string.

## [0.5.0] - 2002.03.27

### Changed
- "Modernización".
  - División del código en varios archivos.
  - Adaptación para usar con Win32 (MinGW) mediante WinSock2.
  - Tipo de socket y protocolo ahora se pueden escoger (en el constructor).

### Added
- Creación de Shared Object para Linux

## [0.4.4] - 2001.03.01

### Added
- Añadidos los métodos SetBufSize y GetBufSize para cambiar el tamaño de los buffers del sistema.
- Añadido el método SetLinger para establecer parámetros de "limpieza".

## [0.4.3] - 2001.02.16

### Changed
- Modificados todos los constructores para eliminar de una vez el absurdo parámetro de construcción.
- Modificados los métodos de apertura para añadir el puerto TCP de conexión.

## [0.4.2] - 2001.02.07

### Added
- Primera documentación de la biblioteca (sólo revisiones).
- Compilación como biblioteca estática para Linux/ELF.

### Fixed
- Corregido un error en srvSocket al obtener la IP por nombre de host.

## [0.4.1] - 2000.12.12

### Changed
- Cambios cosméticos varios. Añadida documentación del código.

## [0.4.0] - 2000.12.01

### Added
- Añadido gethostbyname en la apertura de socket, para localizar el host por nombre (no solo por IP).

### Removed
- Eliminado el buffer de lectura.  Read ahora requiere un buffer externo.

### Changed
- Write admite buffers de bytes (no sólo de caracteres).

## [0.3.1] - 1999.03.26

### Added
- Incluída opción para seleccionar la subred de la que se aceptan mensajes (srvSocket).
- Incluída opción para especificar el tipo de cierre del socket aceptado.

## [0.3.0] - 1999.03.25

### Added
- Primera versión unificada de sockets INET cliente y servidor.
- Separación (por fin) del módulo de sockets respecto de los programas originales.
