include(GenerateExportHeader)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CRYPTOPP_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS message.proto)

set(RAUSCHEND_FILES
  daemon/main.cpp
)

set(RAUSCHEN_SERVER_FILES
  daemon/server.cpp
  daemon/crypto.cpp
  daemon/action.cpp
  daemon/api_actions.cpp
)

set(RAUSCHEN_API_HEADERS
  api/rauschen.h
  api/rauschen_server.h
)

set(RAUSCHEN_CLIENT_API_FILES
  api/rauschen.cpp
)

set(RAUSCHEN_API_FILES
  ${RAUSCHEN_CLIENT_API_FILES}
  api/rauschen_server.cpp
)

if(MSVC)
  add_library(rauschen STATIC ${RAUSCHEN_SERVER_FILES} ${RAUSCHEN_API_FILES} ${PROTO_SRCS})
  add_library(rauschen-client STATIC ${RAUSCHEN_CLIENT_API_FILES} ${PROTO_SRCS})
else()
  add_library(rauschen SHARED ${RAUSCHEN_SERVER_FILES} ${RAUSCHEN_API_FILES} ${PROTO_SRCS})
  add_library(rauschen-client SHARED ${RAUSCHEN_CLIENT_API_FILES} ${PROTO_SRCS})
endif()
generate_export_header(rauschen) #for shared library on Windows
target_link_libraries(rauschen ${CRYPTOPP_LIBRARIES} ${PROTOBUF_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(rauschen-client ${PROTOBUF_LIBRARIES} ${Boost_LIBRARIES})

add_executable(rauschend ${RAUSCHEND_FILES})
target_link_libraries(rauschend rauschen)

if(BUILD_TOOLS) 
  message(STATUS "Building command line tools")
  set(RAUSCHEN_CLI_TOOLS
    rauschen-add-host
    rauschen-send-message
    rauschen-print-messages
    rauschen-execute
  )
    
  add_executable(rauschen-add-host tools/add_host.c)
  target_link_libraries(rauschen-add-host rauschen-client ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

  add_executable(rauschen-send-message tools/send_message.cpp)
  target_link_libraries(rauschen-send-message rauschen-client ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

  add_executable(rauschen-print-messages tools/print_messages.cpp)
  target_link_libraries(rauschen-print-messages rauschen-client ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

  add_executable(rauschen-execute tools/execute.cpp)
  target_link_libraries(rauschen-execute rauschen-client ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})
endif()


### install ###
include(install.cmake)

##### build tests #####
if(BUILD_TESTS)
  message(STATUS "Building tests")
  add_subdirectory(test)
endif()
