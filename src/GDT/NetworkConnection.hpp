
#ifndef ENGINE_CONNECTION_HPP
#define ENGINE_CONNECTION_HPP

#define INVALID_NOTICE_TIME 5.0f

#include <unordered_map>
#include <cassert>
#include <functional>
#include <list>
#include <random>

#include <iostream>

#include "Internal/NetworkIdentifiers.hpp"

namespace GDT
{

/// Implements a UDP based connection manager as client or server.
/**
    The implementation is based on
    http://gafferongames.com/networking-for-game-programmers/ .
    Note that this class uses structs and such defined in
    "GDT/Internal/NetworkIdentifiers.hpp".
    Thus, this class uses struct \ref ConnectionData which uses \ref PacketInfo.

    It is expected that NetworkConnection::update is called periodically from
    within a game loop with a deltaTime (time between calls to update).

    NetworkConnection maintains a queue of packets to send.
    Packets are sent periodically with an interval between 1/30th of a second
    and 1/10th of a second based on whether or not the connection is "good" or
    has a low round-trip-time.
*/
class NetworkConnection
{
public:
    using PacketInfo = GDT::Internal::Network::PacketInfo;
    using ConnectionData = GDT::Internal::Network::ConnectionData;

    /// An enum used for specifying whether or not a connection will run as
    /// "Client" or "Server".
    enum Mode
    {
        SERVER,
        CLIENT
    };

    /// Initializes based on the given mode (Client or Server) and server port.
    /**
        \param mode The enum value specifying whether or not the connection will
        run as Client or Server.
        \param serverPort The port of the server. If mode is "SERVER", then the
            UDP socket used will bind to this port. Otherwise, the "CLIENT" will
            connect to a server at this port and will bind to any UDP port for
            its socket.
        \param clientPort The port of the client. By default is 0, which tells
            the client to bind to any available UDP port. Otherwise binds to the
            specified port.
        \param clientBroadcast If true (and is CLIENT), then when initiating a
            connection to a server, the client will send to the broadcast
            address (255.255.255.255) when establishing a connection and will
            expect the server to respond from an unknown address. Note that this
            will only work if the server is on the same network, as sending to
            the broadcast ip address will only send to the current network.
    */
    NetworkConnection(Mode mode = SERVER, unsigned short serverPort = GDT_INTERNAL_NETWORK_SERVER_PORT, unsigned short clientPort = 0, bool clientBroadcast = false);

    ~NetworkConnection();
    /// If true, then the SERVER will accept new connections and the CLIENT will
    /// accept a connection to a server.
    /**
        Set this to false to prevent connecting to any new peers.
        Note that setting this to false will prevent a Client from trying to
        connect/reconnect to a server.
    */
    bool acceptNewConnections;
    /// If true, then any packets received out of order will be ignored.
    /**
        Ignored packets will not call the received packet callback specified by
        NetworkConnection::setReceivedCallback.
    */
    bool ignoreOutOfSequence;
    /// If true, then timed out packets will be resent when they have timed out.
    bool resendTimedOutPackets;

    /// Checks for received packets and maintains the connection.
    /**
        It is expected for this function to be called periodically in a game's
        main loop where parameter "deltaTime" is the deltaTime between frames.
        \param deltaTime The deltaTime, or time between the previous call to
            update and now.
    */
    void update(float deltaTime);

    /// Tells the Client the IP address of the server to connect to.
    /**
        Once the Client knows the IP address of the server, it will
        periodically attempt to establish a connection to the server unless
        NetworkConnection::acceptNewConnections is false. Connection attempts
        occur in NetworkConnection::update.

        \param a The first byte of the IP address. (for address 192.168.1.2,
            this should be "192")
        \param b The second byte of the IP address. (for address 192.168.1.2,
            this should be "168")
        \param c The third byte of the IP address. (for address 192.168.1.2,
            this should be "1")
        \param d The fourth byte of the IP address. (for address 192.168.1.2,
            this should be "2")
    */
    void connectToServer(unsigned char a,
                         unsigned char b,
                         unsigned char c,
                         unsigned char d);

    /// Tells the Client the IP address of the server to connect to.
    /**
        Once the Client knows the IP address of the server, it will
        periodically attempt to establish a connection to the server unless
        NetworkConnection::acceptNewConnections is false. Connection attempts
        occur in NetworkConnection::update.

        \param address The combined uint32 address. Use the alternate
            connectToServer() function if unsure of how to create this value.
    */
    void connectToServer(uint32_t address);

    /// Adds to the queue of to-send-packets the given packetData to the given
    /// destination IP address.
    void sendPacket(const std::vector<char>& packetData, uint32_t address);
    /// Adds to the queue of to-send-packets the given packetData to the given
    /// destination IP address.
    void sendPacket(const char* packetData, uint32_t packetSize, uint32_t address);

    /// Gets the calculated round-trip-time to an arbritrary connected peer.
    /** \return 0 if there are no connected peers. */
    float getRtt();
    /// Gets the calculated round-trip-time to the specified connected peer.
    /** \return 0 if the specified peer is not connected (not found). */
    float getRtt(uint32_t address);

    /// Sets the callback called when a valid packet is received.
    /**
        The callback will be called with the received data, the byte count of
        the received data, the IP address of the sender as an uint32, and a
        bool that is true when the packet was received out of order.
        Note that if the packet did not have any data other than what was used
        to manage the connection, then the callback will not be called.
    */
    void setReceivedCallback(std::function<void(const char*, uint32_t, uint32_t, bool)> callback);

    /// Sets the callback called when a connection to a peer is established.
    /**
        The callback will be called with the IP address of the peer as an
        uint32 (see SFML's sf::IpAddress).
    */
    void setConnectedCallback(std::function<void(uint32_t)> callback);

    /// Sets the callback called when a connection to a peer is dropped.
    /**
        Typically, a connetion is dropped when the last received packet was
        received too long ago (timed out).
        The callback will be called with the IP address of the peer that
        has disconnected as an uint32 (see SFML's sf::IpAddress).
    */
    void setDisconnectedCallback(std::function<void(uint32_t)> callback);

    /// Gets a list of IP addresses of all connected peers.
    /** Note that the IP addresses is in an uint32 format. */
    std::list<uint32_t> getConnected();

    /// Gets the size of the packet queue for the specified destination address.
    unsigned int getPacketQueueSize(uint32_t destinationAddress);
    /// Clears the packet queue for the specified destination address.
    void clearPacketQueue(uint32_t destinationAddress);

    /// Gets whether or not the connection is good for an arbritrary connection.
    /**
        It is expected to call this function as a Client.
        If there are no valid connections, then "false" will be returned.
        Note that if the connection is "good" then packets will be sent at an
        interval of 1/30th of a second. Otherwise packets will be sent at an
        interval of 1/10th of a second.
    */
    bool connectionIsGood();
    /// Gets whether or not the connection is good for the specified connection.
    /**
        If there is no connection to the specified IP address, then "false"
        will be returned.
        Note that if the connection is "good" then packets will be sent at an
        interval of 1/30th of a second. Otherwise packets will be sent at an
        interval of 1/10th of a second.
    */
    bool connectionIsGood(uint32_t destinationAddress);

    /// Resets the connection as if it was just constructed.
    /**
        Using this function one can change the NetworkConnection instance to a
        different mode/port.
        \param mode The enum value specifying whether or not the connection will
            run as Client or Server.
        \param serverPort The port of the server. If mode is "SERVER", then the
            UDP socket used will bind to this port. Otherwise, the "CLIENT" will
            connect to a server at this port and will bind to any UDP port for
            its socket.
        \param clientPort The port of the client. By default is 0, which tells
            the client to bind to any available UDP port. Otherwise binds to the
            specified port.
        \param clientBroadcast If true (and is CLIENT), then when initiating a
            connection to a server, the client will send to the broadcast
            address (255.255.255.255) when establishing a connection and will
            expect the server to respond from an unknown address. Note that
            this will only work if the server is on the same network, as sending
            to the broadcast ip address will only send to the current network.
    */
    void reset(Mode mode, unsigned short serverPort = GDT_INTERNAL_NETWORK_SERVER_PORT, unsigned short clientPort = 0, bool clientBroadcast = false);

    /// Sets whether or not the Client will broadcast when initiating a
    /// connection to a server.
    /**
        If true (and is CLIENT), then when initiating a connection to a server,
        the client will send to the broadcast address (255.255.255.255) when
        establishing a connection and will expect the server to respond from an
        unknown address. Note that this will only work if the server is on the
        same network, as sending to the broadcast ip address will only send to
        the current network.
    */
    void setClientBroadcast(bool clientWillBroadcast);

private:
    Mode mode;

    int socketHandle;
    sockaddr_in socketInfo;

    std::unordered_map<uint32_t, ConnectionData> connectionData;

    std::random_device rd;
    std::uniform_int_distribution<uint32_t> dist;

    uint32_t clientSentAddress;
    bool clientSentAddressSet;

    std::function<void(const char*, uint32_t, uint32_t, bool)> receivedCallback;
    std::function<void(uint32_t)> connectedCallback;
    std::function<void(uint32_t)> disconnectedCallback;

    bool initialized;
    bool validState;
    float invalidNoticeTimer;

    unsigned short serverPort;
    unsigned short clientPort;

    float clientRetryTimer;

    bool clientBroadcast;

    void registerConnection(uint32_t address, uint32_t ID, unsigned short port);
    void unregisterConnection(uint32_t address);

    void shiftBitfield(uint32_t address, uint32_t diff);

    void checkSentPackets(uint32_t ack, uint32_t bitfield, uint32_t address);

    void lookupRtt(uint32_t address, uint32_t ack);

    void checkSentPacketsSize(uint32_t address);

    uint32_t generateID();

    void preparePacket(std::vector<char>& packetData, uint32_t& sequenceID, uint32_t address, bool isPing = false);

    void sendPacket(const std::vector<char>& data, uint32_t address, uint32_t resendingID);

    void receivedPacket(const char* data, uint32_t count, uint32_t address, bool outOfOrder);

    void connectionMade(uint32_t address);

    void connectionLost(uint32_t address);

    void initialize();

};

} // namespace GDT

#endif

