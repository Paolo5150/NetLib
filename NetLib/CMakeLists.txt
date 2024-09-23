# CMakeList.txt : CMake project for NetLib, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (NetLib    "include/NetLib/TSQueue.h" "include/NetLib/TCPMessage.h" "include/NetLib/TCPConnection.h" "include/NetLib/TCPServer.h" "include/NetLib/TCPClient.h" 
"include/NetLib/TCPClientServerConnection.h" "include/NetLib/TCPServerClientConnection.h" "include/NetLib/UDPSender.h")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NetLib PROPERTY CXX_STANDARD 17)
endif()

# TODO: Add tests and install targets if needed.
