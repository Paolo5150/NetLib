# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("NetLib")

# Include sub-projects.
# add_subdirectory ("NetLib")
# target_include_directories(NetLib PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/asio/include")
# target_include_directories(NetLib PRIVATE "${CMAKE_SOURCE_DIR}/NetLib/include")

# Flatbuffers compiler





# Specify where FlatBuffers is installed or built
set(FLATBUFFERS_COMPILER "${CMAKE_SOURCE_DIR}/Sample/Flatb/flatc.exe")

#schema files
# Find all .fbs files in your project
file(GLOB FLATBUFFERS_SCHEMAS "${CMAKE_SOURCE_DIR}/Sample/Flatb/Schemas/*.fbs")
file(GLOB FLATBUFFERS_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Sample/Flatb/Generated")


foreach(schema ${FLATBUFFERS_SCHEMAS})
    get_filename_component(schema_name ${schema} NAME_WE) # Get name without extension
    message("Compiling  ${schema}")
   add_custom_command(
       OUTPUT ${FLATBUFFERS_OUTPUT_DIR}
       COMMAND ${FLATBUFFERS_COMPILER} --cpp -o ${FLATBUFFERS_OUTPUT_DIR} ${schema}
       DEPENDS ${schema}
       COMMENT "Compiling FlatBuffers schema ${schema_name}.fbs"
   )


endforeach()
add_custom_target(CompileFlatBuffers ALL DEPENDS ${FLATBUFFERS_OUTPUT_DIR} )

add_subdirectory ("Sample/Client")
target_include_directories(Client PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/asio/include")
target_include_directories(Client PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/flatbuffers/include")
target_include_directories(Client PRIVATE "${CMAKE_SOURCE_DIR}/Sample/Flatb/Generated")

add_subdirectory ("Sample/Server")
target_include_directories(Server PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/asio/include")
target_include_directories(Server PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/flatbuffers/include")
target_include_directories(Server PRIVATE "${CMAKE_SOURCE_DIR}/Sample/Flatb/Generated")



add_subdirectory ("Tests")
target_include_directories(Tests PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/asio/include")

