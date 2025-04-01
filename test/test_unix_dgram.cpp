/**
 * @brief       Pruebas de uso de sockets Unix datagram
 */
#include <libSocket/libSocket.h>
#include <iostream>
#include <stdio.h>

enum
{
  BufferSize = 512,
  ReadTimeout = 5000
};
const char sock_name[] = "/tmp/test_ux_dgram";
const uint8_t srv_reply[] = "Recibido, gracias";

void server()
{
    try
    {
        libSocket::unx::DatagramSock srv({sock_name});
        std::cout << "Server socket created\n";
        uint8_t buffer[BufferSize];

        while (1)
        {
            if (srv.waitData(ReadTimeout) == 0)
                std::cout << "Timeout waiting for data\n";
            else
            {
                libSocket::unx::Address origin;
                auto lesen = srv.readMessage(buffer, BufferSize - 1, origin);
                if (lesen >= (BufferSize - 1))
                    std::cout << "Overflow!\n";
                else
                {
                    buffer[lesen] = 0;
                    std::cout << "Received " << lesen << " bytes. Content: " << buffer << std::endl;
                    srv.writeMessage(srv_reply, sizeof(srv_reply), origin);
                }
            }
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << "Something was wrong ...\n" << e.what() << std::endl;
    }
}

void client()
{
    try
    {
        libSocket::unx::DatagramSock cli;
        cli.connect({sock_name});
        std::cout << "Client connected\n";

        uint8_t buffer[BufferSize];
        char fmt[10];
        sprintf(fmt, "%%%ds", BufferSize - 1);

        while (1)
        {
            std::cout << "Write your message: ";
            scanf(fmt, &buffer);
            std::cout << "Sending: '" << buffer << "'\n";

            cli.writeMessage(buffer, strlen(reinterpret_cast<char*>(buffer)));
            if (cli.waitData(ReadTimeout) == 0)
                std::cerr << "Timeout waiting for data\n";
            if (cli.readMessage(buffer, BufferSize - 1) == 0)
                std::cerr << "Nothing to read\n";
            std::cout << "Response received: '" << buffer << "'\n";
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Something was wrong ...\n" << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        std::cout << "Please specify if I must act as (c)lient or as (s)erver\n";
    else if (argv[1][0] == 's')
        server();
    else if (argv[1][0] == 'c')
        client();
    else
        std::cout << "Didn't understood, please try again.\n";
    return 0;
}
