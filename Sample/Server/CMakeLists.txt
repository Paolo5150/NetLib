# CMakeList.txt : CMake project for Server, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Server   "SimpleServer.cpp" "ServerTCP.h" "ServerTCP.cpp" "ServerUDP.h" "ServerUDP.cpp")

target_include_directories(Server PRIVATE "${CMAKE_SOURCE_DIR}/NetLib/include")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Server PROPERTY CXX_STANDARD 17)
endif()

# TODO: Add tests and install targets if needed.
