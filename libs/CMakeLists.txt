set(LLVM_BUILD_TOOLS OFF)
set(LLVM_INCLUDE_TOOLS OFF)
set(LLVM_INCLUDE_EXAMPLES OFF)
set(LLVM_INCLUDE_TESTS OFF)
set(LLVM_INCLUDE_BENCHMARKS OFF)
set(LLVM_APPEND_VC_REV ON)
set(LLVM_ENABLE_RTTI ON)

add_definitions(-DANTLR4CPP_STATIC)
set(WITH_STATIC_CRT ON)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/antlr4/runtime/Cpp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/llvm")

set(CLI11_BUILD_DOCS OFF)
set(CLI11_BUILD_TESTS OFF)
set(CLI11_BUILD_EXAMPLES OFF)
set(CLI11_BUILD_EXAMPLES_JSON OFF)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CLI11")

find_package(Threads)
set(LLVM_PTHREAD_LIB ${CMAKE_THREAD_LIBS_INIT})

set(LLVM_PACKAGE_VERSION "10.0.0")
set(LLVM_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/llvm/cmake/modules")
set(LLVM_TOOLS_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/llvm-project/llvm/${CMAKE_CFG_INTDIR}/bin")
set(LLVM_MAIN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/llvm/include")
set(LLVM_TABLEGEN_FLAGS -I ${LLVM_MAIN_INCLUDE_DIR})

set(LLVM_TABLEGEN_EXE "${LLVM_TOOLS_BINARY_DIR}/llvm-tblgen")

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(LLVM_TABLEGEN_EXE "${LLVM_TABLEGEN_EXE}.exe")
endif()

set(LLD_BUILT_STANDALONE Off)

cmake_policy(SET CMP0057 NEW)
set(PACKAGE_VERSION ${LLVM_PACKAGE_VERSION})

include_directories(
  ${LLVM_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/llvm/include"
  "${CMAKE_BINARY_DIR}/libs/llvm-project/llvm/include"

  "${CMAKE_CURRENT_SOURCE_DIR}/lld/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/lld"

  "${CMAKE_BINARY_DIR}/libs/lld/include"
  "${CMAKE_BINARY_DIR}/libs/lld"
)

add_definitions(${LLVM_DEFINITIONS})

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lld")

set(DEPENDENCIES_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/whereami/src/whereami.c"

  PARENT_SCOPE
)

set(DEPENDENCIES_HEADERS
  ${ANTLR4_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}/antlr4/runtime/Cpp/runtime/src"

  ${LLVM_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/llvm/include"
  "${CMAKE_BINARY_DIR}/libs/llvm-project/llvm/include"

  "${CMAKE_CURRENT_SOURCE_DIR}/CLI11/include"

  "${CMAKE_CURRENT_SOURCE_DIR}/lld/include"
  "${CMAKE_BINARY_DIR}/libs/lld"

  "${CMAKE_CURRENT_SOURCE_DIR}/whereami/src"

  PARENT_SCOPE
)
