# CMakeList.txt : CMake project for Tests, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Tests   "Tests.cpp" "TestUDPPacket.h" "Common.h" "TestUDPPacketAssembler.h" "TestUDPMessager.h" "TestTCP.h")

target_include_directories(Tests PRIVATE "${CMAKE_SOURCE_DIR}/NetLib/include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Tests PROPERTY CXX_STANDARD 17)
endif()

# TODO: Add tests and install targets if needed.
