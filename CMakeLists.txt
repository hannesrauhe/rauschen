cmake_minimum_required( VERSION 2.8.6 )

project(Rauschen)

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

#static runtime linking for Windows
if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG_INIT "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_C_FLAGS_MINSIZEREL_INIT     "/MT /O1 /Ob1 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELEASE_INIT        "/MT /O2 /Ob2 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "/MT /Zi /O2 /Ob1 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "/MT /O1 /Ob1 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT        "/MT /O2 /Ob2 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MT /Zi /O2 /Ob1 /D NDEBUG")
endif()

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

if(DEFINED RAUSCHEN_DEP_DIR)
  message(INFO " Using additional dependency directory")
  include_directories(${RAUSCHEN_DEP_DIR}/include)
  link_directories(${RAUSCHEN_DEP_DIR}/lib)
endif()

if(BUILD_TESTS)
  find_package(GTEST)
endif()

# Output directory
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
add_subdirectory(src)
