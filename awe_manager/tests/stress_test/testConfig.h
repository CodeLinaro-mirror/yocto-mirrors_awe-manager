#pragma once
#include <iostream>
#include <iomanip>
#include <string>

#define IP_PREFIX "-ip:"
#define TPORT_PREFIX "-tport:"
#define EPORT_PREFIX "-eport:"
#define DURATION_PREFIX "-duration:"
#define QUIET_PREFIX "-quiet"

class testConfig
{
public:
    bool extractConfig(int argc, char *argv[])
    {
        for (int i = 1; i < argc; i++)
        {
            std::string arg = argv[i];
            if (arg.rfind(IP_PREFIX, 0) == 0)
            {
                targetIP = arg.substr(std::string(IP_PREFIX).length());
                std::cout << std::left << std::setw(30) << IP_PREFIX << eventPort << std::endl;
            }
            else if (arg.rfind(TPORT_PREFIX, 0) == 0)
            {
                tuningPort = arg.substr(std::string(TPORT_PREFIX).length());
                std::cout << std::left << std::setw(30) << TPORT_PREFIX << tuningPort << std::endl;
            }
            else if (arg.rfind(EPORT_PREFIX, 0) == 0)
            {
                eventPort = arg.substr(std::string(EPORT_PREFIX).length());
                std::cout << std::left << std::setw(30) << EPORT_PREFIX << eventPort << std::endl;
            }
            else if (arg.rfind(DURATION_PREFIX, 0) == 0)
            {
                durationSec = std::atoi(arg.c_str() + std::string(DURATION_PREFIX).length());
                std::cout << std::left << std::setw(30) << EPORT_PREFIX << eventPort << std::endl;
            }
            else if (arg.rfind(QUIET_PREFIX, 0) == 0)
            {
                quiet = true;
                std::cout << std::left << std::setw(30) << QUIET_PREFIX << quiet << std::endl;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    void showUsage(const char *program)
    {
        std::cout << "Usage: " << program << "\n"
                                             "       -ip:bsp_ip                   default 127.0.0.1\n"
                                             "       -tport:N                     default 15002, port number for socket interface. User can choose between 15002 - 15098\n"
                                             "       -eport:N                     default 15010, port number for event socket interface. User can choose between 15010 - 15020\n"
                                             "       -duration:N                  default 10 seconds, Duration of test in seconds\n"
                                             "       -quiet                       works in quiet mode, displays only necessary messages\n"
                                             "This program stresses the awe_manager tuning and events interface."
                  << std::endl;
        return;
    }

    std::string targetIP = "127.0.0.1";
    std::string tuningPort = "15002";
    std::string eventPort = "15010";
    uint32_t durationSec = 10;
    bool quiet = false;
};
