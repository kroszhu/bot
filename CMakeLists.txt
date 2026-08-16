cmake_minimum_required(VERSION 3.16)

project(qqbot VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DEPENDENCY_DIR ${CMAKE_BINARY_DIR}/_deps)
file(MAKE_DIRECTORY ${DEPENDENCY_DIR}/downloads ${DEPENDENCY_DIR}/sources)

function(download_dependency name url sha256 source_directory)
  set(archive ${DEPENDENCY_DIR}/downloads/${name}.archive)
  set(source ${DEPENDENCY_DIR}/sources/${source_directory})
  set(stamp ${source}/.qqbot-extracted)

  if(NOT EXISTS ${stamp})
    file(DOWNLOAD ${url} ${archive}
      EXPECTED_HASH SHA256=${sha256}
      TLS_VERIFY ON
      STATUS download_status
    )
    list(GET download_status 0 status_code)
    list(GET download_status 1 status_message)
    if(NOT status_code EQUAL 0)
      message(FATAL_ERROR "Failed to download ${name}: ${status_message}")
    endif()

    file(REMOVE_RECURSE ${source})
    file(ARCHIVE_EXTRACT INPUT ${archive} DESTINATION
      ${DEPENDENCY_DIR}/sources)
    file(TOUCH ${stamp})
  endif()

  set(${name}_SOURCE_DIR ${source} PARENT_SCOPE)
endfunction()

download_dependency(
  nlohmann_json
  https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
  json
)
download_dependency(
  mbedtls
  https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.2/mbedtls-3.6.2.tar.bz2
  8b54fb9bcf4d5a7078028e0520acddefb7900b3e66fec7f7175bb5b7d85ccdca
  mbedtls-3.6.2
)
download_dependency(
  ixwebsocket
  https://github.com/machinezone/IXWebSocket/archive/refs/tags/v11.4.6.tar.gz
  c024334f8e45980836c67008979a884d6dcc5ef067dd2eb1fa7241f4c17ddc32
  IXWebSocket-11.4.6
)
download_dependency(
  tomlplusplus
  https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.tar.gz
  8517f65938a4faae9ccf8ebb36631a38c1cadfb5efa85d9a72e15b9e97d25155
  tomlplusplus-3.4.0
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${nlohmann_json_SOURCE_DIR}
  ${DEPENDENCY_DIR}/build/nlohmann_json EXCLUDE_FROM_ALL)
add_subdirectory(${tomlplusplus_SOURCE_DIR}
  ${DEPENDENCY_DIR}/build/tomlplusplus EXCLUDE_FROM_ALL)

set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(${mbedtls_SOURCE_DIR}
  ${DEPENDENCY_DIR}/build/mbedtls EXCLUDE_FROM_ALL)

set(USE_TLS ON CACHE BOOL "" FORCE)
set(USE_MBED_TLS ON CACHE BOOL "" FORCE)
set(MBEDTLS_FOUND TRUE)
set(MBEDTLS_VERSION_GREATER_THAN_3 TRUE)
set(MBEDTLS_INCLUDE_DIRS ${mbedtls_SOURCE_DIR}/include)
set(MBEDTLS_LIBRARIES mbedtls mbedx509 mbedcrypto)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${ixwebsocket_SOURCE_DIR}
  ${DEPENDENCY_DIR}/build/ixwebsocket EXCLUDE_FROM_ALL)
target_compile_definitions(ixwebsocket
  PRIVATE IXWEBSOCKET_USE_MBED_TLS_MIN_VERSION_3
)

add_library(qqbot_lib src/client.cpp src/plugin.cpp src/protocol.cpp)
target_include_directories(qqbot_lib
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(qqbot_lib
  PRIVATE ixwebsocket nlohmann_json::nlohmann_json
)
target_compile_options(qqbot_lib PRIVATE -Wall -Wextra -Wpedantic)

file(GLOB PLUGIN_SOURCES CONFIGURE_DEPENDS plugins/*.cpp)
add_executable(qqbot src/main.cpp ${PLUGIN_SOURCES})
target_link_libraries(qqbot PRIVATE qqbot_lib tomlplusplus::tomlplusplus)
target_compile_options(qqbot PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(protocol_test tests/protocol_test.cpp src/protocol.cpp)
target_include_directories(protocol_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(protocol_test PRIVATE nlohmann_json::nlohmann_json)
target_compile_options(protocol_test PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
add_test(NAME protocol_test COMMAND protocol_test)

find_program(CLANG_FORMAT_EXECUTABLE clang-format)
if(CLANG_FORMAT_EXECUTABLE)
  file(GLOB_RECURSE FORMAT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
  )
  add_custom_target(
    format-check
    COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${FORMAT_SOURCES}
    VERBATIM
  )
endif()
