set(RAUSCHEN_BINARIES
  rauschend 	
  ${RAUSCHEN_CLI_TOOLS}
)

install(TARGETS 
  ${RAUSCHEN_BINARIES}
  DESTINATION bin)

if(BUILD_TOOLS)
  install (TARGETS rauschen
     ARCHIVE DESTINATION lib
     LIBRARY DESTINATION lib
     RUNTIME DESTINATION bin)
     
  install(FILES ${RAUSCHEN_API_HEADERS}
    DESTINATION "include"
  )
endif()
    
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