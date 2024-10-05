# CMakeList.txt : CMake project for NetLib, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (NetLib    "include/NetLib/TSQueue.h" "include/NetLib/NetMessage.h" "include/NetLib/TCPConnection.h" "include/NetLib/TCPServer.h" 
"include/NetLib/TCPClient.h" 
"include/NetLib/TCPClientServerConnection.h" "include/NetLib/TCPServerClientConnection.h" "include/NetLib/UDPSender.h" "include/NetLib/UDPPacket.h"
"include/NetLib/UDPPacketAssembler.h" "include/NetLib/UDPMessage.h" "include/NetLib/UDPMessager.h")


 set_property(TARGET NetLib PROPERTY CXX_STANDARD 17)

