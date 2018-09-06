
#ifndef GDT_INTERNAL_NETWORK_IDENTIFIERS_HPP
#define GDT_INTERNAL_NETWORK_IDENTIFIERS_HPP

#ifdef GDT_INTERNAL_NETWORK_CUSTOM_PROTOCOL_ID
 #define GDT_INTERNAL_NETWORK_PROTOCOL_ID GDT_INTERNAL_NETWORK_CUSTOM_PROTOCOL_ID
#else
 #define GDT_INTERNAL_NETWORK_PROTOCOL_ID 1357924680
#endif
#define GDT_INTERNAL_NETWORK_SERVER_PORT 12084
#define GDT_INTERNAL_NETWORK_PACKET_LOST_TIMEOUT_MILLISECONDS 1000
#define GDT_INTERNAL_NETWORK_SENT_PACKET_LIST_MAX_SIZE 34
#define GDT_INTERNAL_NETWORK_CONNECTION_TIMEOUT_MILLISECONDS 10000
#define GDT_INTERNAL_NETWORK_CLIENT_RETRY_TIME_SECONDS 5.0f
#define GDT_INTERNAL_NETWORK_GOOD_RTT_LIMIT_MILLISECONDS 250
#define GDT_INTERNAL_NETWORK_RECEIVED_MAX_SIZE 8192
#define GDT_INTERNAL_NETWORK_HEARTBEAT_SEND_INTERVAL_MILLISECONDS 150

#define GDT_INTERNAL_NETWORK_GOOD_MODE_SEND_INTERVAL 1.0f/30.0f
#define GDT_INTERNAL_NETWORK_BAD_MODE_SEND_INTERVAL 1.0f/10.0f

#include <list>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <atomic>
#include <string>

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
namespace Internal
{
namespace Network
{

extern std::atomic_uint_fast32_t connectionInstanceCount;

struct PacketInfo
{
    PacketInfo();
    PacketInfo(const std::vector<char>& data = std::vector<char>(),
        std::chrono::steady_clock::time_point sentTime =
            std::chrono::steady_clock::time_point(),
        uint32_t address = 0,
        uint32_t id = 0,
        bool isResending = false,
        bool isNotReceivedChecked = false);

    std::vector<char> data;
    std::chrono::steady_clock::time_point sentTime;
    uint32_t address;
    uint32_t id;
    bool isResending;
    bool isNotReceivedChecked;
    bool hasBeenReSent;
};

struct ConnectionData
{
    ConnectionData();
    ConnectionData(uint32_t id, uint32_t lSequence, uint16_t port);

    std::chrono::steady_clock::time_point timeSinceLastReceived;
    std::chrono::steady_clock::time_point timeSinceLastSent;
    uint32_t id;
    uint32_t lSequence;
    uint32_t rSequence;
    uint32_t ackBitfield;
    std::list<PacketInfo> sentPackets;
    std::list<PacketInfo> sendPacketQueue;
    std::chrono::milliseconds rtt;
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
    NONE =          0,
    CONNECT =       0x80000000,
    PING =          0x40000000,
    NO_REC_CHK =    0x20000000,
    RESENDING =     0x10000000
};

bool MoreRecent(uint32_t current, uint32_t previous);
//bool IsSpecialID(uint32_t ID);

bool InitializeSockets();
void CleanupSockets();

std::string addressToString(const uint32_t& address);

uint32_t getLocalIP();

uint32_t getBroadcastAddress();

} // namespace Network
} // namespace Internal
} // namespace GDT

namespace std
{
    template <>
    struct hash<GDT::Internal::Network::ConnectionData>
    {
        std::size_t operator() (const GDT::Internal::Network::ConnectionData& connectionData) const;
    };
}

#endif

