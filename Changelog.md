# Version 1.8

Change default interval of gameloop to 1/120 instead of 1/90.

Added some documentation and fixed Doxyfile.

# Version 1.7

Added path-finding generic function. See src/test/TestPathFinding.cpp for
example usage.

# Version 1.6

Fix SceneNode to properly default initialize matrices to an identity matrix.
Minor fix to NetworkConnection to check ip address when connecting (not in
broadcast mode).

# Version 1.5

Change how NetworkConnection does heartbeat packets:

When no packets are queued by a user, the NetworkConnection still sends packets
at an interval to maintain the connection. This change slows the rate of
"heartbeat" packets to 150 ms instead of "1/30" seconds for good mode and "1/10"
seconds for bad mode.

# Version 1.4

Fix bug where ackBitfield was not properly set in NetworkConnection.cpp, causing
already received packets to be resent if they were received out of order.

Fix bug where resent packets could be resent again.

Fix version in CMakeLists.txt.

Fix warnings emitted when compiling code.

# Version 1.3

Added feature to NetworkConnection where packets can be specified to not be
resent if dropped/timed-out.

The callback function signature for received packets in NetworkConnection now
includes an additional boolean parameter which indicates if the packet was
specified to not be received-checked.

# Version 1.2

Added CollisionDetection.
Moved SceneNode to within GDT namespace.

# Version 1.1

Fixed bug in NetworkConnection where broadcasting client always failed to
connect with server.

# Version 1.0

Initial version with NetworkConnection, SceneNode, and GameLoop.
