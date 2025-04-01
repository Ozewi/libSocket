/* Un pequeño cliente sntp. Pregunta la hora al servidor especificado y vuelca los datos. */

#include <libSocket/libSocket.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <arpa/inet.h>

int error(const std::string& reason)
{
    std::cerr << reason << std::endl;
    return -1;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return error("Please specify the address of the NTP server.");

    uint8_t message[60];
    message[0] = 013;                                       // octal: version 1, mode 3
    auto addr = libSocket::inet::Address({argv[1], 123});   // 123 is UDP port for NTP
    std::cout << "ip: " << inet_ntoa(addr.get()->sin_addr) << std::endl;
    libSocket::inet::DatagramSock sock;
    sock.writeMessage(message, sizeof(message), addr);
    if (sock.waitData(3500) == 0)
        return error ("Timeout waiting for the server.");
    if (sock.pending() == 0)
        return error ("No data received.");
    if (sock.readMessage(message, sizeof(message)) < 48)
        return error ("Received message too short.");

    std::cout << "Message received. Data:\n";
    for (int ix = 0; ix < 48; ix++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(message[ix]) << " ";
        if (ix % 4 == 3)
            std::cout << std::endl;
    }
    time_t time = ntohl(reinterpret_cast<uint32_t*>(message)[10]);
    std::cout << "Time: " << std::hex << std::setw(8) << std::setfill('0') << time << std::endl;
    time -= 0x83aa7e80;
    std::cout << asctime(gmtime(&time)) << std::endl;
}
