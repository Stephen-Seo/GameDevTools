
#include "NetworkConnection.hpp"

#include <cstring>
#include <unistd.h>

GDT::NetworkConnection::NetworkConnection(Mode mode, unsigned short serverPort, unsigned short clientPort, bool clientBroadcast) :
acceptNewConnections(true),
ignoreOutOfSequence(false),
resendTimedOutPackets(true),
mode(mode),
clientSentAddressSet(false),
initialized(false),
validState(false),
invalidNoticeTimer(INVALID_NOTICE_TIME),
serverPort(serverPort),
clientPort(clientPort),
clientRetryTimer(GDT_INTERNAL_NETWORK_CLIENT_RETRY_TIME_SECONDS),
clientBroadcast(clientBroadcast)
{
    if(GDT::Internal::Network::connectionInstanceCount++ == 0)
    {
#ifndef NDEBUG
        bool result = GDT::Internal::Network::InitializeSockets();
        assert(result);
#else
        GDT::Internal::Network::InitializeSockets();
#endif
    }
}

GDT::NetworkConnection::~NetworkConnection()
{
    if(validState)
    {
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
        close(socketHandle);
#else
        closesocket(socketHandle);
#endif
    }

    if(--GDT::Internal::Network::connectionInstanceCount == 0)
    {
        GDT::Internal::Network::CleanupSockets();
    }
}

void GDT::NetworkConnection::update(float deltaTime)
{
    if(!initialized)
    {
#ifndef NDEBUG
        std::cout << "Lazy initializing connection..." << std::endl;
#endif
        initialize();
        initialized = true;
    }

    if(!validState)
    {
        invalidNoticeTimer -= deltaTime;
        if(invalidNoticeTimer <= 0)
        {
            std::clog << "Warning: Connection is in invalid state, not doing anything!" << std::endl;
            invalidNoticeTimer = INVALID_NOTICE_TIME;
        }
        return;
    }

    for(auto iter = connectionData.begin(); iter != connectionData.end(); ++iter)
    {
        iter->second.toggleTimer += deltaTime;
        iter->second.toggledTimer += deltaTime;

        if(iter->second.isGood && !iter->second.isGoodRtt)
        {
            // good status, rtt is bad
#ifndef NDEBUG
            std::cout << "Switching to bad network mode for " << GDT::Internal::Network::addressToString(iter->first) << '\n';
#endif
            iter->second.isGood = false;
            if(iter->second.toggledTimer <= 10.0f)
            {
                iter->second.toggleTime *= 2.0f;
                if(iter->second.toggleTime > 60.0f)
                {
                    iter->second.toggleTime = 60.0f;
                }
            }
            iter->second.toggledTimer = 0.0f;
        }
        else if(iter->second.isGood)
        {
            // good status, rtt is good
            if(iter->second.toggleTimer >= 10.0f)
            {
                iter->second.toggleTimer = 0.0f;
                iter->second.toggleTime /= 2.0f;
                if(iter->second.toggleTime < 1.0f)
                {
                    iter->second.toggleTime = 1.0f;
                }
            }
        }
        else if(!iter->second.isGood && iter->second.isGoodRtt)
        {
            // bad status, rtt is good
            if(iter->second.toggledTimer >= iter->second.toggleTime)
            {
                iter->second.toggleTimer = 0.0f;
                iter->second.toggledTimer = 0.0f;
#ifndef NDEBUG
                std::cout << "Switching to good network mode for " << GDT::Internal::Network::addressToString(iter->first) << '\n';
#endif
                iter->second.isGood = true;
            }
        }
        else
        {
            // bad status, rtt is bad
            iter->second.toggledTimer = 0.0f;
        }

        iter->second.timer += deltaTime;
        if(iter->second.timer >= (iter->second.isGood ? GDT_INTERNAL_NETWORK_GOOD_MODE_SEND_INTERVAL : GDT_INTERNAL_NETWORK_BAD_MODE_SEND_INTERVAL))
        {
            iter->second.timer = 0.0f;
            iter->second.triggerSend = true;
        }
    }

    if(mode == SERVER)
    {
        // check if clients have timed out
        std::list<uint32_t> disconnectQueue;
        for(auto iter = connectionData.begin(); iter != connectionData.end(); ++iter)
        {
            auto duration = std::chrono::steady_clock::now() - iter->second.elapsedTime;
            if(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() >= GDT_INTERNAL_NETWORK_CONNECTION_TIMEOUT_MILLISECONDS)
            {
                disconnectQueue.push_front(iter->first);
            }
        }

        for(auto iter = disconnectQueue.begin(); iter != disconnectQueue.end(); ++iter)
        {
#ifndef NDEBUG
            std::cout << "Disconnected " << GDT::Internal::Network::addressToString(*iter) << std::endl;
#endif
            unregisterConnection(*iter);
        }

        // send packet as server to each client
        for(auto iter = connectionData.begin(); iter != connectionData.end(); ++iter)
        {
            if(iter->second.triggerSend)
            {
                iter->second.triggerSend = false;
                if(!iter->second.sendPacketQueue.empty())
                {
                    auto pInfo = iter->second.sendPacketQueue.back();
                    iter->second.sendPacketQueue.pop_back();

                    std::vector<char> data;
                    uint32_t sequenceID;

                    preparePacket(data, sequenceID, iter->first, false, pInfo.isResending, pInfo.isNotReceivedChecked);
                    // append packetInfo's data to prepared data
                    data.insert(data.end(), pInfo.data.begin(), pInfo.data.end());

                    // send data
                    sockaddr_in destinationInfo;
                    destinationInfo.sin_family = AF_INET;
                    destinationInfo.sin_addr.s_addr = htonl(iter->first);
                    destinationInfo.sin_port = htons(iter->second.port);
                    long int sentBytes = sendto(socketHandle,
                        (const char*) data.data(),
                        data.size(),
                        0,
                        (sockaddr*) &destinationInfo,
                        sizeof(sockaddr_in));

                    if(sentBytes < 0)
                    {
                        std::cerr << "Failed to send packet to client!" << std::endl;
                    }
                    else
                    {
                        unsigned long int u_sentBytes = sentBytes;
                        if(u_sentBytes != data.size())
                        {
                            std::cerr << "Failed to send packet to client!" << std::endl;
                        }
                        else
                        {
                            if(!pInfo.isNotReceivedChecked)
                            {
                                // store current packet info in sentPackets
                                iter->second.sentPackets.push_front(PacketInfo(data, std::chrono::steady_clock::now(), iter->first, sequenceID));
                                checkSentPacketsSize(iter->first);
                            }
                            else
                            {
                                iter->second.sentPackets.push_front(PacketInfo(std::vector<char>(), std::chrono::steady_clock::now(), iter->first, sequenceID, false, true));
                                checkSentPacketsSize(iter->first);
                            }
                        }
                    }
                }
                else
                {
                    // send a heartbeat(empty) packet because the queue is empty

                    std::vector<char> data;
                    uint32_t sequenceID;
                    preparePacket(data, sequenceID, iter->first, false, false, true);

                    sockaddr_in destinationInfo;
                    destinationInfo.sin_family = AF_INET;
                    destinationInfo.sin_addr.s_addr = htonl(iter->first);
                    destinationInfo.sin_port = htons(iter->second.port);
                    long int sentBytes = sendto(socketHandle,
                        (const char*) data.data(),
                        data.size(),
                        0,
                        (sockaddr*) &destinationInfo,
                        sizeof(sockaddr_in));

                    if(sentBytes < 0)
                    {
                        std::cerr << "Failed to send heartbeat packet to client!" << std::endl;
                    }
                    else
                    {
                        unsigned long int u_sentBytes = sentBytes;
                        if(u_sentBytes != data.size())
                        {
                            std::cerr << "Failed to send heartbeat packet to client!" << std::endl;
                        }
                        else
                        {
                            iter->second.sentPackets.push_front(PacketInfo(std::vector<char>(), std::chrono::steady_clock::now(), iter->first, sequenceID, false, true));
                            checkSentPacketsSize(iter->first);
                        }
                    }
                }
            }
        }

        // receive packet
        std::vector<char> data(GDT_INTERNAL_NETWORK_RECEIVED_MAX_SIZE);
#if PLATFORM == PLATFORM_WINDOWS
        typedef int socklen_t;
#endif
        sockaddr_in receivedData;
        socklen_t receivedDataSize = sizeof(receivedData);

        int bytes = recvfrom(socketHandle,
            data.data(),
            GDT_INTERNAL_NETWORK_RECEIVED_MAX_SIZE,
            0,
            (sockaddr*) &receivedData,
            &receivedDataSize);

        if(bytes >= 20)
        {
            uint32_t address = ntohl(receivedData.sin_addr.s_addr);
            uint16_t port = ntohs(receivedData.sin_port);

            uint32_t* tempPtr = (uint32_t*)data.data();
            uint32_t protocolID = ntohl(*tempPtr);

            // check protocol ID
            if(protocolID != GDT_INTERNAL_NETWORK_PROTOCOL_ID)
                return;

            tempPtr = (uint32_t*)(data.data() + 4);
            uint32_t ID = ntohl(*tempPtr);
            tempPtr = (uint32_t*)(data.data() + 8);
            uint32_t sequence = ntohl(*tempPtr);
            tempPtr = (uint32_t*)(data.data() + 12);
            uint32_t ack = ntohl(*tempPtr);
            tempPtr = (uint32_t*)(data.data() + 16);
            uint32_t ackBitfield = ntohl(*tempPtr);

            bool isConnect = (ID & GDT::Internal::Network::CONNECT) != 0;
            bool isPing = (ID & GDT::Internal::Network::PING) != 0;
            bool isNotReceivedChecked = (ID & GDT::Internal::Network::NO_REC_CHK) != 0;
            bool isResent = (ID & GDT::Internal::Network::RESENDING) != 0;

            ID = ID & 0x0FFFFFFF;

            if(isConnect && acceptNewConnections)
            {
                if(connectionData.find(address) == connectionData.end())
                {
#ifndef NDEBUG
                    std::cout << "SERVER: Establishing new connection with " << GDT::Internal::Network::addressToString(address) << '\n';
#endif
                    // Establish connection
                    registerConnection(address, 0, port);
                    connectionData.at(address).triggerSend = true;
                }
                return;
            }
            else if(isPing)
            {
                connectionData.at(address).triggerSend = true;
            }
            else if(connectionData.find(address) == connectionData.end())
            {
                // Unknown client not attemping to connect, ignoring
                return;
            }
            else if(ID != connectionData.at(address).id)
            {
                // ID and address doesn't match, ignoring
                return;
            }

            // packet is valid
#ifndef NDEBUG
            std::cout << "Valid packet " << sequence << " received from " << GDT::Internal::Network::addressToString(address) << '\n';
#endif

            bool outOfOrder = false;

            lookupRtt(address, ack);

            connectionData.at(address).elapsedTime = std::chrono::steady_clock::now();
            checkSentPackets(ack, ackBitfield, address);

            uint32_t diff = 0;
            if(sequence > connectionData.at(address).rSequence)
            {
                diff = sequence - connectionData.at(address).rSequence;
                if(diff <= 0x7FFFFFFF)
                {
                    // sequence is more recent
                    connectionData.at(address).rSequence = sequence;
                    shiftBitfield(address, diff);
                }
                else
                {
                    // sequence is older packet id, diff requires recalc
                    diff = sequence + (0xFFFFFFFF - connectionData.at(address).rSequence) + 1;

                    if((connectionData.at(address).ackBitfield & (0x100000000 >> diff)) != 0x0)
                    {
                        // already received packet
                        return;
                    }
                    connectionData.at(address).ackBitfield |= (0x100000000 >> diff);

                    if(ignoreOutOfSequence)
                        return;

                    outOfOrder = true;
                }
            }
            else if(connectionData.at(address).rSequence > sequence)
            {
                diff = connectionData.at(address).rSequence - sequence;
                if(diff > 0x7FFFFFFF)
                {
                    // sequence is more recent, diff requires recalc
                    diff = sequence + (0xFFFFFFFF - connectionData.at(address).rSequence) + 1;

                    connectionData.at(address).rSequence = sequence;
                    shiftBitfield(address, diff);
                }
                else
                {
                    // sequence is older packet id
                    if((connectionData.at(address).ackBitfield & (0x100000000 >> diff)) != 0x0)
                    {
                        // already received packet
                        return;
                    }
                    connectionData.at(address).ackBitfield |= (0x100000000 >> diff);

                    if(ignoreOutOfSequence)
                        return;

                    outOfOrder = true;
                }
            }
            else
            {
                // duplicate packet, ignoring
                return;
            }

#ifndef NDEBUG
            if(outOfOrder)
            {
                std::cout << "Out of order packet\n";
            }
#endif

            receivedPacket(data.data() + 20, bytes - 20, address, outOfOrder, isResent, isNotReceivedChecked);
        }
    } // if(mode == SERVER)
    else if(mode == CLIENT)
    {
        uint32_t& serverAddress = clientSentAddress;
        // connection established
        if(connectionData.size() > 0)
        {
            // check if timed out
            auto duration = std::chrono::steady_clock::now() - connectionData.at(serverAddress).elapsedTime;
            if(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() > GDT_INTERNAL_NETWORK_CONNECTION_TIMEOUT_MILLISECONDS)
            {
#ifndef NDEBUG
                std::cout << "Disconnected from server " << GDT::Internal::Network::addressToString(serverAddress) << '\n';
#endif
                unregisterConnection(serverAddress);
                return;
            }

            // send packet as client to server
            if(connectionData.at(serverAddress).triggerSend)
            {
                connectionData.at(serverAddress).triggerSend = false;
                if(!connectionData.at(serverAddress).sendPacketQueue.empty())
                {
                    PacketInfo pInfo = connectionData.at(serverAddress).sendPacketQueue.back();
                    connectionData.at(serverAddress).sendPacketQueue.pop_back();


                    std::vector<char> data;
                    uint32_t sequenceID;

                    preparePacket(data, sequenceID, serverAddress, false, pInfo.isResending, pInfo.isNotReceivedChecked);

                    data.insert(data.end(), pInfo.data.begin(), pInfo.data.end());

                    // send data
                    sockaddr_in destinationInfo;
                    destinationInfo.sin_family = AF_INET;
                    destinationInfo.sin_addr.s_addr = htonl(serverAddress);
                    destinationInfo.sin_port = htons(serverPort);
                    long int sentBytes = sendto(socketHandle,
                        (const char*) data.data(),
                        data.size(),
                        0,
                        (sockaddr*) &destinationInfo,
                        sizeof(sockaddr_in));

                    if(sentBytes < 0)
                    {
                        std::cerr << "Failed to send packet to server!" << std::endl;
                    }
                    else
                    {
                        unsigned long int u_sentBytes = sentBytes;
                        if(u_sentBytes != data.size())
                        {
                            std::cerr << "Failed to send packet to server!" << std::endl;
                        }
                        else
                        {
                            if(!pInfo.isNotReceivedChecked)
                            {
                                connectionData.at(serverAddress).sentPackets.push_front(PacketInfo(data, std::chrono::steady_clock::now(), serverAddress, sequenceID));
                                checkSentPacketsSize(serverAddress);
                            }
                            else
                            {
                                connectionData.at(serverAddress).sentPackets.push_front(PacketInfo(std::vector<char>(), std::chrono::steady_clock::now(), serverAddress, sequenceID, false, true));
                                checkSentPacketsSize(serverAddress);
                            }
                        }
                    }
                }
                else
                {
                    // send a heartbeat(empty) packet because the queue is empty

                    std::vector<char> data;
                    uint32_t sequenceID;
                    preparePacket(data, sequenceID, serverAddress, false, false, true);

                    // send data
                    sockaddr_in destinationInfo;
                    destinationInfo.sin_family = AF_INET;
                    destinationInfo.sin_addr.s_addr = htonl(serverAddress);
                    destinationInfo.sin_port = htons(serverPort);
                    long int sentBytes = sendto(socketHandle,
                        (const char*) data.data(),
                        data.size(),
                        0,
                        (sockaddr*) &destinationInfo,
                        sizeof(sockaddr_in));

                    if(sentBytes < 0)
                    {
                        std::cerr << "Failed to send heartbeat packet to server!" << std::endl;
                    }
                    else
                    {
                        unsigned long int u_sentBytes = sentBytes;
                        if(u_sentBytes != data.size())
                        {
                            std::cerr << "Failed to send heartbeat packet to server!" << std::endl;
                        }
                        else
                        {
                            connectionData.at(serverAddress).sentPackets.push_front(PacketInfo(std::vector<char>(), std::chrono::steady_clock::now(), serverAddress, sequenceID, false, true));
                            checkSentPacketsSize(serverAddress);
                        }
                    }
                }
            }

            // receive
            std::vector<char> data(GDT_INTERNAL_NETWORK_RECEIVED_MAX_SIZE);
#if PLATFORM == PLATFORM_WINDOWS
            typedef int socklen_t;
#endif
            sockaddr_in receivedData;
            socklen_t receivedDataSize = sizeof(receivedData);

            int bytes = recvfrom(socketHandle,
                data.data(),
                GDT_INTERNAL_NETWORK_RECEIVED_MAX_SIZE,
                0,
                (sockaddr*) &receivedData,
                &receivedDataSize);

            uint32_t address = ntohl(receivedData.sin_addr.s_addr);
            uint16_t port = ntohs(receivedData.sin_port);

            if(bytes >= 20 && address == serverAddress && port == serverPort)
            {
                uint32_t* tempPtr = (uint32_t*)data.data();
                uint32_t protocolID = ntohl(*tempPtr);

                if(protocolID != GDT_INTERNAL_NETWORK_PROTOCOL_ID)
                    return;

                tempPtr = (uint32_t*)(data.data() + 4);
                uint32_t ID = ntohl(*tempPtr);
                tempPtr = (uint32_t*)(data.data() + 8);
                uint32_t sequence = ntohl(*tempPtr);
                tempPtr = (uint32_t*)(data.data() + 12);
                uint32_t ack = ntohl(*tempPtr);
                tempPtr = (uint32_t*)(data.data() + 16);
                uint32_t bitfield = ntohl(*tempPtr);

                bool isConnect = (ID & GDT::Internal::Network::CONNECT) != 0;
                bool isPing = (ID & GDT::Internal::Network::PING) != 0;
                bool isNotReceivedChecked = (ID & GDT::Internal::Network::NO_REC_CHK) != 0;
                bool isResent = (ID & GDT::Internal::Network::RESENDING) != 0;

                ID = ID & 0x0FFFFFFF;

                if(isPing)
                {
                    connectionData.at(serverAddress).triggerSend = true;
                }
                else if(ID != connectionData.at(serverAddress).id
                        || isConnect)
                    return;

                // packet is valid
#ifndef NDEBUG
                std::cout << "Valid packet " << sequence << " received from " << GDT::Internal::Network::addressToString(serverAddress) << '\n';
#endif

                bool outOfOrder = false;

                lookupRtt(serverAddress, ack);

                connectionData.at(serverAddress).elapsedTime = std::chrono::steady_clock::now();
                checkSentPackets(ack, bitfield, serverAddress);

                uint32_t diff = 0;
                if(sequence > connectionData.at(serverAddress).rSequence)
                {
                    diff = sequence - connectionData.at(serverAddress).rSequence;
                    if(diff <= 0x7FFFFFFF)
                    {
                        // sequence is more recent
                        connectionData.at(serverAddress).rSequence = sequence;
                        shiftBitfield(address, diff);
                    }
                    else
                    {
                        // sequence is older packet id, diff requires recalc
                        diff = sequence + (0xFFFFFFFF - connectionData.at(serverAddress).rSequence) + 1;

                        if((connectionData.at(serverAddress).ackBitfield & (0x100000000 >> diff)) != 0x0)
                        {
                            // already received packet
                            return;
                        }
                        connectionData.at(serverAddress).ackBitfield |= (0x100000000 >> diff);

                        if(ignoreOutOfSequence)
                            return;

                        outOfOrder = true;
                    }
                }
                else if(connectionData.at(serverAddress).rSequence > sequence)
                {
                    diff = connectionData.at(serverAddress).rSequence - sequence;
                    if(diff > 0x7FFFFFFF)
                    {
                        // sequence is more recent, diff requires recalc
                        diff = sequence + (0xFFFFFFFF - connectionData.at(serverAddress).rSequence) + 1;

                        connectionData.at(serverAddress).rSequence = sequence;
                        shiftBitfield(address, diff);
                    }
                    else
                    {
                        // sequence is older packet id
                        if((connectionData.at(serverAddress).ackBitfield & (0x100000000 >> diff)) != 0x0)
                        {
                            // already received packet
                            return;
                        }
                        connectionData.at(serverAddress).ackBitfield |= (0x100000000 >> diff);

                        if(ignoreOutOfSequence)
                            return;

                        outOfOrder = true;
                    }
                }
                else
                {
                    // duplicate packet, ignoring
                    return;
                }

#ifndef NDEBUG
                if(outOfOrder)
                {
                    std::cout << "Out of order packet\n";
                }
#endif

                receivedPacket(data.data() + 20, bytes - 20, serverAddress, outOfOrder, isResent, isNotReceivedChecked);
            }
        }
        // connection not yet established
        else if(acceptNewConnections)
        {
            // check retry timer
            clientRetryTimer += deltaTime;
            if(clientRetryTimer >= GDT_INTERNAL_NETWORK_CLIENT_RETRY_TIME_SECONDS && (clientSentAddressSet || clientBroadcast))
            {
#ifndef NDEBUG
                std::cout << "CLIENT: Establishing connection with server..." << std::endl;
#endif
                clientRetryTimer = 0.0f;
                char data[20];
                uint32_t temp = htonl(GDT_INTERNAL_NETWORK_PROTOCOL_ID);
                memcpy(data, &temp, 4);
                temp = htonl(GDT::Internal::Network::CONNECT);
                memcpy(data + 4, &temp, 4);
                temp = 0;
                memcpy(data + 8, &temp, 4);
                memcpy(data + 12, &temp, 4);
                temp = 0xFFFFFFFF;
                memcpy(data + 16, &temp, 4);

                // send data
                sockaddr_in destinationInfo;
                destinationInfo.sin_family = AF_INET;
                destinationInfo.sin_port = htons(serverPort);
                if(clientBroadcast)
                {
                    uint32_t broadcastAddress = GDT::Internal::Network::getBroadcastAddress();
                    if(broadcastAddress == 0)
                    {
                        std::cerr << "WARNING: Failed to get local address!" << std::endl;
                        destinationInfo.sin_addr.s_addr = 0xFFFFFFFF;
                    }
                    else
                    {
                        destinationInfo.sin_addr.s_addr = htonl(broadcastAddress);
                    }
                }
                else
                {
                    destinationInfo.sin_addr.s_addr = htonl(serverAddress);
                }
                int sentBytes = sendto(socketHandle,
                    (const char*) data,
                    20,
                    0,
                    (sockaddr*) &destinationInfo,
                    sizeof(sockaddr_in));
                if(sentBytes != 20)
                {
                    std::cerr << "ERROR: Failed to send initiate connection packet to server!" << std::endl;
                }
            }

            // receive
            std::vector<char> data(GDT_INTERNAL_NETWORK_RECEIVED_MAX_SIZE);
#if PLATFORM == PLATFORM_WINDOWS
            typedef int socklen_t;
#endif
            sockaddr_in receivedData;
            socklen_t receivedDataSize = sizeof(receivedData);

            int bytes = recvfrom(socketHandle,
                data.data(),
                GDT_INTERNAL_NETWORK_RECEIVED_MAX_SIZE,
                0,
                (sockaddr*) &receivedData,
                &receivedDataSize);

            uint32_t address = ntohl(receivedData.sin_addr.s_addr);
            uint16_t port = ntohs(receivedData.sin_port);

            if(bytes >= 20 && port == serverPort)
            {
#ifndef NDEBUG
                std::cout << "." << std::flush;
#endif
                uint32_t* tempPtr = (uint32_t*)data.data();
                uint32_t protocolID = ntohl(*tempPtr);

                if(protocolID != GDT_INTERNAL_NETWORK_PROTOCOL_ID)
                    return;

                tempPtr = (uint32_t*)(data.data() + 4);
                uint32_t ID = ntohl(*tempPtr) & 0x0FFFFFFF;
//                tempPtr = (uint32_t*)(data.data() + 8);
//                uint32_t sequence = ntohl(*tempPtr);
//                tempPtr = (uint32_t*)(data.data() + 12);
//                uint32_t ack = ntohl(*tempPtr);
//                tempPtr = (uint32_t*)(data.data() + 16);
//                uint32_t bitfield = ntohl(*tempPtr);

                if(clientBroadcast)
                {
                    clientSentAddress = address;
                    clientSentAddressSet = true;
                }
                registerConnection(address, ID, serverPort);
            }
        }
    } // elif(mode == CLIENT)
}

void GDT::NetworkConnection::connectToServer(unsigned char a,
                                 unsigned char b,
                                 unsigned char c,
                                 unsigned char d)
{
    connectToServer(((uint32_t)a << 24) | ((uint32_t)b << 16) |
                    ((uint32_t)c << 8) | (uint32_t)d);
}

void GDT::NetworkConnection::connectToServer(uint32_t address)
{
    if(mode != CLIENT)
        return;

#ifndef NDEBUG
    std::cout << "CLIENT: storing server ip as " << GDT::Internal::Network::addressToString(address) << '\n';
#endif

    clientSentAddress = address;
    clientSentAddressSet = true;
}

void GDT::NetworkConnection::sendPacket(const std::vector<char>& packetData, uint32_t address, bool isReceivedChecked)
{
    if(connectionData.find(address) == connectionData.end())
    {
        std::clog << "WARNING: Tried to queue packet to nonexistent recipient!" << std::endl;
    }
    else
    {
        connectionData.at(address).sendPacketQueue.push_front(PacketInfo(
            packetData,
            std::chrono::steady_clock::time_point(),
            address, 0, false, !isReceivedChecked));
    }
}

void GDT::NetworkConnection::resendPacket(const std::vector<char>& packetData, uint32_t address)
{
    if(connectionData.find(address) == connectionData.end())
    {
        std::clog << "WARNING: Tried to queue packet to nonexistent recipient!" << std::endl;
    }
    else
    {
        connectionData.at(address).sendPacketQueue.push_front(PacketInfo(
            packetData,
            std::chrono::steady_clock::time_point(),
            address,
            0,
            true));
    }
}

void GDT::NetworkConnection::sendPacket(const char* packetData, uint32_t packetSize, uint32_t address, bool isReceivedChecked)
{
    std::vector<char> data(packetData, packetData + packetSize);
    sendPacket(data, address, isReceivedChecked);
}

float GDT::NetworkConnection::getRtt()
{
    if(connectionData.empty())
    {
        return 0;
    }
    return connectionData.begin()->second.rtt.count() / 1000.0f;
}

float GDT::NetworkConnection::getRtt(uint32_t address)
{
    if(connectionData.find(address) == connectionData.end())
    {
        return 0;
    }
    return connectionData.at(address).rtt.count() / 1000.0f;
}

void GDT::NetworkConnection::setReceivedCallback(std::function<void(const char*, uint32_t, uint32_t, bool, bool, bool)> callback)
{
    receivedCallback = callback;
}

void GDT::NetworkConnection::setConnectedCallback(std::function<void(uint32_t)> callback)
{
    connectedCallback = callback;
}

void GDT::NetworkConnection::setDisconnectedCallback(std::function<void(uint32_t)> callback)
{
    disconnectedCallback = callback;
}

std::vector<uint32_t> GDT::NetworkConnection::getConnected()
{
    std::vector<uint32_t> connectedList;

    for(auto iter = connectionData.begin(); iter != connectionData.end(); ++iter)
    {
        connectedList.push_back(iter->first);
    }

    return connectedList;
}

unsigned int GDT::NetworkConnection::getPacketQueueSize(uint32_t destinationAddress)
{
    auto connectionDataIter = connectionData.find(destinationAddress);
    if(connectionDataIter == connectionData.end())
    {
        return 0;
    }

    return connectionDataIter->second.sendPacketQueue.size();
}

void GDT::NetworkConnection::clearPacketQueue(uint32_t destinationAddress)
{
    auto connectionDataIter = connectionData.find(destinationAddress);
    if(connectionDataIter == connectionData.end())
    {
        return;
    }

    connectionDataIter->second.sendPacketQueue.clear();
}

bool GDT::NetworkConnection::connectionIsGood()
{
    auto connectionDataIter = connectionData.begin();
    if(connectionDataIter == connectionData.end())
    {
        return false;
    }

    return connectionDataIter->second.isGood;
}

bool GDT::NetworkConnection::connectionIsGood(uint32_t destinationAddress)
{
    auto connectionDataIter = connectionData.find(destinationAddress);
    if(connectionDataIter == connectionData.end())
    {
        return false;
    }

    return connectionDataIter->second.isGood;
}

void GDT::NetworkConnection::reset(NetworkConnection::Mode mode, unsigned short serverPort, unsigned short clientPort, bool clientBroadcast)
{
    this->mode = mode;
    this->serverPort = serverPort;
    this->clientPort = clientPort;
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
    if(validState) close(socketHandle);
#else
    if(validState) closesocket(socketHandle);
#endif
    connectionData.clear();
    clientSentAddressSet = false;
    initialized = false;
    validState = false;
    invalidNoticeTimer = INVALID_NOTICE_TIME;
    clientRetryTimer = GDT_INTERNAL_NETWORK_CLIENT_RETRY_TIME_SECONDS;
    this->clientBroadcast = clientBroadcast;
}

void GDT::NetworkConnection::setClientBroadcast(bool clientWillBroadcast)
{
    clientBroadcast = clientWillBroadcast;
}

void GDT::NetworkConnection::registerConnection(uint32_t address, uint32_t ID, unsigned short port)
{
    if(mode == SERVER)
    {
        connectionData.insert(std::make_pair(address, ConnectionData(generateID(), 0, port)));
    }
    else if(mode == CLIENT)
    {
        connectionData.insert(std::make_pair(address, ConnectionData(ID, 1, port)));
    }

    connectionMade(address);
}

void GDT::NetworkConnection::unregisterConnection(uint32_t address)
{
    if(connectionData.erase(address) != 0)
    {
        connectionLost(address);
    }
    else
    {
        std::cerr << "WARNING: unregisterConnection called with no matching connection!" << std::endl;
    }
}

void GDT::NetworkConnection::shiftBitfield(uint32_t address, uint32_t diff)
{
    connectionData.at(address).ackBitfield = (connectionData.at(address).ackBitfield >> diff) | 0x80000000;
}

void GDT::NetworkConnection::checkSentPackets(uint32_t ack, uint32_t bitfield, uint32_t address)
{
    if(!resendTimedOutPackets)
        return;

    --ack;
    for(; bitfield != 0x0; bitfield = bitfield << 1)
    {
        // if received, don't bother checking
        if((0x80000000 & bitfield) != 0x0)
        {
            --ack;
            continue;
        }

        // not received by client yet, checking if packet timed out
        for(auto iter = connectionData.at(address).sentPackets.begin(); iter != connectionData.at(address).sentPackets.end(); ++iter)
        {
            if(iter->id == ack)
            {
                if(iter->isNotReceivedChecked || iter->hasBeenReSent)
                {
                    // skip packets that intentionally are not checked or have
                    // already been re-sent
                    break;
                }
                // timed out, adding to send queue
                auto duration = std::chrono::steady_clock::now() - iter->sentTime;
                if(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() >= GDT_INTERNAL_NETWORK_PACKET_LOST_TIMEOUT_MILLISECONDS)
                {
#ifndef NDEBUG
                    std::cout << "Packet " << ack << "(" << std::hex << std::showbase << ack << std::dec;
                    std::cout << ") timed out\n";
#endif
                    std::vector<char> data = iter->data;
                    data.erase(data.begin(), data.begin() + 20);
                    resendPacket(data, address);
                    iter->hasBeenReSent = true;
                }
                break;
            }
        }

        --ack;
    }
}

void GDT::NetworkConnection::lookupRtt(uint32_t address, uint32_t ack)
{
    for(auto iter = connectionData.at(address).sentPackets.begin(); iter != connectionData.at(address).sentPackets.end(); ++iter)
    {
        if(iter->id == ack)
        {
            auto duration = std::chrono::steady_clock::now() - iter->sentTime;
            if(duration > connectionData.at(address).rtt)
            {
                connectionData.at(address).rtt += (std::chrono::duration_cast<std::chrono::milliseconds>(duration) - connectionData.at(address).rtt) / 10;
            }
            else
            {
                connectionData.at(address).rtt -= (connectionData.at(address).rtt - std::chrono::duration_cast<std::chrono::milliseconds>(duration)) / 10;
            }
#ifndef NDEBUG
            std::cout << "(" << ack << ") RTT of " << GDT::Internal::Network::addressToString(address) << " = " << connectionData.at(address).rtt.count() << '\n';
#endif
            connectionData.at(address).isGoodRtt = connectionData.at(address).rtt.count() <= GDT_INTERNAL_NETWORK_GOOD_RTT_LIMIT_MILLISECONDS;
            break;
        }
    }
}

void GDT::NetworkConnection::checkSentPacketsSize(uint32_t address)
{
    while(connectionData.at(address).sentPackets.size() > GDT_INTERNAL_NETWORK_SENT_PACKET_LIST_MAX_SIZE)
    {
        connectionData.at(address).sentPackets.pop_back();
    }
}

uint32_t GDT::NetworkConnection::generateID()
{
    uint32_t id;
    do
    {
        id = dist(rd)
            & ~(GDT::Internal::Network::CONNECT
                | GDT::Internal::Network::PING
                | GDT::Internal::Network::NO_REC_CHK
                | GDT::Internal::Network::RESENDING);
    } while (connectionData.find(id) != connectionData.end());

    return id;
}

void GDT::NetworkConnection::preparePacket(std::vector<char>& packetData, uint32_t& sequenceID, uint32_t address, bool isPing, bool isResending, bool isNotCheckReceivedPkt)
{
    assert(packetData.empty());

    auto iter = connectionData.find(address);
    assert(iter != connectionData.end());

    uint32_t id = iter->second.id;

    sequenceID = (iter->second.lSequence)++;

    uint32_t ack = iter->second.rSequence;

    uint32_t ackBitfield = iter->second.ackBitfield;

    char data[20];
    if(isNotCheckReceivedPkt)
    {
        uint32_t tempValue = htonl(GDT_INTERNAL_NETWORK_PROTOCOL_ID);
        std::memcpy(data, &tempValue, 4);
        tempValue = htonl(id | GDT::Internal::Network::NO_REC_CHK);
        std::memcpy(data + 4, &tempValue, 4);
        tempValue = htonl(sequenceID);
        std::memcpy(data + 8, &tempValue, 4);
        tempValue = htonl(ack);
        std::memcpy(data + 12, &tempValue, 4);
        tempValue = htonl(ackBitfield);
        std::memcpy(data + 16, &tempValue, 4);
    }
    else
    {
        if(isPing)
        {
            uint32_t tempValue = htonl(GDT_INTERNAL_NETWORK_PROTOCOL_ID);
            std::memcpy(data, &tempValue, 4);
            tempValue = htonl(id | GDT::Internal::Network::PING);
            std::memcpy(data + 4, &tempValue, 4);
            tempValue = htonl(sequenceID);
            std::memcpy(data + 8, &tempValue, 4);
            tempValue = htonl(ack);
            std::memcpy(data + 12, &tempValue, 4);
            tempValue = htonl(ackBitfield);
            std::memcpy(data + 16, &tempValue, 4);
        }
        else
        {
            uint32_t tempValue = htonl(GDT_INTERNAL_NETWORK_PROTOCOL_ID);
            std::memcpy(data, &tempValue, 4);
            tempValue = htonl(id
                | (isResending ? GDT::Internal::Network::RESENDING :
                    GDT::Internal::Network::NONE));
            std::memcpy(data + 4, &tempValue, 4);
            tempValue = htonl(sequenceID);
            std::memcpy(data + 8, &tempValue, 4);
            tempValue = htonl(ack);
            std::memcpy(data + 12, &tempValue, 4);
            tempValue = htonl(ackBitfield);
            std::memcpy(data + 16, &tempValue, 4);
        }
    }
    packetData.insert(packetData.end(), data, data + 20);
}

void GDT::NetworkConnection::receivedPacket(const char* data, uint32_t count, uint32_t address, bool outOfOrder, bool isResent, bool isNoIncSeq)
{
    if(receivedCallback && count > 0)
    {
        receivedCallback(data, count, address, outOfOrder, isResent, !isNoIncSeq);
    }
}

void GDT::NetworkConnection::connectionMade(uint32_t address)
{
    if(connectedCallback)
    {
        connectedCallback(address);
    }
}

void GDT::NetworkConnection::connectionLost(uint32_t address)
{
    if(disconnectedCallback)
    {
        disconnectedCallback(address);
    }
}

void GDT::NetworkConnection::initialize()
{
    if(validState)
    {
        return;
    }

    // get socket handle (file descriptor)
    socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socketHandle <= 0)
    {
        validState = false;
#ifndef NDEBUG
        std::cout << "ERROR: Failed to get socket!" << std::endl;
#endif
        return;
    }

    // set socket info
    socketInfo.sin_family = AF_INET;
    socketInfo.sin_addr.s_addr = INADDR_ANY;
    if(mode == CLIENT)
    {
        socketInfo.sin_port = htons(clientPort);
    }
    else
    {
        socketInfo.sin_port = htons(serverPort);
    }

    // bind socketInfo to socket
    if(bind(socketHandle, (const sockaddr*) &socketInfo, sizeof(sockaddr_in)) < 0)
    {
        validState = false;
#ifndef NDEBUG
        std::cout << "ERROR: Failed to bind socket!" << std::endl;
#endif
        return;
    }

    // set nonblocking
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
    int nonblocking = 1;
    if(fcntl(socketHandle, F_SETFL, O_NONBLOCK, nonblocking) == -1)
    {
        validState = false;
#ifndef NDEBUG
        std::cout << "ERROR: Failed to set socket non-blocking!" << std::endl;
#endif
        return;
    }
#else
    DWORD nonblocking = 1;
    if(ioctlsocket(socketHandle, FIONBIO, &nonblocking) != 0)
    {
        validState = false;
#ifndef NDEBUG
        std::cout << "ERROR: Failed to set socket non-blocking!" << std::endl;
#endif
        return;
    }
#endif

    if(clientBroadcast)
    {
        // set enable broadcast
#if PLATFORM == PLATFORM_WINDOWS
        char enabled = 1;
#else
        int enabled = 1;
#endif
        setsockopt(socketHandle, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));
    }

    validState = true;
}

