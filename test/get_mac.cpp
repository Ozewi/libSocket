/* pruebas para obtener la MAC a partir de un socket normal */

#include <libSocket/libSocket.h>
#include <stdio.h>

void print_mac(const char* str, std::basic_string<uint8_t> mac)
{
  printf("%s: ", str);
  printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int main()
{
    libSocket::packet::PacketSock sock("eth5", 0xFF00);
    print_mac("socket raw", sock.getMac());

    print_mac("getMac func", libSocket::getMac("eth5"));
}
