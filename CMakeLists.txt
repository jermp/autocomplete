cmake_minimum_required(VERSION 2.8)
project(AUTOCOMPLETE)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  MESSAGE(STATUS "DEBUG defined")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

MESSAGE( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

if(UNIX)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-braces")

  if(USE_SANITIZERS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  endif()

  if(USE_PDEP)
    # Use parallel bit deposit intrinsic.
    # Available on x86-64 since Intel’s Haswell CPUs: BMI2 (Bit Manipulation Instruction Set 2).
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PDEP")
  endif()

  if(USE_INTRINSICS)
    # Use intrinsics. Available on x86-64 since SSE4.2.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_INTRINSICS")
  endif()

endif()

include_directories(${AUTOCOMPLETE_SOURCE_DIR}/include)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(benchmark)

enable_testing()
file(GLOB TEST_SOURCES test/test_*.cpp)
foreach(TEST_SRC ${TEST_SOURCES})
  get_filename_component (TEST_SRC_NAME ${TEST_SRC} NAME_WE) # without extension
  add_executable(${TEST_SRC_NAME} ${TEST_SRC})
  add_test(${TEST_SRC_NAME} ${TEST_SRC_NAME})
endforeach(TEST_SRC)
