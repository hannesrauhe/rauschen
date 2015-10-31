include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CRYPTOPP_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS message.proto)

set(RAUSCHEND_CPP_FILES
  daemon/main.cpp
  daemon/server.cpp
  daemon/crypto.cpp
  daemon/action.cpp
  daemon/api_actions.cpp
)

set(RAUSCHEN_API_HEADERS
  api/rauschen.h
)

set(RAUSCHEN_API_FILES
  api/rauschen.cpp
)

add_executable(rauschend ${RAUSCHEND_CPP_FILES} ${PROTO_SRCS})
target_link_libraries(rauschend ${CRYPTOPP_LIBRARIES} ${PROTOBUF_LIBRARIES})

#GENERATE_EXPORT_HEADER( rauschen
#             BASE_NAME rauschen
#             EXPORT_MACRO_NAME rauschen_EXPORT
#             EXPORT_FILE_NAME rauschen_Export.h
#             STATIC_DEFINE rauschen_BUILT_AS_STATIC
#)

if(BUILD_TOOLS)
  add_library(rauschen SHARED ${RAUSCHEN_API_FILES} ${PROTO_SRCS})
  target_link_libraries(rauschen ${PROTOBUF_LIBRARIES})
  
  set(RAUSCHEN_CLI_TOOLS
    rauschen-add-host
    rauschen-send-message
    rauschen-print-messages
    rauschen-execute
  )
    
  add_executable(rauschen-add-host tools/add_host.c)
  target_link_libraries(rauschen-add-host rauschen ${PROTOBUF_LIBRARIES})

  add_executable(rauschen-send-message tools/send_message.cpp)
  target_link_libraries(rauschen-send-message rauschen ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

  add_executable(rauschen-print-messages tools/print_messages.cpp)
  target_link_libraries(rauschen-print-messages rauschen ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

  add_executable(rauschen-execute tools/execute.cpp)
  target_link_libraries(rauschen-execute rauschen ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})
endif()


### install ###
include(install.cmake)

##### build tests #####
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
