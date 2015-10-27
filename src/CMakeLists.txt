include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

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
target_link_libraries(rauschend cryptopp ${PROTOBUF_LIBRARIES})

add_library(rauschen SHARED ${RAUSCHEN_API_FILES} ${PROTO_SRCS})
target_link_libraries(rauschen ${PROTOBUF_LIBRARIES})

#GENERATE_EXPORT_HEADER( rauschen
#             BASE_NAME rauschen
#             EXPORT_MACRO_NAME rauschen_EXPORT
#             EXPORT_FILE_NAME rauschen_Export.h
#             STATIC_DEFINE rauschen_BUILT_AS_STATIC
#)

add_executable(rauschen-add-host tools/add_host.c)
target_link_libraries(rauschen-add-host rauschen ${PROTOBUF_LIBRARIES})

add_executable(rauschen-send-message tools/send_message.cpp)
target_link_libraries(rauschen-send-message rauschen ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

add_executable(rauschen-print-messages tools/print_messages.cpp)
target_link_libraries(rauschen-print-messages rauschen ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

add_executable(rauschen-execute tools/execute.cpp)
target_link_libraries(rauschen-execute rauschen ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})


### install ###
set(RAUSCHEN_BINARIES
    rauschend 
    rauschen-add-host 
    rauschen-send-message 
    rauschen-print-messages 
    rauschen-execute
)

install(TARGETS 
    ${RAUSCHEN_BINARIES}
    DESTINATION bin)
    
install (TARGETS rauschen
     ARCHIVE DESTINATION lib
     LIBRARY DESTINATION lib
     RUNTIME DESTINATION bin)
     
install(FILES ${RAUSCHEN_API_HEADERS}
    DESTINATION "include"
)
    
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Hannes Rauhe")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Peer-to-Peer messaging within a local network")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_STRIP_FILES ${RAUSCHEN_BINARIES})

include(CPack)


##### build tests #####
if(BUILD_TESTS)
add_subdirectory(test)
endif()