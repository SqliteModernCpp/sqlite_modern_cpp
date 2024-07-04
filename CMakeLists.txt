cmake_minimum_required(VERSION 3.22)
option(ENABLE_SQLCIPHER_TESTS "enable sqlchipher test")

# Creates the file compile_commands.json in the build directory.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(HUNTER_TLS_VERIFY ON)

option(HUNTER_ENABLED "Enable Hunter package manager support" ON)

set(IS_MAIN_FILE CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
option(BUILD_TESTING "Enable build of tests" IS_MAIN_FILE)
option(ENABLE_INSTALL "Enable the installation of the library" "NOT IS_MAIN_FILE")

include("cmake/HunterGate.cmake")
include("cmake/Catch.cmake")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.24.15.tar.gz"
    SHA1 "8010d63d5ae611c564889d5fe12d3cb7a45703ac"
)

project(SQLiteModernCpp LANGUAGES CXX)

hunter_add_package(sqlite3)
find_package(sqlite3 CONFIG REQUIRED)

add_library(SQLiteModernCpp INTERFACE)

target_include_directories(SQLiteModernCpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/hdr>
)

include(GNUInstallDirs)

if(ENABLE_INSTALL)
    install(
        TARGETS SQLiteModernCpp
        EXPORT SQLiteModernCppConfig
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT SQLiteModernCppConfig
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SQLiteModernCpp"
        NAMESPACE SQLiteModernCpp::
    )

    install(
        DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/hdr/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

if(BUILD_TESTING)
    hunter_add_package(Catch)
    find_package(Catch2 CONFIG REQUIRED)

    set(TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    file(GLOB TEST_SOURCES ${TEST_SOURCE_DIR}/*.cc)

    IF(NOT ENABLE_SQLCIPHER_TESTS)
        list(REMOVE_ITEM TEST_SOURCES ${TEST_SOURCE_DIR}/sqlcipher.cc)
    ENDIF(NOT ENABLE_SQLCIPHER_TESTS)

    enable_testing()

    add_executable(tests_runner ${TEST_SOURCES})
    target_include_directories(tests_runner INTERFACE ${SQLITE3_INCLUDE_DIRS})

    if(ENABLE_SQLCIPHER_TESTS)
        target_link_libraries(tests_runner Catch2::Catch2 sqlite_modern_cpp sqlite3::sqlite3 -lsqlcipher)
    else()
        target_link_libraries(tests_runner Catch2::Catch2 sqlite_modern_cpp sqlite3::sqlite3)
    endif()

    catch_discover_tests(tests_runner)

    target_compile_options(tests_runner PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
endif()

# Place the file in the source directory, permitting us to place a single configuration file for YCM there.
# YCM is the code-completion engine for (neo)vim https://github.com/Valloric/YouCompleteMe
IF(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
ENDIF()