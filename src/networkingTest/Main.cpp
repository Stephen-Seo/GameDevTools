
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include <GDT/GameLoop.hpp>
#include <GDT/NetworkConnection.hpp>

void printUsage()
{
    std::cout << "USAGE:"
        "\n  ./NetworkingTest -s <server_port>"
        "\n  ./NetworkingTest -c <server_ip> <server_port> [client_port]"
        "\n  ./NetworkingTest -b <server_port> [client_port]"
        << std::endl;
}

uint32_t parseIP(std::string string)
{
    uint32_t address = 0;

    auto index = string.find(".");
    if(index == std::string::npos)
    {
        std::cerr << "Failed to find first ." << std::endl;
        return 0;
    }
    address |= std::stoul(string.substr(0, index)) << 24;

    auto prevIndex = index;
    index = string.find(".", index + 1);
    if(index == std::string::npos)
    {
        std::cerr << "Failed to find second ." << std::endl;
        return 0;
    }
    address |= (std::stoul(string.substr(prevIndex + 1, index)) & 0xFF) << 16;

    prevIndex = index;
    index = string.find(".", index + 1);
    if(index == std::string::npos)
    {
        std::cerr << "Failed to find third ." << std::endl;
        return 0;
    }
    address |= (std::stoul(string.substr(prevIndex + 1, index)) & 0xFF) << 8;

    prevIndex = index;
    index = string.size();
    if(index == std::string::npos)
    {
        return 0;
    }
    address |= (std::stoul(string.substr(prevIndex + 1, index)) & 0xFF);

    return address;
}

int main(int argc, char** argv)
{
    if(argc < 3 || argc > 5)
    {
        printUsage();
        return 1;
    }

    uint32_t serverIP = 0;
    uint16_t serverPort = 0;
    uint16_t clientPort = 0;
    bool isServer = false;
    bool willBroadcast = false;
    if(std::strcmp(argv[1], "-s") == 0 && argc == 3)
    {
        isServer = true;
        serverPort = std::strtoul(argv[2], nullptr, 10);
        if(serverPort == 0)
        {
            printUsage();
            return 2;
        }
    }
    else if(std::strcmp(argv[1], "-c") == 0 && (argc == 4 || argc == 5))
    {
        isServer = false;
        serverIP = parseIP(std::string(argv[2]));
        serverPort = std::strtoul(argv[3], nullptr, 10);
        if(serverIP == 0 || serverPort == 0)
        {
            printUsage();
            return 3;
        }
        if(argc == 5)
        {
            clientPort = std::strtoul(argv[4], nullptr, 10);
            if(clientPort == 0)
            {
                printUsage();
                return 4;
            }
        }
    }
    else if(std::strcmp(argv[1], "-b") == 0 && (argc == 3 || argc == 4))
    {
        isServer = false;
        willBroadcast = true;
        serverPort = std::strtoul(argv[2], nullptr, 10);
        if(serverPort == 0)
        {
            printUsage();
            return 5;
        }
        if(argc == 4)
        {
            clientPort = std::strtoul(argv[3], nullptr, 10);
            if(clientPort == 0)
            {
                printUsage();
                return 6;
            }
        }
    }
    else
    {
        printUsage();
        return 7;
    }

    using Connection = GDT::Network::Connection;
    Connection connection(isServer ? Connection::SERVER : Connection::CLIENT,
        serverPort,
        clientPort,
        willBroadcast);

    connection.connectToServer(serverIP);

    float timer = 0;

    auto update = [&connection, &timer] (float deltaTime) {
        connection.update(deltaTime);
        timer += deltaTime;
        if(timer >= 10.0f)
        {
            timer -= 10.0f;
            char data[5] = "derp";
            connection.sendPacket(data, 5, connection.getConnected().front());
        }
    };
    auto draw = [] () {};

    if(isServer)
    {
        connection.setReceivedCallback([] (const char* data, uint32_t count, uint32_t address, bool outOfOrder) {
            std::cout << "Received extra as server" << std::endl;
        });
    }
    else
    {
        connection.setReceivedCallback([] (const char* data, uint32_t count, uint32_t address, bool outOfOrder) {
            std::cout << "Received extra as client" << std::endl;
        });
    }

    bool runFlag = true;

    GDT::IntervalBasedGameLoop(&runFlag,
        update,
        draw);
}

