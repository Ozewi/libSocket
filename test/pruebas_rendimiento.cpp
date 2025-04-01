#include <libSocket/libSocket.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>

using namespace std::string_literals;

uint64_t timenow()
{
  timeval tv;
  gettimeofday(&tv, 0);
  uint64_t rt = tv.tv_sec;
  rt = (rt * 1000000) + tv.tv_usec;
  return rt;
}

char prompt(const char* menu = 0, const char* options = 0)
{
    while (1)
    {
        if (menu)
            printf("%s: ", menu);
        else
            printf("Pulsa Intro para continuar...");
        int key = getchar();
        if (!options)
            return key;
        for (const char* tmp = options; *tmp; tmp++)
            if (*tmp == key)
                return key;
        printf("La opción '%c' no es válida. Inténtalo de nuevo\n", key);
    }
}

void log (const std::string& s)
{
    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    std::cout << std::setfill('0') << std::setw(7) << (now.tv_sec % 10000000) << "." << std::setw(6) << (now.tv_nsec / 1000) << " | " << s << std::endl;
}

#define CHUNK_SIZE_WRITE 100
#define CHUNK_SIZE_READ 10

void server()
{
    try
    {
        libSocket::inet::StreamServerSock srv({INADDR_ANY, 55000});
        log("Server created.");

        srv.setListen();
        log("I'm listening.");

        auto accept = srv.getConnection(3000);     // 3 seconds timeout
        log("accept: "s + (accept.has_value()? "valid"s : "nullptr"s));
        if (accept.has_value())
        {

            uint8_t buf[CHUNK_SIZE_READ];
            uint32_t lesen = 0;
            uint64_t elapsed = timenow();
            for (bool loop = true; loop; )
            {
                int data = accept->read(buf, CHUNK_SIZE_READ);
                if (data)
                {
                    for (int i = 0; i < data && loop; i++)
                        if (buf[i] == '\n')
                            loop = false;
                    lesen += data;
                }

//                if (loop)
//                    sched_yield();
            }
/*
            while (accept->readall(buf, CHUNK_SIZE_READ, 10) == CHUNK_SIZE_READ)
                lesen+=CHUNK_SIZE_READ;
            while (accept->pending())
                lesen += accept->read(buf, CHUNK_SIZE_READ, READ_WAIT_ALL);
*/
            elapsed = timenow() - elapsed;
            log("Read "s + std::to_string(lesen) + " bytes in "s + std::to_string(elapsed) + " microseconds."s);
/*
            if (accept->pending())
            {
                puts("accept tiene datos pendientes");
                uint8_t buffer[512];
                int lesen = accept->read(buffer, 512);
                if (lesen > 0)
                    printf("he leído %d bytes: '%s'\n", lesen, buffer);
                else
                    puts("No he leído nada.");
            }
            printf("pending: %d\n", accept->pending());
*/
        }
        else
            log("Accept: timeout waiting.");
    }
    catch(const std::exception& e)
    {
        log("Can't open server socket.");
        throw;
    }
}

void client()
{
    auto ip_addr = libSocket::getLocalAddr("wlp9s0f3u2");
    log(ip_addr);
    libSocket::inet::StreamClientSock cli({ip_addr, 55000});

    // prompt();
    log("about to send something");
    uint8_t buf[CHUNK_SIZE_WRITE];
    int i;
    uint64_t elapsed = timenow();
    try
    {
        cli.setLinger(1000);
        for (i = 0; i < 10000000; i+=CHUNK_SIZE_WRITE)
            cli.write(buf, CHUNK_SIZE_WRITE, libSocket::SocketBase::WRITE_WAIT_QUEUED);

        *buf = '\n';
        cli.write(buf, 1, libSocket::SocketBase::WRITE_WAIT_QUEUED);
        cli.close();
        elapsed = timenow() - elapsed;
        log("Wrote "s + std::to_string(i) + " bytes in "s + std::to_string(elapsed) + " microseconds."s);
/*
        cli.write("something to say");
        prompt();
*/
    }
    catch(const std::exception& e)
    {
        std::cout << "Something was wrong ...\n" << e.what() << std::endl;
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
