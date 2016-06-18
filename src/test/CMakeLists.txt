set(RAUSCHEND_TEST_FILES
    rauschend/test_crypto.cpp
    rauschend/test_dispatcher.cpp
    rauschend/fake_server.cpp
    rauschend/main.cpp
    ../daemon/action.cpp
    ../daemon/api_actions.cpp
    ../daemon/crypto.cpp
)

set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

add_executable(test_rauschend ${RAUSCHEND_TEST_FILES} ${PROTO_SRCS})
target_link_libraries(test_rauschend cryptopp gtest asan ${PROTOBUF_LIBRARIES})
  
set(RAUSCHEN_API_TEST_FILES
  api/test_connector.cpp
  api/main.cpp
)
  
add_executable(test_rauschen_api ${RAUSCHEN_API_TEST_FILES} ${PROTO_SRCS})
target_link_libraries(test_rauschen_api gtest gmock asan ${PROTOBUF_LIBRARIES})