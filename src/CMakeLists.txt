PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS message.proto)

set(RAUSCHEN_CPP_FILES
  main.cpp
  server.cpp
  message_handler.cpp
)

add_executable(rauschen ${RAUSCHEN_CPP_FILES} ${PROTO_SRCS})
target_link_libraries(rauschen gcrypt asan ${PROTOBUF_LIBRARIES})

add_executable(sender sender.cpp ${PROTO_SRCS})
target_link_libraries(sender gcrypt ${PROTOBUF_LIBRARIES})

##### build tests #####
set(RAUSCHEN_TEST_FILES
  test/test_crypto.cpp
)
add_executable(test_rauschen ${RAUSCHEN_TEST_FILES} ${PROTO_SRCS})
target_link_libraries(test_rauschen gcrypt gtest asan c ${PROTOBUF_LIBRARIES})