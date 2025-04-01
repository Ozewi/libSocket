/**
 * @brief       Pruebas de uso de sockets Unix datagram probando los tiempos de espera
 */
#include <libSocket/libSocket.h>
#include <iostream>
#include <time.h>
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

enum
{
    BufferSize = 512,
    ReadTimeout = 5000,
    TICS_PER_SEC = 1000000
};
const char sock_name[] = "/tmp/test_ux_dgram";
const uint8_t srv_reply[] = "Recibido, gracias";

uint64_t GetTimeNow()
{
    timespec now;
    clock_gettime(CLOCK_BOOTTIME, &now);
    uint64_t bignow = now.tv_sec;                         // tv_sec is of type time_t, which is an integer (32 or 64 bits depending upon the system architecture)
    bignow *= TICS_PER_SEC;
    bignow += (now.tv_nsec / (1000000000 / TICS_PER_SEC));
    return bignow;
}

void log(uint64_t lapse)
{
    timespec now;
    clock_gettime(CLOCK_BOOTTIME, &now);
    printf("[%06ld.%06ld] tiempo: %ld\n", now.tv_sec, now.tv_nsec / 1000, lapse);
}

void server()
{
    try
    {
        libSocket::unx::DatagramSock srv({sock_name});
        std::cout << "Server socket created\n";

        uint64_t packet;
        uint64_t clock;
        libSocket::unx::Address origin;

        while (1)
        {
            if (srv.waitData(libSocket::SocketBase::WAIT_DATA_FOREVER) == 0)
                std::cout << "timeout waiting for data\n";
            else
            {
                auto lesen = srv.readMessage(&packet, sizeof(packet), origin);
                if (lesen != sizeof(packet))
                    std::cout << "Error receiving data: expected " << sizeof(packet) << ", received " << lesen << std::endl;
                else
                {
                    clock = GetTimeNow();
                    log(clock - packet);
                    packet = clock;
                    srv.writeMessage(&packet, sizeof(packet), origin);
                    clock = GetTimeNow();
                    log(clock - packet);
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}

void client()
{
    try
    {
        libSocket::unx::DatagramSock cli;
        cli.connect({sock_name});
        std::cout << "Connected\n";

        uint64_t packet;
        uint64_t clock;

        while (1)
        {
            packet = GetTimeNow();
            cli.writeMessage(&packet, sizeof(packet));
            clock = GetTimeNow();
            log(clock - packet);

            if (cli.waitData(ReadTimeout) == 0)
                std::cout << "Timeout waiting for data\n";
            else
            {
                int lesen = cli.readMessage(&packet, sizeof(packet));
                if (lesen <= 0)
                    std::cout << "No data received\n";
                else
                {
                    clock = GetTimeNow();
                    log(clock - packet);
                }
            }
            std::this_thread::sleep_for(1s);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
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
