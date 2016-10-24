
#ifndef GDT_NETWORK_IDENTIFIERS_HPP
#define GDT_NETWORK_IDENTIFIERS_HPP

#ifdef GDT_CUSTOM_NETWORK_PROTOCOL_ID
 #define GAME_PROTOCOL_ID GDT_CUSTOM_NETWORK_PROTOCOL_ID
#else
 #define GAME_PROTOCOL_ID 1357924680
#endif
#define GDT_NETWORK_SERVER_PORT 12084
#define HEARTBEAT_MILLISECONDS 1500
#define PACKET_LOST_TIMEOUT_MILLISECONDS 1000
#define SENT_PACKET_LIST_MAX_SIZE 33
#define CONNECTION_TIMEOUT_MILLISECONDS 10000
#define CLIENT_RETRY_TIME_SECONDS 5.0f

#define NETWORK_GOOD_MODE_SEND_INTERVAL 1.0f/30.0f
#define NETWORK_BAD_MODE_SEND_INTERVAL 1.0f/10.0f

#include <list>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <chrono>

#include "Platform.hpp"

#if PLATFORM == PLATFORM_WINDOWS
 #pragma comment( lib, "wsock32.lib" )
 #include <winsock2.h>
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <fcntl.h>
#endif

namespace GDT
{
namespace Network
{

struct PacketInfo
{
    PacketInfo();
    PacketInfo(std::chrono::steady_clock::time_point sentTime =
            std::chrono::steady_clock::time_point(),
        uint32_t address = 0,
        uint32_t id = 0,
        bool isResending = false);

    std::vector<char> data;
    std::chrono::steady_clock::time_point sentTime;
    uint32_t address;
    uint32_t id;
    bool isResending;
};

struct ConnectionData
{
    ConnectionData();
    ConnectionData(uint32_t id, uint32_t lSequence, uint16_t port);

    std::chrono::steady_clock::time_point elapsedTime;
    uint32_t id;
    uint32_t lSequence;
    uint32_t rSequence;
    uint32_t ackBitfield;
    std::list<PacketInfo> sentPackets;
    std::list<PacketInfo> sendPacketQueue;
    std::chrono::steady_clock::duration rtt;
    bool triggerSend;
    float timer;
    bool isGood;
    bool isGoodRtt;
    float toggleTime;
    float toggleTimer;
    float toggledTimer;
    uint16_t port;

    bool operator== (const ConnectionData& other) const;
};

enum SpecialIDs
{
    CONNECT = 0xFFFFFFFF,
    PING = 0xFFFFFFFE
};

bool MoreRecent(uint32_t current, uint32_t previous);
bool IsSpecialID(uint32_t ID);

bool InitializeSockets();
void CleanupSockets();

} // namespace Network
} // namespace GDT

namespace std
{
    template <>
    struct hash<GDT::Network::ConnectionData>
    {
        std::size_t operator() (const GDT::Network::ConnectionData& connectionData) const;
    };
}

#endif

