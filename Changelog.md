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
