#include <libSocket/libSocket.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

static constexpr const char sock_name[] = "/tmp/test_unix";

char prompt(const char* menu = 0, const char* options = 0)
{
    while (1)
    {
        if (menu)
            std::cout << menu << ": ";
        else
            std::cout << "Press Enter to continue...";
        int key = getchar();
        if (!options)
            return key;
        for (const char* tmp = options; *tmp; tmp++)
            if (*tmp == key)
                return key;
        std::cout << "Option '" << key << "' is not valid. Try again.\n";
    }
}

void server()
{
    try
    {
        libSocket::unx::StreamServerSock srv({sock_name});
        std::cout << "Socket created -- go to listen\n";
        srv.setListen();
        prompt();
        auto got = srv.getConnection(10000);
        if (got.has_value())
        {
            std::cout << "Got connection\n";
            while (got->waitData(500) > 0)
            {
                std::cout << "Data received\n";
                std::this_thread::sleep_for(1s);
                uint8_t buffer[512];
                auto lesen = got->read(buffer, sizeof(buffer));
                if (lesen > 0)
                    std::cout << "Got " << lesen << " bytes: '" << buffer << "'\n";
                else
                {
                    std::cout << "Nothing to read.\n";
                    break;
                }
            }
            std::cout << "Timeout waiting for data.\n";
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
        libSocket::unx::StreamClientSock cli({sock_name});
        std::cout << "about to write something\n";
        int sent = cli.write("something to send");
        std::cout << "Sent " << sent << " bytes.\n";
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
