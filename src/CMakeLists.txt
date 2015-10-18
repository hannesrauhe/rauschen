include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS message.proto)

set(RAUSCHEN_CPP_FILES
  main.cpp
  server.cpp
  crypto.cpp
  action.cpp
  api_actions.cpp
)

set(RAUSCHEN_API_FILES
  api/rauschen.cpp
)

add_executable(rauschend ${RAUSCHEN_CPP_FILES} ${PROTO_SRCS})
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
target_link_libraries(rauschen-send-message rauschen boost_program_options ${PROTOBUF_LIBRARIES})

add_executable(rauschen-print-messages tools/print_messages.cpp)
target_link_libraries(rauschen-print-messages rauschen boost_program_options ${PROTOBUF_LIBRARIES})

##### build tests #####
if(BUILD_TESTS)
  set(RAUSCHEND_TEST_FILES
    test/rauschend/test_crypto.cpp
    test/rauschend/test_dispatcher.cpp
    test/rauschend/fake_server.cpp
    action.cpp
    api_actions.cpp
    crypto.cpp
  )
  add_executable(test_rauschend ${RAUSCHEND_TEST_FILES} ${PROTO_SRCS})
  target_link_libraries(test_rauschend cryptopp gtest asan c ${PROTOBUF_LIBRARIES})
endif()