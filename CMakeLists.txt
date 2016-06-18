cmake_minimum_required( VERSION 3.0.2 )

project(rauschen)

find_package(Git)
if(GIT_FOUND)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  include(GetGitRevisionDescription)
  git_describe(VERSION "--tags")
  
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
    set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
else()
  set(VERSION UNKNOWN)
endif()

message(STATUS "Building Version ${VERSION}")

set(CMAKE_CXX_STANDARD 11)
if (NOT MSVC)
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()
endif()

if(DEFINED RAUSCHEN_DEP_DIR)
  message(STATUS " Using additional dependency directory")
  include_directories(${RAUSCHEN_DEP_DIR}/include)
  link_directories(${RAUSCHEN_DEP_DIR}/lib)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${RAUSCHEN_DEP_DIR})
  set(CRYPTOPP_ROOT_DIR ${RAUSCHEN_DEP_DIR})
  set(ASIO_ROOT_DIR ${RAUSCHEN_DEP_DIR})
endif()

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

if(MSVC)
  set(Boost_USE_STATIC_LIBS   ON)
endif()
find_package(Boost REQUIRED program_options system date_time regex)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS}/lib)


find_package(CryptoPP REQUIRED)

find_package(GTest)
#if(GTEST_FOUND)
#  set(BUILD_TESTS 1)
#endif()

# Output directory
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})

add_subdirectory(src)
