cmake_minimum_required(VERSION 3.10)

# Creates the file compile_commands.json in the build directory.
SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set (CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include("cmake/ParseAndAddCatchTests.cmake")
include("cmake/DownloadProject.CMake")

project(SqliteModernCpp)

find_package(sqlite3 REQUIRED)
download_project(
    PROJ                catch
    GIT_REPOSITORY      git@github.com:catchorg/Catch2.git
    GIT_TAG             v2.1.0
    UPDATE_DISCONNECTED 1
)

set(CATCH_INCLUDE_DIR           ${CMAKE_BINARY_DIR}/catch-src/single_include/)
set(TEST_SOURCE_DIR             ${CMAKE_SOURCE_DIR}/tests)
#file(GLOB TEST_SOURCES          ${TEST_SOURCE_DIR}/*.cc)
set(TEST_SOURCES 
    ${TEST_SOURCE_DIR}/readme_example.cc
    ${TEST_SOURCE_DIR}/blob_example.cc
    ${TEST_SOURCE_DIR}/error_log.cc
    ${TEST_SOURCE_DIR}/error_log2.cc
    ${TEST_SOURCE_DIR}/exception_dont_execute.cc
    ${TEST_SOURCE_DIR}/exception_dont_execute_nested.cc
)

enable_testing()

add_library (sqlite_modern_cpp INTERFACE)
target_include_directories(sqlite_modern_cpp INTERFACE hdr/)

add_library (Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_executable(tests ${TEST_SOURCES})
target_include_directories(tests INTERFACE ${SQLITE3_INCLUDE_DIRS})
target_link_libraries(tests Catch sqlite_modern_cpp sqlite3)

ParseAndAddCatchTests(tests)

# Place the file in the source directory, permitting us to place a single configuration file for YCM there.
# YCM is the code-completion engine for (neo)vim https://github.com/Valloric/YouCompleteMe
IF(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
  )
ENDIF()
