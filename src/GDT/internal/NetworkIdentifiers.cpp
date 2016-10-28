
#include "NetworkIdentifiers.hpp"

#include <unistd.h>
#if PLATFORM != PLATFORM_WINDOWS
 #include <netdb.h>
 #include <ifaddrs.h>
 #include <net/if.h>
#endif

#ifndef NDEBUG
 #include <iostream>
#endif

std::atomic_uint_fast32_t GDT::Internal::Network::connectionInstanceCount{};

GDT::Internal::Network::PacketInfo::PacketInfo() :
address(0),
id(0),
isResending(false)
{}

GDT::Internal::Network::PacketInfo::PacketInfo(
    const std::vector<char>& data,
    std::chrono::steady_clock::time_point sentTime,
    uint32_t address,
    uint32_t id,
    bool isResending) :
data(data),
sentTime(sentTime),
address(address),
id(id),
isResending(isResending)
{}

GDT::Internal::Network::ConnectionData::ConnectionData() :
elapsedTime(std::chrono::steady_clock::now()),
rSequence(0),
ackBitfield(0xFFFFFFFF),
triggerSend(false),
timer(0.0f),
isGood(false),
isGoodRtt(false),
toggleTime(30.0f),
toggleTimer(0.0f),
toggledTimer(0.0f)
{}

GDT::Internal::Network::ConnectionData::ConnectionData(uint32_t id, uint32_t lSequence, uint16_t port) :
elapsedTime(std::chrono::steady_clock::now()),
id(id),
lSequence(lSequence),
rSequence(0),
ackBitfield(0xFFFFFFFF),
triggerSend(false),
timer(0.0f),
isGood(false),
isGoodRtt(false),
toggleTime(30.0f),
toggleTimer(0.0f),
toggledTimer(0.0f),
port(port)
{}

bool GDT::Internal::Network::ConnectionData::operator== (const GDT::Internal::Network::ConnectionData& other) const
{
    return id == other.id;
}

bool GDT::Internal::Network::MoreRecent(uint32_t current, uint32_t previous)
{
    return (((current > previous) && (current - previous <= 0x7FFFFFFF)) ||
            ((previous > current) && (previous - current > 0x7FFFFFFF)));
}

bool GDT::Internal::Network::IsSpecialID(uint32_t ID)
{
    return ID == CONNECT || ID == PING;
}

bool GDT::Internal::Network::InitializeSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
    WSADATA WsaData;
    return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
    return true;
#endif
}

void GDT::Internal::Network::CleanupSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
    WSACleanup();
#endif
}

std::string GDT::Internal::Network::addressToString(const uint32_t& address)
{
    return
        std::to_string(address >> 24) + std::string(".") +
        std::to_string((address >> 16) & 0xFF) + std::string(".") +
        std::to_string((address >> 8) & 0xFF) + std::string(".") +
        std::to_string(address & 0xFF);
}

uint32_t GDT::Internal::Network::getLocalIP()
{
    // initialize if not initialized
    if(connectionInstanceCount++ == 0)
    {
        if(!InitializeSockets())
        {
            --connectionInstanceCount;
            return 0;
        }
    }

    auto cleanupF = [] () {
        // cleanup if only this function requires initialized state
        if(--connectionInstanceCount == 0)
        {
            CleanupSockets();
        }
    };

    // get hostname
    char hostname[1024];
    if(gethostname(hostname, 1024) != 0)
    {
        cleanupF();
        return 0;
    }

    // get local address
    hostent *host = gethostbyname(hostname);
    if(host == nullptr)
    {
        cleanupF();
        return 0;
    }

    uint32_t address =
        ntohl(((in_addr*)(host->h_addr))->s_addr);

    cleanupF();

#ifndef NDEBUG
    std::cout << "getLocalIP(): Got address " << addressToString(address) << std::endl;
#endif

    return address;
}

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
uint32_t GDT::Internal::Network::getBroadcastAddress()
{
    // initialize if not initialized
    if(connectionInstanceCount++ == 0)
    {
        if(!InitializeSockets())
        {
            --connectionInstanceCount;
            return 0;
        }
    }

    auto cleanupF = [] () {
        // cleanup if only this function requires initialized state
        if(--connectionInstanceCount == 0)
        {
            CleanupSockets();
        }
    };

    ifaddrs* ifaddrsInfo;
    if(getifaddrs(&ifaddrsInfo) != 0)
    {
        cleanupF();
        return 0;
    }

    uint32_t localIP = getLocalIP();
    uint32_t broadcastAddress = 0;
    do
    {
        sockaddr_in* sockaddrInfo = (sockaddr_in*)(ifaddrsInfo->ifa_addr);
        if(sockaddrInfo != nullptr)
        {
            if(localIP == ntohl(sockaddrInfo->sin_addr.s_addr))
            {
                if((ifaddrsInfo->ifa_flags | IFF_BROADCAST) != 0)
                {
                    sockaddr_in* broadcastInfo = (sockaddr_in*)(ifaddrsInfo->ifa_ifu.ifu_broadaddr);
                    if(broadcastInfo != nullptr)
                    {
                        broadcastAddress = ntohl(broadcastInfo->sin_addr.s_addr);
#ifndef NDEBUG
                        std::cout << "getBroadcastAddress(): got " << addressToString(broadcastAddress) << std::endl;
#endif
                        cleanupF();
                        return broadcastAddress;
                    }
                    else
                    {
                        cleanupF();
                        return 0;
                    }
                }
                else
                {
                    cleanupF();
                    return 0;
                }
            }
        }
        ifaddrsInfo = ifaddrsInfo->ifa_next;
    } while (ifaddrsInfo != nullptr);

    cleanupF();
    return 0;
}
#endif

std::size_t std::hash<GDT::Internal::Network::ConnectionData>::operator() (const GDT::Internal::Network::ConnectionData& connectionData) const
{
    return connectionData.id;
}

