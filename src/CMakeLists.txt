PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS message.proto)

set(RAUSCHEN_CPP_FILES
  main.cpp
  server.cpp
  crypto.cpp
  message_action.cpp
)

set(RAUSCHEN_API_FILES
  api/rauschen.cpp
)

add_executable(rauschend ${RAUSCHEN_CPP_FILES} ${PROTO_SRCS})
target_link_libraries(rauschend cryptopp ${PROTOBUF_LIBRARIES})

add_library(rauschen SHARED ${RAUSCHEN_API_FILES} ${PROTO_SRCS})
target_link_libraries(rauschen ${PROTOBUF_LIBRARIES})

add_executable(rauschen-add-host tools/add_host.cpp)
target_link_libraries(rauschen-add-host rauschen ${PROTOBUF_LIBRARIES})

##### build tests #####
if(BUILD_TESTS)
  set(RAUSCHEN_TEST_FILES
    test/test_crypto.cpp
    test/test_dispatcher.cpp
    server.cpp
    message_action.cpp
    crypto.cpp
  )
  add_executable(test_rauschen ${RAUSCHEN_TEST_FILES} ${PROTO_SRCS})
  target_link_libraries(test_rauschen cryptopp gtest asan c ${PROTOBUF_LIBRARIES})
endif()