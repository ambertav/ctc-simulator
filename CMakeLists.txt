cmake_minimum_required(VERSION 3.20)
project(ctc_simulator VERSION 0.1 LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(LOG_DIR ${CMAKE_SOURCE_DIR}/logs)
set(DATA_DIR ${CMAKE_SOURCE_DIR}/data)

# =============================
# app
# =============================
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
add_executable(app ${SOURCES})
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_compile_definitions(app PRIVATE 
    LOG_DIR=\"${LOG_DIR}\"
    DATA_DIR=\"${DATA_DIR}\"
)


# =============================
# tests
# =============================
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
)
file(GLOB_RECURSE PRODUCTION_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
list(FILTER PRODUCTION_SOURCES EXCLUDE REGEX "main.cpp")
add_executable(ctc_tests ${TEST_SOURCES} ${PRODUCTION_SOURCES})
target_include_directories(ctc_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ctc_tests PRIVATE gmock_main)
set_target_properties(ctc_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

target_compile_definitions(ctc_tests PRIVATE 
    LOG_DIR=\"${LOG_DIR}\"
    DATA_DIR=\"${DATA_DIR}\"
)

include(GoogleTest)
gtest_discover_tests(ctc_tests)