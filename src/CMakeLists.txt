PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS message.proto)

add_executable(rausch main.cpp ${PROTO_SRCS})
target_link_libraries(rausch gcrypt ${PROTOBUF_LIBRARIES})

add_executable(sender sender.cpp ${PROTO_SRCS})
target_link_libraries(sender gcrypt ${PROTOBUF_LIBRARIES})