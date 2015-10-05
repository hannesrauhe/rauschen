PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS message.proto)

set(RAUSCHEN_CPP_FILES
  main.cpp
  server.cpp
  crypto.cpp
  message_action.cpp
)

add_executable(rauschen ${RAUSCHEN_CPP_FILES} ${PROTO_SRCS})
target_link_libraries(rauschen cryptopp ${PROTOBUF_LIBRARIES})

add_executable(sender sender.cpp crypto.cpp ${PROTO_SRCS})
target_link_libraries(sender cryptopp ${PROTOBUF_LIBRARIES})

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