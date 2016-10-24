
#include "NetworkIdentifiers.hpp"

GDT::Network::PacketInfo::PacketInfo() :
address(0),
id(0),
isResending(false)
{}

GDT::Network::PacketInfo::PacketInfo(std::chrono::steady_clock::time_point sentTime,
    uint32_t address,
    uint32_t id,
    bool isResending) :
sentTime(sentTime),
address(address),
id(id),
isResending(isResending)
{}

GDT::Network::ConnectionData::ConnectionData() :
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

GDT::Network::ConnectionData::ConnectionData(uint32_t id, uint32_t lSequence, uint16_t port) :
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

bool GDT::Network::ConnectionData::operator== (const GDT::Network::ConnectionData& other) const
{
    return id == other.id;
}

bool GDT::Network::MoreRecent(uint32_t current, uint32_t previous)
{
    return (((current > previous) && (current - previous <= 0x7FFFFFFF)) ||
            ((previous > current) && (previous - current > 0x7FFFFFFF)));
}

bool GDT::Network::IsSpecialID(uint32_t ID)
{
    return ID == CONNECT || ID == PING;
}

bool GDT::Network::InitializeSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
    WSADATA WsaData;
    return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
    return true;
#endif
}

void GDT::Network::CleanupSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
    WSACleanup();
#endif
}

std::size_t std::hash<GDT::Network::ConnectionData>::operator() (const GDT::Network::ConnectionData& connectionData) const
{
    return connectionData.id;
}

