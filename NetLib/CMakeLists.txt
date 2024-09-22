# CMakeList.txt : CMake project for NetLib, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (NetLib   "src/Example.cpp" "include/NetLib/TSQueue.h" "include/NetLib/Message.h" "include/NetLib/NetConnection.h" "include/NetLib/NetServer.h" "include/NetLib/NetClient.h")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NetLib PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
