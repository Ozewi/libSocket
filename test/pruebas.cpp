#include <libSocket/libSocket.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <time.h>           // clock_gettime

char prompt(const char* menu = 0, const char* options = 0)
{
    while (1)
    {
        if (menu)
            std::cout << menu << ": ";
        else
            printf("Press Enter to continue...");
        int key = getchar();
        if (!options)
            return key;
        for (const char* tmp = options; *tmp; tmp++)
            if (*tmp == key)
                return key;
        std::cout << "Option '" << key << "' is not valid. Try again.\n";
    }
}

void log (const std::string& s)
{
    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    std::cout << std::setfill('0') << std::setw(7) << (now.tv_sec % 10000000) << "." << std::setw(6) << (now.tv_nsec / 1000) << " | " << s << std::endl;
}

void server()
{
    try
    {
        libSocket::inet::StreamServerSock srv({INADDR_ANY, 55000});
        log("Server created.");
        prompt();
        srv.setListen();
        log("I'm listening.");
        prompt();

        auto accept = srv.getConnection(10000);     // 10 seconds timeout
        if (accept != nullptr)
        {
            log("Got connection");
            while (accept->waitData(100) > 0)
            {
                log("data received!");
                uint8_t buffer[512];
                int lesen = accept->read(buffer, 512);
                if (lesen > 0)
                {
                    log("Got data ---");
                    std::cout << "   Bytes: " << lesen << " - Content: '" << buffer << "'\n";
                    accept->write("Reply back!");
                    log("reply sent.");
                }
                else
                {
                    log("Didn't read anything.");
                    break;
                }
            }
            log("timeout waiting for data.");
        }
    }
    catch (std::exception& e)
    {
        log("Can't open server socket.");
        throw;
    }
}

void client()
{
    libSocket::inet::StreamClientSock cli({"localhost", 55000});

    // prompt();
    log("about to send something");

    try {
        auto wrote = cli.write("Writing something");
        log("Data sent ---");
        std::cout << "   wrote: " << wrote << " bytes\n";
        if (cli.waitData(1000) == 0)
            log("Timeout waiting for data.");
        else
        {
            uint8_t buffer[256];
            int lesen = cli.read(buffer, 256);
            log("Data received ---");
            std::cout << "   Bytes: " << lesen << " - Content: '" << buffer << "'\n";
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Something was wrong ...\n" << e.what() << std::endl;
    }
    // prompt();
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
