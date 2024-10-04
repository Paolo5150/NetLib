# CMakeList.txt : CMake project for Client, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Client   "SimpleClient.cpp" "ClientTCP.h" "ClientTCP.cpp" "ClientUDP.h" "ClientUDP.cpp")

target_include_directories(Client PRIVATE "${CMAKE_SOURCE_DIR}/NetLib/include")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Client PROPERTY CXX_STANDARD 17)
endif()

# TODO: Add tests and install targets if needed.
